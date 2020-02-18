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
du = zeros(rows,cols);
dv = zeros(rows,cols);
dt = zeros(rows,cols);
for v = 1:rows-1
    for u = 2:cols-2
        if null(v,u) == false
            du(v,u) = (rx_ninv(v,u-1)*(pointCloudAvg(v,u+1,3)+pointCloudAvg(v,u,3))...
                + rx_ninv(v,u)*(pointCloudAvg(v,u,3)+pointCloudAvg(v,u-1,3)))...
                /(rx_ninv(v,u)+rx_ninv(v,u-1));
        end
        du(v,1) = du(v,2);
        du(v, cols-1) = du(v,cols-2);
    end
end
for u = 1:cols-1
    for v = 2:rows-2
        if null(v,u) == false
            dv(v,u) = (ry_ninv(v,u-1)*(pointCloudAvg(v+1,u,3)+pointCloudAvg(v,u,3))...
                + ry_ninv(v,u)*(pointCloudAvg(v,u,3)+pointCloudAvg(v-1,u,3)))...
                /(ry_ninv(v,u)+ry_ninv(v-1,u));
        end
        dv(1,u) = dv(2,u);
        dv(rows-1,u) = dv(rows-2,u);
    end
end
% Temporal derivative
for u = 1:cols-1
    for v = 1:rows-1
        if null(v,u) == false
            dt(v,u) = constants.fps * (pointCloudAvg(v,u)-pointCloudOld(v,u));
        end
    end
end
end