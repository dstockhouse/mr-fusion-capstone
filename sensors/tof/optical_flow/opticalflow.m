% Optical Flow 
% Joy Fucella
% Start: 30 Jan 2020
% Last Mod: 6 Feb 2020
clear; clc;
% webcamlist
cam = webcam('PicoZense RGBD Camera'); % change to necessary cam

preview(cam);

%%
I0 = 0;
while(on)
    % Read image files
    depth = imread('sample5_depthmap.ppm');
    rgb = imread('sample1_rgb.bmp');
    rawpoints = dlmread('sample5_pointcloud.txt');
    rawpc = pointCloud(rawpoints);
    % imfinfo('')
    if I0 == 0
        I0 = I;
    elseif I0 == 1
        
    end
end
