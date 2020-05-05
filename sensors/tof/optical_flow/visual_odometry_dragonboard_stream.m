% Optical Flow using range cameras from Jaimez (2015)
% Joy Fucella and David Stockhouse

clear; clc; close all;


%% Read image file
addpath('../samples');
% From video captured from dragonboard
samples_path = '../samples';
addpath(samples_path);
% filename = [samples_path '/far_noise_reduce_60_rotate.tof'];
% filename = [samples_path '/far_noise_reduce_60_linear_move.tof'];
% filename = [samples_path '/medium_noise_reduce_60_rotate.tof'];
% filename = [samples_path '/medium_noise_reduce_60_linear_move.tof'];
% filename = [samples_path '/far_move_forward_then_back.tof'];
filename = [samples_path '/medium_apt_rotate.tof'];
% filename = [samples_path '/far_apt_stationary.tof'];
% filename = [samples_path '/medium_apt_stationary.tof'];

% Ensure file exists where expected
if ~isfile(filename)
    fprintf('File ''%s'' does not exist. Look at the README in ''%s'' for instructions on downloading sample.\n',...
        filename, samples_path);
    return
end

% Read metadata from file header
fprintf('Reading image header from %s...\n', filename);
[fd, constants] = read_tof_header(filename);

% Set additional ToF dev kit parameters
constants.fov_horizontal = 90    *pi/180;
constants.fov_vertical   = 69.2  *pi/180;
constants.fovh = constants.fov_horizontal;
constants.fovv = constants.fov_vertical;

% Print some of the stats:
fprintf('\trows: %d\n', constants.rows);
fprintf('\tcols: %d\n', constants.cols);
fprintf('\tfps:  %d\n', constants.fps);
fprintf('\t %d frames for %.3f second video duration\n',...
    constants.num_frames, constants.duration);
rows = constants.rows;
cols = constants.cols;
fps = constants.fps;
num_frames = constants.num_frames;
focal_length = cols / (2 * tan(0.5 * constants.fovh));
constants.focal_length = focal_length;


% Thresholding constants for image filtering
ir_min_thresh = 30;
ir_max_thresh = 4090;

% Read starting frame
[depth_frame, ir_frame] = read_tof_frame(fd, constants.frame_size, constants);
raw_depth = depth_frame;
raw_ir = ir_frame;

% Reduce integer depth (mm) measurements to double (m) measurements
depth_frame = double(depth_frame) / 1000;


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

vtop = 0;
vbot = 0;
wtop = 0;
wbot = 0;
vtop_unf = 0;
vbot_unf = 0;
wtop_unf = 0;
wbot_unf = 0;
    
% First image is the "old" image
gaussian_levels = 6;
top_odometry_level = 2;
start_depth = fuse_ir_depth(depth_frame, ir_frame, ir_max_thresh, ir_min_thresh);
denoise_window = 6;
denoise_threshold = .7;
start_depth = depth_denoise(start_depth, focal_length, denoise_window, denoise_threshold);
[p_depth_old, p_points_old] = gaussian_pyramid(start_depth, gaussian_levels, constants);
kai_est_old = zeros(6,1);

min_depth = min(min(start_depth));
max_depth = max(max(start_depth));

still_frame = false;
[path, fname, ext] = fileparts(filename);
if still_frame
    moviename = [fname '_still_frame.mp4'];
else 
    moviename = [fname '_with_coord.mp4'];
end
fprintf('Writing video to %s, press any key to continue\n\n\n', moviename);
pause;
v = VideoWriter(moviename, 'MPEG-4');
v.FrameRate = constants.fps;
open(v);

