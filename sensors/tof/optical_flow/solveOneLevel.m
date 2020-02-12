function [kai, est_cov] = solveOneLevel(pointCloudAvg,weights, constants)
% Input:
%    pointCloudAvg
%      Point cloud that is a spatial average of both inputs
%    weights
%      
%    constants
%      Structure of the camera parameters
%
% Output:
%    kai
%      Velocity state
%    est_cov
%      Covariance matrix

% Get dimensions of image
depth_dim = size(pointCloudAvg);
rows = depth_dim(1);
cols = depth_dim(2);

f_inv = cols/(2*tan(0.5*constants.fovh));

end