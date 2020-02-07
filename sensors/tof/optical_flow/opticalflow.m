% Optical Flow 
% Joy Fucella
% Start: 30 Jan 2020
% Last Mod: 6 Feb 2020
clear; clc;
% webcamlist
cam = webcam('PicoZense RGBD Camera'); % change to necessary cam

preview(cam);

%%
clc;
% Read image files
depth = imread('samples\startpoint_depthmap.ppm');
rgb = imread('samples\startpoint_rgb.bmp');
rawpoints = dlmread('samples\startpoint_pointcloud.txt');
rawpc = pointCloud(rawpoints);

depth1 = imread('samples\move10cm_depthmap.ppm');
rgb1 = imread('samples\move10cm_rgb.bmp');
rawpoints1 = dlmread('samples\move10cm_pointcloud.txt');
rawpc1 = pointCloud(rawpoints1);

depth2 = imread('samples\rotate30deg_depthmap.ppm');
rgb2 = imread('samples\rotate30deg_rgb.bmp');
rawpoints2 = dlmread('samples\rotate30deg_pointcloud.txt');
rawpc2 = pointCloud(rawpoints2);

pos1 = rgb2hsv(rgb);
pos2 = rgb2hsv(rgb1);

pos1 = pos1(:,:,3);
pos2 = pos2(:,:,3);

diff1 = abs(pos1-pos2);

Ithresh = diff1 > 0.02;

SE = strel('disk',5,8);
BW = imopen(Ithresh,SE);
BW = imclose(BW,SE);

[labels,number] = bwlabel(BW,8);

Istats = regionprops(labels,'basic','Centroid');

[maxVal, maxIndex] = max([Istats.Area]);

imshow(BW,'Border','tight');
