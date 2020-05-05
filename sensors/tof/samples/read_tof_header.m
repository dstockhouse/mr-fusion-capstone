function [fd, constants] = read_tof_header(filename)

fileinfo = dir(filename);
filelength = fileinfo.bytes;

% Read contents of file
fd = fopen(filename);
header_length = 12;
header_data = fread(fd, header_length);

% leave file open to return fd for subsequent reads


%% Parse raw metadata into important fields
field_len = 4;
fpos = 1;

% Parse height from little-endian bytes
raw_height = header_data(fpos:fpos+field_len-1);
height = 0;
for ii=1:field_len
    height = height + raw_height(ii) * 2^((ii-1)*8);
end
fpos = fpos + field_len;

% Parse width from little-endian bytes
raw_width = header_data(fpos:fpos+field_len-1);
width = 0;
for ii=1:field_len
    width = width + raw_width(ii) * 2^((ii-1)*8);
end
fpos = fpos + field_len;

% Parse fps from little-endian bytes
raw_fps = header_data(fpos:fpos+field_len-1);
fps = 0;
for ii=1:field_len
    fps = fps + raw_fps(ii) * 2^((ii-1)*8);
end
fpos = fpos + field_len;

% More metadata
num_frames = floor((filelength - field_len * 3) / (width*height) / 2);
duration = num_frames / fps;

% Height read from file is actually twice the pixel height. It stores a depth
% frame followed by IR frame, so each frame is effectively doubled
rows = height / 2;
cols = width;

% Save all return values in single structure
constants.fps = fps;
constants.rows = rows;
constants.cols = cols;
constants.num_frames = num_frames;
constants.frame_size = width*height*2;
constants.duration = duration;

end