for frame_index = 2:constants.num_frames
    
    fprintf('Computing visual odometry on frame %d of %d\n', frame_index, constants.num_frames);
    
    % Read next frame
    if still_frame
        new_depth = raw_depth;
    else
        [new_depth, ir_frame] = read_tof_frame(fd, constants.frame_size, constants);
    end
    new_depth = double(new_depth) / 1000;
    
    %% Gaussian pyramid
    
    % Clean noise from data
    new_depth = fuse_ir_depth(new_depth, ir_frame, ir_max_thresh, ir_min_thresh);
    new_depth = depth_denoise(new_depth, focal_length, denoise_window, denoise_threshold);
    
    if still_frame
        % Add noise
        new_depth(new_depth > .001) = new_depth(new_depth > .001) + .00001*rand(size(new_depth(new_depth > .001)));
    end
    
    % Construct pyramid with new frames
    [p_depth_new, p_points_new] = gaussian_pyramid(new_depth, gaussian_levels, constants);
    
    % Plot the pyramid
    PLOT_PYRAMID = false;
    if PLOT_PYRAMID
        
        % Show starting depth and points
        cols = constants.cols;
        rows = constants.rows;
        figure(5);
        imshownorm(reshape(p_depth_old(1, 1:rows, 1:cols), rows, cols));
        title('Old depth');
        figure(6);
        for ii = 1:gaussian_levels
            
            clear points_temp;
            points_temp(1:rows, 1:cols, 1:3) = p_points_old(ii,1:rows,1:cols,:);
            % Subplotting is not general, assumes 6 gaussian levels
            subplot(2, 3, ii);
            pcshow(points_temp);
            view([0 0 -1]);
            title(['level ' num2str(ii) ', ' num2str(cols) 'x' num2str(rows)]);
            xlabel('x (m)');
            ylabel('y (m)');
            zlabel('z (m)');
            
            cols = cols/2;
            rows = rows/2;
            
        end
        if exist('sgtitle', 'builtin') || exist('sgtitle', 'file')
            % Figure title
            sgtitle(['Frame ' num2str(frame_index) ' of ' num2str(num_frames) ...
                ' old (' num2str(frame_index/fps, '%.1f') '/' num2str(num_frames/fps, '%.1f') ' s)'],...
                'Color', 'w');
        end
        
        % Show ending depth and points
        cols = constants.cols;
        rows = constants.rows;
        figure(7);
        imshownorm(reshape(p_depth_new(1,1:rows, 1:cols), rows, cols));
        title('New depth');
        figure(8);
        for ii = 1:gaussian_levels
            
            clear points_temp;
            points_temp(1:rows, 1:cols, 1:3) = p_points_new(ii,1:rows,1:cols,:);
            % Subplotting is not general, assumes 6 gaussian levels
            subplot(2, 3, ii);
            pcshow(points_temp);
            view([0 0 -1]);
            title(['level ' num2str(ii) ', ' num2str(cols) 'x' num2str(rows)]);
            xlabel('x (m)');
            ylabel('y (m)');
            zlabel('z (m)');
            
            cols = cols/2;
            rows = rows/2;
            
        end
        if exist('sgtitle', 'builtin') || exist('sgtitle', 'file')
            % Figure title
            sgtitle(['Frame ' num2str(frame_index) ' of ' num2str(num_frames) ...
                ' new (' num2str(frame_index/fps, '%.1f') '/' num2str(num_frames/fps, '%.1f') ' s)'],...
                'Color', 'w');
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
    for image_level = gaussian_levels:-1:top_odometry_level
        
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
        p_kai_unfiltered(image_level,:) = kai_est;
        kai_est_unf = kai_est;
        p_transformations_unfiltered(image_level,:,:) = kai2trans(reshape(p_kai_unfiltered(image_level,:), 6, 1));
        [p_kai(image_level,:), p_transformations(image_level,:,:)] = filter_velocity(constants,kai_est_old,est_cov,kai_est,image_level,accumulatedTransformation);
        
        %% Update state for next loop
        level_transformation = reshape(p_transformations(image_level,:,:),4,4);
        accumulatedTransformation = level_transformation * accumulatedTransformation;
        kai_degrees = trans2kai(accumulatedTransformation)' .* [1 1 1 180/pi*[1 1 1]];
        fprintf('  kai %d = %7.4f  %7.4f  %7.4f  %7.4f  %7.4f  %7.4f\n', image_level,...
            kai_degrees(1), kai_degrees(2), kai_degrees(3),...
            kai_degrees(4), kai_degrees(5), kai_degrees(6));
        
        level_transformation_unfiltered = reshape(p_transformations_unfiltered(image_level,:,:),4,4);
        accumulatedTransformation_unfiltered = level_transformation_unfiltered * accumulatedTransformation_unfiltered;
        kai_degrees_unf = kai_est_unf' .* [1 1 1 180/pi*[1 1 1]];
        fprintf(' ukai %d = %7.4f  %7.4f  %7.4f  %7.4f  %7.4f  %7.4f\n', image_level,...
            kai_degrees_unf(1), kai_degrees_unf(2), kai_degrees_unf(3),...
            kai_degrees_unf(4), kai_degrees_unf(5), kai_degrees_unf(6));
        
    end % for image_level 
    
    
    
    % Update camera position
