%% Read depth and IR image data from file

clear; clc; close all;
addpath('../optical_flow');

% Ensure you downloaded one of the following video files from 
% http://mercury.pr.erau.edu/~stockhod/samples/[filename]
% and put it in an accessible directory
% filename = 'C:\Users\stockhod\Downloads\very_little_motion.tof';
% filename = 'C:\Users\stockhod\Downloads\med_rotate_left_then_right.tof';
% filename = 'C:\Users\stockhod\Downloads\far_rotate_right_then_left.tof';
% filename = 'C:\Users\stockhod\Downloads\medium_rotate_right_then_left.tof';
% filename = 'C:\Users\stockhod\Downloads\near_rotate_right_then_left.tof';
% filename = 'C:\Users\stockhod\Downloads\medium_noise_reduce_60_linear_move.tof';
% filename = 'C:\Users\stockhod\Downloads\medium_noise_reduce_60_rotate.tof';
% filename = 'C:\Users\stockhod\Downloads\far_noise_reduce_60_linear_move.tof';
% filename = 'C:\Users\stockhod\Downloads\far_noise_reduce_60_rotate.tof';
% filename = 'C:\Users\stockhod\Downloads\cart_long_turn.tof';
% filename = 'C:\Users\stockhod\Downloads\cart_forward2.tof';
% filename = 'C:\Users\stockhod\Downloads\cart_forward.tof';
% filename = 'C:\Users\stockhod\Downloads\cart_rotate.tof';
filename = 'cart2_very_long_path.tof';
% filename = 'C:\Users\stockhod\Downloads\cart2_medium_gravel_stop.tof';
% filename = 'C:\Users\stockhod\Downloads\cart2_u_turn.tof';


[path, fname, ext] = fileparts(filename);
moviename = [fname '.mp4'];

[depth_frames, ir_frames, constants] = read_tof_file(filename, 200000000);
rows = constants.rows;
cols = constants.cols;

% Calculate camera focal length
fov_horizontal = 90*pi/180;
fov_vertical = 69.2*pi/180;
f_length = cols / (2 * tan(0.5 * fov_horizontal));

fprintf('Read camera data: captured at %d fps\n', constants.fps);


%% Limit number of frames
% num_frames = 235;
num_frames = constants.num_frames;
if num_frames > constants.num_frames
    num_frames = constants.num_frames;
end
fprintf('Processing %d of %d frames (%.1f/%.1f)\n', ...
    num_frames, constants.num_frames, ...
    num_frames/constants.fps, constants.num_frames/constants.fps);


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

exposure_remove = false;
fast_movie = true;
for ii = 1:num_frames
    
    fprintf('Frame %d of %d\n', ii, num_frames);
    
    depth = reshape(depth_frames(ii, :, :), rows, cols);
    ir = reshape(ir_frames(ii, :, :), rows, cols);
    if exposure_remove
        ir((ir < 40) | (ir > 4090)) = 0;
    end
    % Filter depth map
    depth = fuse_ir_depth(depth, ir, 4090, 40);
    
    % Generate point cloud from depth data
    points = depth2points(double(depth)/1000, f_length);
    
    vfig = figure(1);
    
    % Top left plot
    subplot(2, 2, 1);
%     subplot(2, 1, 1);
    imshownorm(depth, [min(min(min(depth_frames))) max(max(max(depth_frames)))]);
%     imshow(depth/max(max(max(depth_frames))));
%     fprintf('frame %d depth max: %d\n', ii, max(max(depth)));
    xlabel('Depth', 'Color', 'w', 'FontWeight', 'bold');
    
    % Top right plot
    subplot(2, 2, 2);
    imshownorm(ir);
%     imshow(ir/max(max(max(ir_frames))));
%     fprintf('frame %d IR max: %d\n', ii, max(max(ir)));
    xlabel('IR Intensity', 'Color', 'w', 'FontWeight', 'bold');
    
    % Bottom left plot
    subplot(2, 2, 3);
%     pcshow(points);
    cutoff_threshold = .7;
%     p_select = points(:,:,3) < (max(max(points(:,:,3)))*cutoff_threshold);
%     p_selected = points(reshape([p_select(:,:) p_select(:,:) p_select(:,:)], rows, cols, 3));
%     p_exclude = reshape(p_selected, length(p_selected)/3, 3);
%     fprintf('%d valid points\n', length(p_selected)/3);
%     p_exclude = points;
%     p_colors = zeros(size(points));
%     p_colors = reshape([depth.*(depth>0) depth.*(depth>0) depth.*(depth>0)], rows, cols, 3);
    p_exclude = points;
%     p_colors = reshape(depth.*(depth>0), rows*cols, 1)/4096;
    pcshow(p_exclude);
    title('Point Cloud (front)');
    view([0 0 -1]);
    xlabel('x (m)');
    ylabel('y (m)');
    zlabel('z (m)');
    axis([ -4 4 -4 3 0 6]);
    
    
    % Bottom right plot
    subplot(2, 2, 4);
%     pcshow(points);
    pcshow(p_exclude);
    title('Point Cloud (angled from above)');
    view([0 -1.8 -1]);
    xlabel('x (m)');
    ylabel('y (m)');
    zlabel('z (m)');
    axis([ -4 4 -4 3 0 6]);
    
    if exist('sgtitle', 'builtin')
        % Figure title
        sgtitle(['Frame ' num2str(ii) ' of ' num2str(num_frames) ...
            ' (' num2str(ii/constants.fps, '%.1f') '/' num2str(num_frames/constants.fps, '%.1f') ' s)'],...
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

% %% Display Point clouds in a movie
% for ii = 1:constants.num_frames
%     
%     depth = double(reshape(depth_frames(ii, :, :), rows, cols))/1000;
%     points = depth2points(depth, f_length);
%     
%     figure(2);
%     pcshow(points);
%     view([0 0 -1])
%     xlabel('x (m)');
%     ylabel('y (m)');
%     zlabel('z (m)');
%     if fast_movie
%         pause(1/constants.fps/1000); % Very short delay
%     else
%         pause(1/constants.fps);
%     end
%     
% end
% 
% %% Display IR images in a movie
% for ii = 1:constants.num_frames
%     
%     ir = reshape(ir_frames(ii, :, :), rows, cols);
%     
%     figure(3);
%     imshownorm(ir);
%     if fast_movie
%         pause(1/constants.fps/1000); % Very short delay
%     else
%         pause(1/constants.fps);
%     end
%     
% end
