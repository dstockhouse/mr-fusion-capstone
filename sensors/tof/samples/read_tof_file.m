function [depth_frames, ir_frames, constants] = read_tof_file(filename)
% Reads a *.tof file captured by the Arrow ToF SDK.
%
% Note: File format is described at 
% https://github.com/analogdevicesinc/aditof_sdk/tree/master/examples/aditof-demo
%
% Input:
%   filename
%     Name of *.tof file to be read
% Output:
%   depth_frames
%     Image data in a N-by-rows-by-cols matrix
%   constants
%     Struct containing resolution, FOV, and FPS information for camera
%     data

filename = 'very_little_motion.tof';

% Read contents of file
fd = fopen(filename);
raw_data = fread(fd);
fclose(fd);

%% Parse raw metadata into important fields
field_len = 4;
fpos = 1;

% Parse height from little-endian bytes
raw_height = raw_data(fpos:fpos+field_len);
height = 0;
for ii=1:field_len
    height = height + raw_height(ii) * 2^((ii-1)*8);
end
fpos = fpos + field_len;

% Parse width from little-endian bytes
raw_width = raw_data(fpos:fpos+field_len);
width = 0;
for ii=1:field_len
    width = width + raw_width(ii) * 2^((ii-1)*8);
end
fpos = fpos + field_len;

% Parse fps from little-endian bytes
raw_fps = raw_data(fpos:fpos+field_len);
fps = 0;
for ii=1:field_len
    fps = fps + raw_fps(ii) * 2^((ii-1)*8);
end
fpos = fpos + field_len;

% More metadata
num_frames = (length(raw_data) - field_len * 3) / (width*height) / 2;
duration = num_frames / fps;

% Height read from file is actually twice the pixel height. It stores a depth
% frame followed by IR frame, so each frame is effectively doubled
rows = height / 2;
cols = width;

%% Now read raw frame data
depth_frames = zeros(num_frames, rows, cols);
ir_frames = zeros(num_frames, rows, cols);
frame = zeros(rows, cols);
for frame_index = 1:num_frames
    
    % First read depth data sequentially
    % Read entire row first, then col
    for v = 1:rows
        for u = 1:cols
            frame(v, u) = raw_data(fpos) + raw_data(fpos+1)*256;
            fpos = fpos + 2;
        end
    end
    depth_frames(frame_index,:,:) = frame;
    
    % Then read IR data sequentially
    % Read entire row first, then col
    for v = 1:rows
        for u = 1:cols
            frame(v, u) = raw_data(fpos) + raw_data(fpos+1)*256;
            fpos = fpos + 2;
        end
    end
    ir_frames(frame_index,:,:) = frame;
end


%% Package metadata into constants
constants.fps = fps;
constants.rows = rows;
constants.cols = cols;
constants.num_frames = num_frames;
constants.duration = duration;

end