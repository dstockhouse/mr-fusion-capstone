function [weights] = compute_weights(pointCloudOld, pointCloudNew, pointCloudAvg, differentials, kai_est, cumulativeTransformation, constants)
% Input:
%    pointCloudOld
%      Previous point cloud (first to use in average)
%    pointCloudNew
%      New point cloud (second to use in average)
%    pointCloudAvg
%      Point cloud that is a spatial average of both inputs
%    differentials
%      rows-by-cols-by-3 array of du,dv,dt differentials at each pixel
%    constants
%      Structure of the camera parameters
%
% Output:
%    weights
%      Matrix containing the weighted least squares solution weights 
%

depth_dim = size(pointCloudOld);
rows = depth_dim(1);
cols = depth_dim(2);

% Unpack SE(3) rigid transformation into se(3) vector
transformationVector = trans2kai(cumulativeTransformation * constants.fps);

% Split up the input differentials
du = differentials(:,:,1);
dv = differentials(:,:,2);
dt = differentials(:,:,3);

% Set up some constant parameters for error
% Measurement
focal_length = 2 * tan(0.5 * constants.fovh) / cols;
f = focal_length;

% Error (approximation) constants
% kduv = 20e-5;
% kdt = kduv / constants.fps^2;
kduvt = 200;

% k2duv = 5e-6;
k2duv = 5;

% k2dt = 5e-6;
k2dt = 5;

% kz2 = (1.425e-5)^2 / 25; % One approximation
kz2 = 0.01;

% Preallocation
weights = zeros(rows, cols);

% Step through each pixel
for u = 2:(cols-1)
    for v = 2:(rows-1)
        
        % Only compute if it has a nonzero depth value at the point
        if pointCloudAvg(v, u, 3) > 0.01
            
            x = pointCloudAvg(v, u, 1);
            y = pointCloudAvg(v, u, 2);
            z = pointCloudAvg(v, u, 3);
            
            z2 = z^2;
            z4 = z2^2;
            
            %% Measurement error
            % Easier method
            sigma_m = kz2 * z^4;
            
            %% Linearization error
            % Renamed from example code:
            %   du_old -> ini_du
            %   du_new -> final_du
            du_old = pointCloudOld(v, u + 1, 3) - pointCloudOld(v, u - 1, 3);
            dv_old = pointCloudOld(v + 1, u, 3) - pointCloudOld(v - 1, u, 3);
            du_new = pointCloudNew(v, u + 1, 3) - pointCloudNew(v, u - 1, 3);
            dv_new = pointCloudNew(v + 1, u, 3) - pointCloudNew(v - 1, u, 3);
            
            dut = du_old - du_new;
            dvt = dv_old - dv_new;
            
            duu = du(v, u + 1) - du(v, u - 1);
            dvv = dv(v + 1, u) - dv(v - 1, u);
            
            dvu = dv(v, u + 1) - dv(v, u - 1);
            
            sigma_l =...
                kduvt * (dt(v, u)^2 +du(v, u)^2 + dv(v, u)^2) + ...
                k2dt * (dut^2 + dvt^2) +...
                k2duv * (duu^2 + dvv^2 + dvu^2);

            weights(v, u) = sqrt(1/(sigma_m + sigma_l));
            
        end
   end
end

% Normalize weights to the range [0,1]
weights = weights / max(max(weights));

end

