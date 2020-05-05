%% Read depth and IR image data from file

clear; clc; close all;
addpath('../optical_flow');

% Downloaded one or more of the following video files from 
% http://mercury.pr.erau.edu/~stockhod/samples/[filename]
% and put it in an accessible directory
% filename = 'very_little_motion.tof';
% filename = 'med_rotate_left_then_right.tof';
% filename = 'far_rotate_right_then_left.tof';
% filename = 'medium_rotate_right_then_left.tof';
% filename = 'near_rotate_right_then_left.tof';
% filename = 'medium_noise_reduce_60_linear_move.tof';
% filename = 'medium_noise_reduce_60_rotate.tof';
% filename = 'far_noise_reduce_60_linear_move.tof';
% filename = 'far_noise_reduce_60_rotate.tof';
% filename = 'cart_long_turn.tof';
% filename = 'cart_forward2.tof';
% filename = 'cart_forward.tof';
% filename = 'cart_rotate.tof';
% filename = 'cart2_very_long_path.tof';
% filename = 'cart2_medium_gravel_stop.tof';
% filename = 'cart2_u_turn.tof';

% Iterate through all tof files
filestat = dir('*.tof');
for ii=1:length(filestat)
    
    filename = filestat(ii).name;
    
    [path, fname, ext] = fileparts(filename);
    moviename = [fname '.mp4'];
    
    
    % Read metadata from file header
    fprintf('Reading images from %s...\n', filename);
    [fd, constants] = read_tof_header(filename);
    
    rows = constants.rows;
    cols = constants.cols;
    
    % Calculate camera focal length
    constants.fov_horizontal = 90    *pi/180;
    constants.fov_vertical   = 69.2  *pi/180;
    constants.fovh = constants.fov_horizontal;
    constants.fovv = constants.fov_vertical;
    fovh = constants.fovh;
    fovv = constants.fovv;
    f_length = cols / (2 * tan(0.5 * fovh));
    fps = constants.fps;
    num_frames = constants.num_frames;
    
    fprintf('Camera data captured %d frames at at %d fps\n', num_frames, fps);
    
    % Thresholding constants for image filtering
    ir_min_thresh = 30;
    ir_max_thresh = 4096;
    denoise_window = 6;
    denoise_threshold = .7;
    
    %% Display depth images in a movie
    
    % Bool to save movie or not
    save_movie = true;
    
    % Full screen figure window
    if ~save_movie
        hfig = figure(1);
        set(hfig, 'position', [0 0 1 1], 'units', 'normalized');
    end
    
    if save_movie
        v = VideoWriter(moviename, 'MPEG-4');
        v.FrameRate = constants.fps;
        open(v);
        vfig = figure(1);
    end
    
    min_depth = 2^31;
    max_depth = 0;
    
    exposure_remove = false;
    fast_movie = true;
    for frame_index = 1:num_frames
        
        % fprintf('Frame %d of %d\n', frame_index, num_frames);

        [depth, ir] = read_tof_frame(fd, constants.frame_size, constants);
        depth = double(depth) / 1000;
        
        % Clean noise from data
        depth = fuse_ir_depth(depth, ir, ir_max_thresh, ir_min_thresh, false);
        depth = depth_denoise(depth, f_length, denoise_window, denoise_threshold);
        
        % Generate point cloud from depth data
        points = depth2points(depth, f_length);
        
        vfig = figure(1);
        
        % Top left plot
        subplot(2, 2, 1);
        % Calculate global min/max
        newmin = min(min(depth));
        newmax = max(max(depth));
        if newmin < min_depth
            min_depth = newmin;
        end
        if newmax > max_depth
            max_depth = newmax;
        end
        imshownorm(depth, [min_depth max_depth]);
        xlabel('Depth', 'Color', 'w', 'FontWeight', 'bold');
        
        % Top right plot
        subplot(2, 2, 2);
        imshownorm(ir);
        xlabel('IR Intensity (normalized)', 'Color', 'w', 'FontWeight', 'bold');
        
        % Bottom left plot
        subplot(2, 2, 3);
        pcshow(points);
        title('Point Cloud (front)');
        view([0 0 -1]);
        xlabel('x (m)');
        ylabel('y (m)');
        zlabel('z (m)');
        axis([ -4 4 -4 3 0 6]);
        
        
        % Bottom right plot
        subplot(2, 2, 4);
        pcshow(points);
        title('Point Cloud (angled from above)');
        view([0 -1.8 -1]);
        xlabel('x (m)');
        ylabel('y (m)');
        zlabel('z (m)');
        axis([ -4 4 -4 3 0 6]);
        
        if exist('sgtitle', 'builtin') || exist('sgtitle', 'file')
            % Figure title
            sgtitle(['Frame ' num2str(frame_index) ' of ' num2str(num_frames) ...
                ' (' num2str(frame_index/constants.fps, '%.1f') '/' num2str(num_frames/constants.fps, '%.1f') ' s)'],...
                'Color', 'w');
        end
        
        
        if save_movie
            frame = getframe(vfig);
            writeVideo(v, frame);
        end
        if fast_movie
            pause(1/constants.fps/1000); % Very short delay
        else
            pause(1/constants.fps);
        end
        
    end
    
    if save_movie
        v.FrameRate;
        close(v);
        fprintf('Finished writing ''%s''\n', moviename);
    end
    
end

