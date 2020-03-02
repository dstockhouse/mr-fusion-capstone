% Optical Flow using range cameras from Jaimez (2015)
% Joy Fucella and David Stockhouse

clear; clc; close all;

% Notes to complete before IDR:
%
% * Make slides for the visual odometry algorithm, as a flow chart, set of
%   steps as in the paper, and talk about what we implement here
% * Make this into a depth movie where the point cloud and depth/IR/points plot
%   is combined with some representation of the velocity. Maybe plot a moving
%   XYZ coordinate axis that shows where we believe the camera to be
% * Discuss the pitfalls of the depth map output and discuss fusing it with IR

%% Read image files

% From video captured from dragonboard
samples_path = '../samples';
addpath(samples_path);
filename = [samples_path '/very_little_motion.tof'];

% Ensure file exists where expected
if ~isfile(filename)
    fprintf('File ''%s'' does not exist. Look at the README in ''%s'' for instructions on downloading sample.\n',...
        filename, samples_path);
    return
end

% Read image stream from file
fprintf('Reading images from %s\n', filename);
[depth_images, ir_images, constants] = read_tof_file(filename);

% Set additional ToF dev kit parameters
constants.fov_horizontal = 90    *pi/180;
constants.fov_vertical   = 69.2  *pi/180;
constants.fovh = constants.fov_horizontal;
constants.fovv = constants.fov_vertical;

% Print some of the stats:
fprintf('\trows: %d\n', constants.rows);
fprintf('\tcols: %d\n', constants.cols);
fprintf('\tfps:  %d\n', constants.fps);
fprintf('\tRead %d frames for %.3f second video duration\n',...
    constants.num_frames, constants.duration);
rows = constants.rows;
cols = constants.cols;
fps = constants.fps;


% For now, only keep one second's worth of frames
numSeconds = 15;
num_frames = numSeconds * fps;
if num_frames < constants.num_frames
    
    % Update constants
    constants.num_frames = num_frames;
    constants.duration = constants.num_frames / constants.fps;
    
    % Restrict image streams
    depth_images = depth_images(1:num_frames, :, :);
    ir_images = ir_images(1:num_frames, :, :);
end

% Reduce integer (mm) measurements to double (m) measurements
depth_images = double(depth_images) / 1000;



%%%%%%%% Start Visual Odometry Calculation %%%%%%%%
%% Visual Odometry Calculation

% The process for computing the VO (in Jaimez 2015) is as follows:
%
%   Build the Gaussian Pyramid
%   For each pyramid step starting with "highest level" (lowest resolution)
%      Warp pyramid points to align with optical flow of previous level (if not first level)
%      Generate point cloud at pixel indices, nullify 0-depth points
%      Compute depth derivatives
%      Compute uncertainty weighting (for weighted least squares)
%      Solve weighted least squares linear equation for velocity state
%      Filter velocity with position kinematics
%   Compute relative motion from previous frame


% Setup initial state
camera_pose = eye(4);
    
% First image is the "old" image
gaussian_levels = 5;
start_depth = reshape(depth_images(1, :, :), constants.rows, constants.cols);
[p_depth_old, p_points_old] = gaussian_pyramid(start_depth, gaussian_levels, constants);

for frame_index = 2:num_frames
    
    fprintf('Computing visual odometry on frame %d of %d\n', frame_index, num_frames);
    
    %% Gaussian pyramid
    
    % Fetch next frame from camera and construct pyramid
    new_depth = reshape(depth_images(frame_index, :, :), constants.rows, constants.cols);
    [p_depth_new, p_points_new] = gaussian_pyramid(new_depth, gaussian_levels, constants);
    
    % Plot the pyramid
    PLOT_PYRAMID = false;
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
            view([0 0 -1]);
            
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
            view([0 0 -1]);
            
            cols = cols/2;
            rows = rows/2;
            
        end
        
        % Wait until user done looking at pyramid
        fprintf('\tPress any key to continue...\n');
        pause;
    end
    
    % Pre-allocate enough room to save transformation state for each pyramid level
    p_kai = zeros(gaussian_levels, 6);
    p_transformations = zeros(gaussian_levels, 4, 4);
    kai_est = zeros(6, 1);
    accumulatedTransformation = eye(4);

    % Iterate through the gaussian pyramid
    for image_level = gaussian_levels:-1:1
        
        % Number of rows and cols in this iteration
        cols = round(constants.cols / 2^(image_level-1));
        rows = round(constants.rows / 2^(image_level-1));
        
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
        
    end % for image_level 
    
    % Update camera position
    
    
    % Set up "old" state for next frame
    p_points_old = p_points_new;
    p_depth_old = p_depth_new;
    kai_est = trans2kai(accumulatedTransformation);
    fprintf('\tEstimated v = [%.3f %.3f %.3f] m/s, w = [%.3f %.3f %.3f] deg/s\n',...
        kai_est(1), kai_est(2), kai_est(3),...
        kai_est(4)*180/pi, kai_est(5)*180/pi, kai_est(6)*180/pi);
    
    
    % Update pose?

end % for frame_index