%     camera_pose = accumulatedTransformation * camera_pose;
    [cam_pos, cam_att] = update_pose(constants, cam_pos, cam_att, accumulatedTransformation);
    [cam_pos_unfiltered, cam_att_unfiltered] = update_pose(constants, cam_pos_unfiltered, cam_att_unfiltered, accumulatedTransformation_unfiltered);
    
    
    
    
    %% Nice plot
    
    vfig = figure(1);
    clf;
    
    % Top left plot
    subplot(2, 2, 1);
    if min(min(new_depth)) < min_depth
        min_depth = min(min(new_depth));
    end
    if max(max(new_depth)) > max_depth
        max_depth = max(max(new_depth));
    end
    imshownorm(new_depth, [min_depth max_depth]);
    xlabel('Depth', 'Color', 'w', 'FontWeight', 'bold');
    
    % Top right plot
    subplot(2, 2, 2);
    imshownorm(ir_frame);
    xlabel('IR Intensity', 'Color', 'w', 'FontWeight', 'bold');
    
    % Bottom left plot
    subplot(2, 2, 3);
    pcshow(reshape(p_points_new(1,:,:,:), constants.rows, constants.cols, 3));
    title('Point Cloud (front)');
    view([0 0 -1]);
    xlabel('x (m)');
    ylabel('y (m)');
    zlabel('z (m)');
    axis([ -4 4 -4 3 0 6]);
    
    
    % Bottom right plot
    subplot(2, 2, 4);
    plot_frame_orig(cam_att, cam_pos, '', 'w');
    ax = gca;
    ax.Color = 'black';
    ax.XColor = [.85 .85 .85];
    ax.YColor = [.85 .85 .85];
    ax.ZColor = [.85 .85 .85];
    ax.GridColor = [.85 .85 .85];
    title(['Estimated Motion (' num2str(norm(cam_pos), '%.2f') 'm displaced)'], 'color', 'w');
    view([0.01 0.1 -1]);
    axis([-1.2 1.2 -1.2 1.2 -1.2 1.2]);
    grid on;
    xlabel('x (m)', 'color', [.85 .85 .85]);
    ylabel('y (m)', 'color', [.85 .85 .85]);
    zlabel('z (m)', 'color', [.85 .85 .85]);
    
    if exist('sgtitle', 'builtin') || exist('sgtitle', 'file')
        % Figure title
        sgtitle(['Frame ' num2str(frame_index) ' of ' num2str(num_frames) ...
            ' (' num2str(frame_index/fps, '%.1f') '/' num2str(num_frames/fps, '%.1f') ' s)'],...
            'Color', 'w');
    end    
    frame = getframe(vfig);
    writeVideo(v, frame);
    
    
    % Less good plot
    figure(2);
    bound = 2;
    clf;
    subplot(1, 2, 1);
    plot_frame_orig(cam_att, cam_pos, '', 'k');
    title('filtered');
    view([0.01 0.1 -1]);
    axis([-bound bound -bound bound -bound bound]);
    grid on;
    
    subplot(1, 2, 2);
    plot_frame_orig(cam_att_unfiltered, cam_pos_unfiltered, '', 'k');
    title('unfiltered');
    view([-1 0.01 0.1]);
    axis([-bound bound -bound bound -bound bound]);
    grid on;
    
    
    
    % Set up "old" state for next frame
    p_points_old = p_points_new;
    p_depth_old = p_depth_new;
    kai_est = trans2kai(accumulatedTransformation);
    kai_est_old = kai_est;
    
    
    
    
    
