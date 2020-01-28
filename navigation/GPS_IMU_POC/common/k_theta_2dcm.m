function C = k_theta_2dcm(k, theta)
% Function Description:
%   Converts a 3X1 angle-axis vector k to a 3X3 DCM matrix
%
% INPUTS:
%   k = 3X1 normalized axis of rotation vector = [k1 k2 k3] (dimensionless)
%   theta = angle of rotation about k (rad)
%   
% OUTPUTS:
%   C = 3x3 direction cosine matrix
%
% NOTES:
%   - Using MATLAB defined eps = 2.2204e-016 as ~ zero

if (abs(theta) > eps)            % Check for degenerate case: Angle of rotation Non zero
    Ct = cos(theta);
    St = sin(theta);
    V  = 1-Ct;
    
    C  = [  k(1)*k(1)*V + Ct,     k(1)*k(2)*V-k(3)*St,  k(1)*k(3)*V+k(2)*St;
            k(1)*k(2)*V+k(3)*St,  k(2)*k(2)*V + Ct,     k(2)*k(3)*V-k(1)*St;
            k(1)*k(3)*V-k(2)*St,  k(2)*k(3)*V+k(1)*St,  k(3)*k(3)*V + Ct    ];
else
    C = [1,0,0;            % Degenerate case => theta = 0, or 2 n Pi
         0,1,0;
         0,0,1];
end
end % Close function