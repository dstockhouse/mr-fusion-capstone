function [depth_frame, ir_frame] = read_tof_frame(fd, num_bytes, constants)

rows = constants.rows;
cols = constants.cols;

raw_data = fread(fd, num_bytes);
fpos = 1;

% Read raw frame data
frame = zeros(rows, cols);

% First read depth data sequentially
% Read entire row first, then col
for v = 1:rows
    for u = 1:cols
        frame(v, u) = raw_data(fpos) + raw_data(fpos+1)*256;
        fpos = fpos + 2;
    end
end
depth_frame = frame;

% Then read IR data sequentially
% Read entire row first, then col
for v = 1:rows
    for u = 1:cols
        frame(v, u) = raw_data(fpos) + raw_data(fpos+1)*256;
        fpos = fpos + 2;
    end
end
ir_frame = frame;

end
