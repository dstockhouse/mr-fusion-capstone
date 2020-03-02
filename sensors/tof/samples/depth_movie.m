%% Read depth and IR image data from file

clear; clc; close all;
addpath('../optical_flow');

% Ensure you downloaded one of the following video files from 
% http://mercury.pr.erau.edu/~stockhod/samples/very_little_motion.tof
% and put it in this directory
% filename = 'very_little_motion.tof';
% filename = 'E:\med_rotate_left_then_right.tof';
filename = 'C:\Users\stockhod\Downloads\far_rotate_left_then_right.tof';
moviename = 'far_rotate_left_then_right';
moviename = [moviename '.mp4'];

[depth_frames, ir_frames, constants] = read_tof_file(filename, 500000000);
rows = constants.rows;
cols = constants.cols;

% Calculate camera focal length
fov_horizontal = 90*pi/180;
fov_vertical = 69.2*pi/180;
f_length = cols / (2 * tan(0.5 * fov_horizontal));

fprintf('Read camera data: captured at %d fps\n', constants.fps);

num_frames = 275;
fprintf('Processing %d of %d frames (%.1f/%.1f)\n', ...
    num_frames, constants.num_frames, ...
    num_frames/constants.fps, constants.num_frames/constants.fps);

%% Display depth images in a movie

% Full screen figure window
% hfig = figure(1);
% set(hfig, 'position', [0 0 1 1], 'units', 'normalized');

save_movie = true;
if save_movie
    v = VideoWriter(moviename, 'MPEG-4');
    v.FrameRate = constants.fps;
    open(v);
    vfig = figure(1);
end

fast_movie = true;
for ii = 1:num_frames
    
    depth = reshape(depth_frames(ii, :, :), rows, cols);
    ir = reshape(ir_frames(ii, :, :), rows, cols);
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
    imshownorm(ir, [0 4095]);
%     imshow(ir/max(max(max(ir_frames))));
%     fprintf('frame %d IR max: %d\n', ii, max(max(ir)));
    xlabel('IR Intensity', 'Color', 'w', 'FontWeight', 'bold');
    
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
    
    % Figure title
    sgtitle(['Frame ' num2str(ii) ' of ' num2str(num_frames) ...
        ' (' num2str(ii/constants.fps, '%.1f') '/' num2str(num_frames/constants.fps, '%.1f') ' s)'],...
        'Color', 'w');
    
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
