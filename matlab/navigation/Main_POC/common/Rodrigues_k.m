function [C] = Rodrigues_k(k, theta)
% FUNCTION DESCRIPTION:
%   Implements the Rodrigues formula for constructing a DCM from an angle/axis
%   vector.  
%
% INPUTS:
%   k = 3X1 normalized axis of rotation vector = [k1 k2 k3] (dimensionless)
%   theta = angle of rotation about k (rad)
%
% OUTPUTS:
%   C   = 3X3 Rotation matrix C = exp(Skew(k)) = exp(K) (dimless)
%
% NOTES: Can accept k as a non normalized vector.  For this case do NOT
%       include theta (i.e. pass only 1 input argument)

if nargin == 1          % If only 1 input argument (k NOT normalized)
    theta = norm(k);
    k = k / theta; 
end

I3 = eye(3);            % 3X3 Identity matrix

K = vec2ss(k);      % Compute a skew-symmetric matrix from the vector k

% C = I + Sin(theta) K + (1-Cos(theta)) K^2  = Rodrigues formula
C = I3 + sin(theta) * K + (1 - cos(theta)) * K^2;
end

