function [kai, est_cov] = solveOneLevel(pointCloudAvg,weights,du,dv,dt, numValidPoints, constants)
% Input:
%    pointCloudAvg
%      Point cloud that is a spatial average of both inputs
%    weights
%      
%    constants
%      Structure of the camera parameters
%    du, dv, dt - derivatives
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
cont = 1;

A = zeros(numValidPoints, 6);
B = zeros(numValidPoints, 1);

for u = 2:cols-1
    for v = 2:rows-1
        if null(v,u) == false
            d = pointCloudAvg(v,u,3);
            inv_d = 1/d;
            x = pointCloudAvg(v,u,1);
            y = pointCloudAvg(v,u,2);
            dycomp = du(v,u) * f_inv * inv_d;
            dzcomp = dv(v,u) * f_inv * inv_d;
            tw = weights(v,u);
            
            % Fill matrix
            A(cont,1) = tw*(1 + dycomp*x*inv_d + dzcomp*y*inv_d);
            A(cont,2) = tw*(-dycomp);
            A(cont,3) = tw*(-dzcomp);
            A(cont,4) = tw*(dycomp*y - dzcomp*x);
            A(cont,5) = tw*(y + dycomp*inv_d*y*x + dzcomp*(y*y*inv_d + d));
            A(cont,6) = tw*(-x - dycomp*(x*x*inv_d + d) - dzcomp*inv_d*y*x);
            B(cont,1) = tw*(-dt(v,u));
            cont = cont+1;
        end
    end
end
% Solve the linear system of equations using weighted least squares
AtA = A' .* A;
AtB = A' .* B;
var = linsolve(AtA,AtB);

% Covariance matrix calc
res = -B;
for k=1:6
    res = res + var(k) * A(:,k);
end
est_cov = (1/(num_valid_points - 6))*inv(AtA)*norm(res);

% Update velocity
kai = var;

end