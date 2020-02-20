function [weights] = compute_weights(pointCloudOld, pointCloudNew, pointCloudAvg, differentials, kai_old, cumulativeTransformation, constants)
% Input:
%    pointCloudOld
%      Previous point cloud (first to use in average)
%    pointCloudNew
%      New point cloud (second to use in average)
%    pointCloudAvg
%      Point cloud that is a spatial average of both inputs
%    pointCloudAvg
%      Point cloud that is a spatial average of both inputs
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

% Perform this log in SE(3) ??? 
% Still figuring this out
transformationVector = log(cumulativeTransformation) * constants.fps;

% kai = velocity state [vx, vy, vz, wx, wy, wz]'
kai_level = kai_old - transformationVector;

% Split up the input differentials
du = differentials(:,:,1);
dv = differentials(:,:,2);
dt = differentials(:,:,3);

% Set up some constant parameters for error
% Measurement
focal_length = 2 * tan(0.5 * constants.fovh) / cols;
f = focal_length;
kz2 = (1.425e-5)^2 / 25

% Linearization
kduv = 20e-5;
kdt = kduv / constants.fps^2;
k2duv = 5e-6;
k2dt = 5e-6;

% Preallocation
weights = zeros(rows, cols);

% Step through each pixel
for u = 2:(cols-1)
    for v = 2:(rows-1)

        x = pointCloudAvg(v, u, 1)
        y = pointCloudAvg(v, u, 2)
        z = pointCloudAvg(v, u, 3)


        inv_d = 1/z;
        z2 = z^2;
        z4 = z^4;


        %% Measurement error
        % I honestly have no idea

        var44 = kz2 * z4 * fps^2;
        var55 = kz2 * z4 * 0.25;
        var66 = var55;

        j4 = 1;
        j5 = x / z^2 / f * (kai_level(3) + y*kai_level(4)*kai_level(5)) + ...
        (-kai_level(1) - z*kai_level(5) + y*kai_level(6)) / d / f;
        j5 = y / z^2 / f * (kai_level(3) + y*kai_level(4)*kai_level(5)) + ...
        (-kai_level(2) - z*kai_level(4) + x*kai_level(6)) / d / f;

        sigma_m = j4^2 * var44 + j5^2 * var55 + j6^2 * var66;


        %% Linearization error
        % du_old = ini_du
        % du_new = final_du
        du_old = pointCloudOld(v, u + 1, 3) - pointCloudOld(v, u - 1, 3)
        dv_old = pointCloudOld(v + 1, u, 3) - pointCloudOld(v - 1, u, 3)
        du_new = pointCloudNew(v, u + 1, 3) - pointCloudNew(v, u - 1, 3)
        dv_new = pointCloudNew(v + 1, u, 3) - pointCloudNew(v - 1, u, 3)

        dut = du_old - du_new;
        dvt = dv_old - dv_new;

        duu = du(v, u + 1) - du(v, u - 1);
        dvv = dv(v + 1, u) - dv(v - 1, u);

        dvu = dv(v, u + 1) - dv(v, u - 1);
        duv = du(v + 1, u) - du(v - 1, u);

        sigma_l = kdt * dt(v, u)^2 + kduv * (du(v, u)^2 + dt(v, u)^2) + ...
        k2dt * (dut^2 + dvt^2) + k2duv * (duu^2 + dvv^2 + dvu^2);

        weights(v, u) = sqrt(1/(sigma_m + sigma_l));

   end
end

% Normalize weights to the range [0,1]
weights = weights / max(weights);

end

