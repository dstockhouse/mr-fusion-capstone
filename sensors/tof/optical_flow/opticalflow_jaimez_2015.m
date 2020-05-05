% Optical Flow using range cameras from Jaimez (2015)
% Joy Fucella and David Stockhouse

clear; clc; close all;

% webcamlist
% cam = webcam('PicoZense RGBD Camera'); % change to necessary cam
% 
% preview(cam);

% Read image files
start_depth = imread('samples\startpoint_depthmap.ppm');
start_rgb = imread('samples\startpoint_rgb.bmp');
start_rawpoints = dlmread('samples\startpoint_pointcloud.txt');
% start_rawpc = pointCloud(rawpoints);

move_depth = imread('samples\move10cm_depthmap.ppm');
move_rgb = imread('samples\move10cm_rgb.bmp');
move_rawpoints = dlmread('samples\move10cm_pointcloud.txt');
% move_rawpc = pointCloud(rawpoints1);

rotate_depth = imread('samples\rotate30deg_depthmap.ppm');
rotate_rgb = imread('samples\rotate30deg_rgb.bmp');
rotate_rawpoints = dlmread('samples\rotate30deg_pointcloud.txt');
% rotate_rawpc = pointCloud(rawpoints2);


%
%
% The process for computing the optical flow (in Jaimez 2015) is as follows:
%
%   Build the Gaussian Pyramid
%
%   For each pyramid step starting with "highest level" (lowest resolution)
%
%      Warp pyramid points to align with optical flow of previous level (if not first level)
%
%      Generate point cloud at pixel indices, nullify 0-depth points
%
%      Compute depth derivatives
%
%      Compute uncertainty weighting (for weighted least squares)
%
%      Solve weighted least squares linear equation for velocity state
%
%      Filter velocity with position kinematics
%
%   Compute 
%

% Set some constants
constants.fov_horizontal = 69 *pi/180;
constants.fov_vertical   = 51 *pi/180;
constants.fovh = constants.fov_horizontal;
constants.fovv = constants.fov_vertical;
constants.fps = 1;

constants.cols = 640;
constants.rows = 360;

%% Gaussian pyramid

gaussian_levels = 3;
[p_depth_old, p_points_old] = gaussian_pyramid(double(start_depth) ./ 1000, gaussian_levels, constants);

% Add noise to simulate stationary camera
% new_depth = double(start_depth) + 10*randn(size(start_depth));
% new_depth = double(start_depth) ./ 1000;
% new_depth = double(move_depth) ./ 1000;
new_depth = double(rotate_depth) ./ 1000;
[p_depth_new, p_points_new] = gaussian_pyramid(new_depth, gaussian_levels, constants);

% p_depth_new = p_depth_old + 10*randn(size(p_depth_old));
% p_points_new = p_points_old + 10*randn(size(p_points_old));
% p_depth_new = p_depth_old;
% p_points_new = p_points_old;
% [p_depth_new, p_points_new] = gaussian_pyramid(move_depth, gaussian_levels, constants);

% Plot the pyramid
PLOT_PYRAMID = true;
if PLOT_PYRAMID
    
    % Show starting depth and points
    cols = constants.cols;
    rows = constants.rows;
    figure(1);
    imshownorm(reshape(p_depth_old(1,1:rows, 1:cols), rows, cols));
    for ii = 1:gaussian_levels
        
        clear points_temp;
        points_temp(1:rows, 1:cols, 1:3) = p_points_old(ii,1:rows,1:cols,:);
        figure(1+ii);
        pcshow(points_temp);
        view(2);
        
        cols = cols/2;
        rows = rows/2;
        
    end
    
    % Show ending depth and points
    cols = constants.cols;
    rows = constants.rows;
    figure(gaussian_levels+1);
    imshownorm(reshape(p_depth_new(1,1:rows, 1:cols), rows, cols));
    for ii = 1:gaussian_levels
        
        clear points_temp;
        points_temp(1:rows, 1:cols, 1:3) = p_points_new(ii,1:rows,1:cols,:);
        figure(gaussian_levels+ii+1);
        pcshow(points_temp);
        view(2);
        
        cols = cols/2;
        rows = rows/2;
        
    end
    
    % Wait until user done looking at pyramid
    fprintf('\tPress any key to continue...\n');
    pause;
end

% Enough room to save transformation state for each pyramid level
p_kai = zeros(gaussian_levels, 6);
p_transformations = zeros(gaussian_levels, 4, 4);
accumulatedTransformation = eye(4);

cols = round(constants.max_cols / 2^gaussian_levels);
rows = round(constants.max_rows / 2^gaussian_levels);
kai_est = zeros(6, 1);
% Iterate through the gaussian pyramid
for image_level = gaussian_levels:-1:1

    %% Warp Image
    
    % Index pyramid to get current points now
    level_points_new = reshape(p_points_new(image_level, 1:rows, 1:cols, :), rows, cols, 3);

    % Don't warp if top of pyramid
    if image_level == gaussian_levels
        % Copy top level
        points_warped = level_points_new;
    else
        points_warped = warp_image(level_points_new, accumulatedTransformation, constants);
    end
    level_points_warped = reshape(points_warped, rows, cols, 3);

	%% Take the average of the old PC and new warped PC
    % Error popping up here
    level_points_old = reshape(p_points_old(image_level,1:rows,1:cols,:), rows, cols, 3);
    [points_average, num_points] = average_point_clouds(...
        level_points_old,...
        level_points_warped,...
        constants);

    %% Compute depth derivatives
    [du, dv, dt] = calcDepthDerivatives(...
        points_average,...
        level_points_old,...
        level_points_new,...
        constants);
    differentials(1:rows, 1:cols, 1) = du;
    differentials(1:rows, 1:cols, 2) = dv;
    differentials(1:rows, 1:cols, 3) = dt;

    %% Compute uncertainties & weighting
    weights = compute_weights(...
        level_points_old,...
        level_points_new,...
        points_average,...
        differentials,...
        kai_est,...
        accumulatedTransformation,...
        constants);

    %% Solve weighted least squares
    [p_kai(image_level, :), est_cov] = solveOneLevel(...
        points_average,...
        weights,...
        du, dv, dt,...
        num_points,...
        constants);

    %% Filter velocity
%     p_kai(image_level, :) = filter_velocity();
    p_transformations(image_level,:,:) = kai2trans(p_kai(image_level, :)');
%     p_transformations(image_level,:,:) = eye(4);

    %% Update state for next loop
    level_transformation = reshape(p_transformations(image_level,:,:),4,4);
    accumulatedTransformation = level_transformation * accumulatedTransformation;
    kai_level = trans2kai(accumulatedTransformation)

    cols = cols * 2;
    rows = rows * 2;

end

% If more than 2 images in sequence
p_points_old = p_points_new;
p_depth_old = p_depth_new;
kai_est = trans2kai(accumulatedTransformation);