%     fprintf('\tEstimated v = [%.3f %.3f %.3f] m/s, w = [%.3f %.3f %.3f] deg/s\n',...
%         kai_est(1), kai_est(2), kai_est(3),...
%         kai_est(4)*180/pi, kai_est(5)*180/pi, kai_est(6)*180/pi);
    % Estimated velocity graph
    if max(kai_est(1:3)) > vtop
        vtop = max(kai_est(1:3, :));
    end
    if min(kai_est(1:3)) < vbot
        vbot = min(kai_est(1:3, :));
    end
    if max(kai_est(4:6))*180/pi > wtop
        wtop = max(kai_est(4:6, :))*180/pi;
    end
    if min(kai_est(4:6))*180/pi < wbot
        wbot = min(kai_est(4:6, :))*180/pi;
    end
    figure(3);
    subplot(3,2,1);
    plot(frame_index,kai_est(1),'*r');
    ylim([vbot vtop]);
    title('v_x estimate');
    xlabel('Frame #');
    ylabel('m/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    subplot(3,2,3);
    plot(frame_index,kai_est(2),'*g');
    ylim([vbot vtop]);
    title('v_y estimate');
    xlabel('Frame #');
    ylabel('m/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    subplot(3,2,5);
    plot(frame_index,kai_est(3),'*b');
    ylim([vbot vtop]);
    title('v_z estimate');
    xlabel('Frame #');
    ylabel('m/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    subplot(3,2,2);
    plot(frame_index,kai_est(4)*180/pi,'*r');
    ylim([wbot wtop]);
    title('\omega_x estimate');
    xlabel('Frame #');
    ylabel('deg/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    subplot(3,2,4);
    plot(frame_index,kai_est(5)*180/pi,'*g');
    ylim([wbot wtop]);
    title('\omega_y estimate');
    xlabel('Frame #');
    ylabel('deg/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    subplot(3,2,6);
    plot(frame_index,kai_est(6)*180/pi,'*b');
    ylim([wbot wtop]);
    title('\omega_z estimate');
    xlabel('Frame #');
    ylabel('deg/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    if exist('sgtitle', 'builtin') || exist('sgtitle', 'file')
        % Figure title
        sgtitle('Filtered \xi');
    end
    pause(1e-3);
    
    
    % Plot unfiltered velocity estimates
    if max(kai_est_unf(1:3)) > vtop_unf
        vtop_unf = max(kai_est_unf(1:3, :));
    end
    if min(kai_est_unf(1:3)) < vbot_unf
        vbot_unf = min(kai_est_unf(1:3, :));
    end
    if max(kai_est_unf(4:6))*180/pi > wtop_unf
        wtop_unf = max(kai_est_unf(4:6, :))*180/pi;
    end
    if min(kai_est_unf(4:6))*180/pi < wbot_unf
        wbot_unf = min(kai_est_unf(4:6, :))*180/pi;
    end
    figure(4);
    subplot(3,2,1);
    plot(frame_index,kai_est_unf(1),'*r');
    ylim([vbot_unf vtop_unf]);
    title('v_x estimate');
    xlabel('Frame #');
    ylabel('m/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    subplot(3,2,3);
    plot(frame_index,kai_est_unf(2),'*g');
    ylim([vbot_unf vtop_unf]);
    title('v_y estimate');
    xlabel('Frame #');
    ylabel('m/s');
    hold on;
    grid on;
    axis tight;
    if still_frame
        axis tight;
    end
    subplot(3,2,5);
    plot(frame_index,kai_est_unf(3),'*b');
    ylim([vbot_unf vtop_unf]);
    title('v_z estimate');
    xlabel('Frame #');
    ylabel('m/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    subplot(3,2,2);
    plot(frame_index,kai_est_unf(4)*180/pi,'*r');
    ylim([wbot_unf wtop_unf]);
    title('\omega_x estimate');
    xlabel('Frame #');
    ylabel('deg/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    subplot(3,2,4);
    plot(frame_index,kai_est_unf(5)*180/pi,'*g');
    ylim([wbot_unf wtop_unf]);
    title('\omega_y estimate');
    xlabel('Frame #');
    ylabel('deg/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    subplot(3,2,6);
    plot(frame_index,kai_est_unf(6)*180/pi,'*b');
    ylim([wbot_unf wtop_unf]);
    title('\omega_z estimate');
    xlabel('Frame #');
    ylabel('deg/s');
    hold on;
    grid on;
    if still_frame
        axis tight;
    end
    
    if exist('sgtitle', 'builtin') || exist('sgtitle', 'file')
        % Figure title
        sgtitle('Unfiltered \xi');
    end
    pause(1e-3);
    

end % for frame_index

% Close image stream file
fclose(fd);

% Close animation video file
v.FrameRate;
close(v);
fprintf('Finished writing ''%s''\n', moviename);

