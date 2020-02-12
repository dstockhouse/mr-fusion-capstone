function [pointCloudAvg, numValidPoints] = average_point_clouds(pointCloudOld, pointCloudNew, constants)
% Input:
%    pointCloudOld
%      Previous point cloud (first to use in average)
%    pointCloudNew
%      New point cloud (second to use in average)
%    constants
%      Structure of the camera parameters
%
% Output:
%    pointCloudAvg
%      Point cloud that is a spatial average of both inputs
%    numValidPoints
%      Number of points that have nonzero depth
%

% Get dimensions of original image
depth_dim = size(pointCloud);
rows = depth_dim(1);
cols = depth_dim(2);
focal_pt = cols / (2*tan(0.5*constants.fovh));

if size(pointCloudOld) != size(pointCloudNew)
    fprintf('Cannot average. Input point clouds are not the same length!!!');
    return
end

pointCloudAvg = zeros(size(pointCloudOld));
numValidPoints = 0;

% Step through each pixel
for jj=1:(cols-1)
   for ii = 1:(rows-1)

       % Only average if both have positive depth
       if pointCloudOld(jj, ii, 3) > 0 && pointCloudNew(jj, ii, 3) > 0

           pointCloudAvg(jj, ii, 1) = (pointCloudOld(jj, ii, 1) + pointCloudNew(jj, ii, 1)) / 2;
           pointCloudAvg(jj, ii, 2) = (pointCloudOld(jj, ii, 2) + pointCloudNew(jj, ii, 2)) / 2;
           pointCloudAvg(jj, ii, 3) = (pointCloudOld(jj, ii, 3) + pointCloudNew(jj, ii, 3)) / 2;

           numValidPoints = numValidPoints + 1;
       end

   end
end

end

