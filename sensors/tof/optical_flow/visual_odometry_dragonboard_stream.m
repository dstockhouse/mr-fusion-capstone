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
addpath('../samples');
% From video captured from dragonboard
samples_path = '../samples';
addpath(samples_path);
filename = [samples_path '/medium_noise_reduce_60_rotate.tof'];

% Ensure file exists where expected
if ~isfile(filename)
    fprintf('File ''%s'' does not exist. Look at the README in ''%s'' for instructions on downloading sample.\n',...
        filename, samples_path);
    return
end

% Read image stream from file
fprintf('Reading images from %s\n', filename);
[depth_images, ir_images, constants] = read_tof_file(filename, 200000000);

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

end

% Restrict image streams
depth_images = depth_images(1:constants.num_frames, :, :);
ir_images = ir_images(1:constants.num_frames, :, :);

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
cam_pos = zeros(3, 1);
cam_att = eye(3);
cam_pos_unfiltered = zeros(3, 1);
cam_att_unfiltered = eye(3);
    
% First image is the "old" image
gaussian_levels = 6;
start_depth = reshape(depth_images(1, :, :), constants.rows, constants.cols);
% start_depth = fuse_ir_depth(start_depth,reshape(ir_images(1, :, :), constants.rows, constants.cols),60);
[p_depth_old, p_points_old] = gaussian_pyramid(start_depth, gaussian_levels, constants);
kai_est_old = zeros(6,1);


moviename = 'coordinate_motion.mp4'
v = VideoWriter(moviename, 'MPEG-4');
v.FrameRate = constants.fps;
open(v);
vfig = figure(2);

for frame_index = 2:constants.num_frames
    
    fprintf('Computing visual odometry on frame %d of %d\n', frame_index, constants.num_frames);
    
    %% Gaussian pyramid
    
    % Fetch next frame from camera and construct pyramid
    new_depth = reshape(depth_images(frame_index, :, :), constants.rows, constants.cols);
%     new_depth = fuse_ir_depth(new_depth,reshape(ir_images(frame_index, :, :), constants.rows, constants.cols),60);
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
    p_kai_unfiltered = zeros(gaussian_levels, 6);
    p_transformations_unfiltered = zeros(gaussian_levels, 4, 4);
    kai_est = zeros(6, 1);
    accumulatedTransformation = eye(4);
    accumulatedTransformation_unfiltered = eye(4);

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
            level_points_warped,...
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
        kai_est = reshape(p_kai(image_level,:),6,1);
        
        %% Filter velocity
        %     p_kai(image_level, :) = filter_velocity();
%         p_transformations(image_level,:,:) = kai2trans(p_kai(image_level, :)');
        %     p_transformations(image_level,:,:) = eye(4);
        p_kai_unfiltered(image_level,:) = kai_est;
        p_transformations_unfiltered(image_level,:,:) = kai2trans(reshape(p_kai_unfiltered(image_level,:), 6, 1));
        [p_kai(image_level,:), p_transformations(image_level,:,:)] = filter_velocity(constants,kai_est_old,est_cov,kai_est,image_level,accumulatedTransformation);
        
        %% Update state for next loop
        level_transformation = reshape(p_transformations(image_level,:,:),4,4);
        accumulatedTransformation = level_transformation * accumulatedTransformation;
        
        level_transformation_unfiltered = reshape(p_transformations_unfiltered(image_level,:,:),4,4);
        accumulatedTransformation_unfiltered = level_transformation_unfiltered * accumulatedTransformation_unfiltered;
        
    end % for image_level 
    
    % Update camera position
%     camera_pose = accumulatedTransformation * camera_pose;
    [cam_pos, cam_att] = update_pose(constants, cam_pos, cam_att, accumulatedTransformation);
    [cam_pos_unfiltered, cam_att_unfiltered] = update_pose(constants, cam_pos_unfiltered, cam_att_unfiltered, accumulatedTransformation_unfiltered);
    figure(2);
    clf;
    subplot(1, 2, 1);
    plot_frame_orig(cam_att, cam_pos, '', 'k');
    title('filtered');
    view([-1 0.01 0.1]);
    axis([-1.2 1.2 -1.2 1.2 -1.2 1.2]);
    grid on;
    
    subplot(1, 2, 2);
    plot_frame_orig(cam_att_unfiltered, cam_pos_unfiltered, '', 'k');
    title('unfiltered');
    view([-1 0.01 0.1]);
    axis([-1.2 1.2 -1.2 1.2 -1.2 1.2]);
    grid on;
    
    frame = getframe(vfig);
    writeVideo(v, frame);
    
    % Get v_xyz, w_xyz as 3-vectors
    % Convert w_xyz into a 3x3 rotation matrix
    %   Rotate camera orientation with rotation matrix
    %   Translate camera position with velocity vector
    
    
    
    % Set up "old" state for next frame
    p_points_old = p_points_new;
    p_depth_old = p_depth_new;
    kai_est = trans2kai(accumulatedTransformation);
    kai_est_old = kai_est;
    
    
    
    
    
%     fprintf('\tEstimated v = [%.3f %.3f %.3f] m/s, w = [%.3f %.3f %.3f] deg/s\n',...
%         kai_est(1), kai_est(2), kai_est(3),...
%         kai_est(4)*180/pi, kai_est(5)*180/pi, kai_est(6)*180/pi);
    % Estimated velocity graph
    figure(1);
    subplot(3,2,1);
    plot(frame_index,kai_est(1),'*');
    title('V_x estimate');
    hold on;
    grid on;
    subplot(3,2,3);
    plot(frame_index,kai_est(2),'*');
    title('V_y estimate');
    hold on;
    grid on;
    subplot(3,2,5);
    plot(frame_index,kai_est(3),'*');
    title('V_z estimate');
    hold on;
    grid on;
    subplot(3,2,2);
    plot(frame_index,kai_est(4)*180/pi,'*');
    title('\omega_x estimate');
    hold on;
    grid on;
    subplot(3,2,4);
    plot(frame_index,kai_est(5)*180/pi,'*');
    title('\omega_y estimate');
    hold on;
    grid on;
    subplot(3,2,6);
    plot(frame_index,kai_est(6)*180/pi,'*');
    title('\omega_z estimate');
    hold on;
    grid on;
    pause(1e-3);
    % Update pose?
    
    
    
    %% Plot raw camera data 
    % See ../samples/depth_movie.m
    
    

end % for frame_index


v.FrameRate;
close(v);
fprintf('Finished writing ''%s''\n', moviename);

