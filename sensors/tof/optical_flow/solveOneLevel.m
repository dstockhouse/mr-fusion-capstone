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

f = cols/(2*tan(0.5*constants.fovh));
cont = 1;

A = zeros(numValidPoints, 6);
B = zeros(numValidPoints, 1);

for u = 2:cols-1
    for v = 2:rows-1
        
        % Only compute if point has nonzero depth (real point)
        if pointCloudAvg(v,u,3) > .01
            d = pointCloudAvg(v,u,3);
            inv_d = 1/d;
            x = pointCloudAvg(v,u,1);
            y = pointCloudAvg(v,u,2);
            dycomp = du(v,u) * f * inv_d;
            dzcomp = dv(v,u) * f * inv_d;
            tw = weights(v,u);
            
            % Fill matrix
            % This fills in the order (vz, vx, vy, wz, wx, wy)
            % The order may need to be adjusted for the future, but for now axes
            % will just be mislabeled
            % vz
            A(cont,1) = tw*(1 + dycomp*x*inv_d + dzcomp*y*inv_d);
            % vx
            A(cont,2) = tw*(-dycomp);
            % vy
            A(cont,3) = tw*(-dzcomp);
            
            % wz
            A(cont,4) = tw*(dycomp*y - dzcomp*x);
            % wx
            A(cont,5) = tw*(y + dycomp*inv_d*y*x + dzcomp*(y*y*inv_d + d));
            % wy
            A(cont,6) = tw*(-x - dycomp*(x*x*inv_d + d) - dzcomp*inv_d*y*x);
            
            % -Z_t
            B(cont,1) = tw*(-dt(v,u));
            cont = cont+1;
        end
    end
end

% Restrict to get rid of the points on the edges (excluded by loop above)
A = A(1:cont-1,:);
B = B(1:cont-1,:);

% Solve the linear system of equations using weighted least squares
AtA = A' * A;
AtB = A' * B;
kai = A \ B; % Update Velocity

% Covariance matrix calc
res = -B;
for k=1:6
    res = res + kai(k) * A(:,k);
end
est_cov = (1/(numValidPoints - 6))*inv(AtA)*norm(res,2)^2; % Might just be norm(res)^2

end
