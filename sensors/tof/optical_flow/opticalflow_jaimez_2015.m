% Optical Flow using range cameras from Jaimez (2015)
% Joy Fucella and David Stockhouse

clear; clc; close all;

% webcamlist
% cam = webcam('PicoZense RGBD Camera'); % change to necessary cam
% 
% preview(cam);

% Read image files
start_depth = imread('samples\startpoint_depthmap.ppm');
start_rgb = imread('samples\startpoint_rgb.bmp');
start_rawpoints = dlmread('samples\startpoint_pointcloud.txt');
% start_rawpc = pointCloud(rawpoints);

move_depth = imread('samples\move10cm_depthmap.ppm');
move_rgb = imread('samples\move10cm_rgb.bmp');
move_rawpoints = dlmread('samples\move10cm_pointcloud.txt');
% move_rawpc = pointCloud(rawpoints1);

rotate_depth = imread('samples\rotate30deg_depthmap.ppm');
rotate_rgb = imread('samples\rotate30deg_rgb.bmp');
rotate_rawpoints = dlmread('samples\rotate30deg_pointcloud.txt');
% rotate_rawpc = pointCloud(rawpoints2);


%
%
% The process for computing the optical flow (in Jaimez 2015) is as follows:
%
%   Build the Gaussian Pyramid
%
%   For each pyramid step starting with "highest level" (lowest resolution)
%
%      Warp pyramid points to align with optical flow of previous level (if not first level)
%
%      Generate point cloud at pixel indices, nullify 0-depth points
%
%      Compute depth derivatives
%
%      Compute uncertainty weighting (for weighted least squares)
%
%      Solve weighted least squares linear equation for velocity state
%
%      Filter velocity with position kinematics
%
%   Compute 
%

% Set some constants
constants.fov_horizontal = 69 *pi/180;
constants.fov_vertical   = 51 *pi/180;

%% Gaussian pyramid

gaussian_levels = 4;
[p_depth, p_points] = gaussian_pyramid(start_depth,...
	gaussian_levels,...
	constants);

% Plot the pyramid
normimage = mat2gray(start_depth);
figure(1);
imshow(normimage);

cols = 640;
rows = 360;

for ii = 1:gaussian_levels

    clear normimage;
	normimage(1:rows,1:cols) = mat2gray(p_depth(ii, 1:rows, 1:cols));
	figure(1+ii);
	imshow(normimage);
    
    cols = cols/2;
    rows = rows/2;

end

% Iterate through the gaussian pyramid
for image_level = gaussian_levels:-1:1

	%% Warp Image

	% Don't warp if top of pyramid
	if image_level == gaussian_levels
		% Copy top leve
	end

	%% Generate point cloud

	%% Compute depth derivatives

	%% Compute uncertainties & weighting

	%% Solve weighted least squares

	%% Filter velocity

end

