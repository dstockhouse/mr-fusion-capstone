%% Read depth and IR image data from file

clear; clc; close all;

% Ensure you downloaded the video file from 
% http://mercury.pr.erau.edu/~stockhod/samples/very_little_motion.tof
% and put it in this directory
filename = 'very_little_motion.tof';

[depth_frames, ir_frames, constants] = read_tof_file(filename);
rows = constants.rows;
cols = constants.cols;

fast_movie = true;

%% Display depth images in a movie
for ii = 1:constants.num_frames
    
    depth = reshape(depth_frames(ii, :, :), rows, cols);
    
    figure(1);
    imshownorm(depth);
    if fast_movie
        pause(1/constants.fps/1000); % Very short delay
    else
        pause(1/constants.fps);
    end
    
end

%% Display IR images in a movie
for ii = 1:constants.num_frames
    
    ir = reshape(ir_frames(ii, :, :), rows, cols);
    
    figure(2);
    imshownorm(ir);
    if fast_movie
        pause(1/constants.fps/1000); % Very short delay
    else
        pause(1/constants.fps);
    end
    
end
