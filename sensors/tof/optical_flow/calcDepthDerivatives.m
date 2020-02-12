function [du,dv,dt] = calcDepthDerivatives(pointCloudAvg,pointCloudOld, pointCloudNew, constants)
% Input:
%    pointCloudAvg
%      Point cloud that is a spatial average of both inputs
%    pointCloudOld
%      Previous point cloud (first to use in average)
%    pointCloudNew
%      New point cloud (second to use in average)
%    constants
%      Structure of the camera parameters
%
% Output:
%    du, dv, dt - derivatives

% Get dimensions of original image
depth_dim = size(pointCloudOld);
rows = depth_dim(1);
cols = depth_dim(2);

if size(pointCloudOld) ~= size(pointCloudNew)
    fprintf('Cannot average. Input point clouds are not the same length!!!');
    return
end
% Compute connectivity
for u = 1:cols-1
   for v = 1:rows-1
      if null(v,u) == false
          if u < cols-2
              rx_ninv(v,u) = sqrt((pointCloudAvg(v,u+1,1)+pointCloudAvg(v,u,1))^2 ...
                                  +(pointCloudAvg(v,u+1,3)+pointCloudAvg(v,u,3))^2);
          end
          if v < cols-2
              ry_ninv(v,u) = sqrt((pointCloudAvg(v+1,u,2)+pointCloudAvg(v,u,2))^2 ...
                                  +(pointCloudAvg(v+1,u,3)+pointCloudAvg(v,u,3))^2);
          end
      end
   end
end
% Spatial derivatives

end