%% Read depth and IR image data from file

clear; clc; close all;
addpath('../optical_flow');

% imagedir = 'tof_outdoor_robot_png1_05.04.2020';
imagedir = '../../../../mr_fusion_integrated_data/tof'
fprintf('Reading images from %s...\n', imagedir);

moviename = [imagedir '.mp4'];

% Iterate through all depth and ir files
depthfilestat = dir([imagedir '/*_depth.png']);
irfilestat = dir([imagedir '/*_ir.png']);
num_frames = min(length(depthfilestat), length(irfilestat));
final_metadata = sscanf(depthfilestat(num_frames).name, 'im%d_%f_depth.png');
final_timestamp = final_metadata(2);

% Camera parameters
constants.rows = 480;
constants.cols = 640;
rows = constants.rows;
cols = constants.cols;
constants.fov_horizontal = 90    *pi/180;
constants.fov_vertical   = 69.2  *pi/180;
constants.fovh = constants.fov_horizontal;
constants.fovv = constants.fov_vertical;
fovh = constants.fovh;
fovv = constants.fovv;
f_length = cols / (2 * tan(0.5 * fovh));
constants.fps = 8;
fps = constants.fps;
constants.num_frames = num_frames;

% Thresholding constants for image filtering
ir_min_thresh = 30;
ir_max_thresh = 4096;
denoise_window = 6;
denoise_threshold = .7;
    
% Flow control
exposure_remove = false;
fast_movie = true;
save_movie = true;

if save_movie
    v = VideoWriter(moviename, 'MPEG-4');
    v.FrameRate = constants.fps;
    open(v);
    vfig = figure(1);
end

% Iterate through every frame
for frame_index=1:num_frames
    
    fprintf('Frame %d of %d\n', frame_index, num_frames);
    
    depthfilename = depthfilestat(frame_index).name;
    irfilename = irfilestat(frame_index).name;
    
    depth_metadata = sscanf(depthfilename, 'im%d_%f_depth.png');
    ir_metadata = sscanf(irfilename, 'im%d_%f_ir.png');
    
    if depth_metadata ~= ir_metadata
        fprintf('  Error: frame metadata does not match: d[%d %.3f]; i[%d %.3f]\n', ...
            depth_metadata(1), depth_metadata(2), ir_metadata(1), ir_metadata(2))
    end
    
    image_id = depth_metadata(1);
    timestamp = depth_metadata(2);
    
    % Read image data
    depth = imread([imagedir '/' depthfilename]);
    ir = imread([imagedir '/' irfilename]);
    
    max_depth = 4.5;
    min_depth = 0;
    max_ir = 4096;
    min_ir = 0;
    
    % Rescale images
    depth = double(depth) * max_depth/65535;
    ir = double(ir) * max_ir/65535;
    
    
    %% Display depth images in a movie
    
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
            ' (' num2str(timestamp, '%.1f') '/' num2str(final_timestamp, '%.1f') ' s)'],...
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

