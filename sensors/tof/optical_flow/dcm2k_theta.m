function [k, theta] = dcm2k_theta(C)
% Function Description:
%   Converts a 3X3 Direction cosine matrix C to an angle and axis of rotation
%
% INPUTS:
%   C = 3x3 direction cosine matrix (dimensionless)
%
% OUTPUTS:
%   k = 3X1 normalized axis of rotation vector = [k1 k2 k3] (dimensionless)
%   theta = angle of rotation about k (rad)
%
% NOTES:
%   - The algorithm can become ill-conditioned if theta becomes "too small"
%   - Using MATLAB defined eps = 2.2204e-016 as ~ zero

k  = [0; 0; 0];                 % Initialize the axis of rotation (set to 0)

Tr  = C(1,1)+C(2,2)+C(3,3);     % Trace(C) = 1 + 2Cos(theta)

if(Tr >= (3-eps))               % degenerate case Tr=3 => theta = 0    
    theta = 0;                  % Angle of rotation is zero
elseif (Tr <= (-1+eps))         % degenerate case Tr=-1 => theta = Pi
    [Mx,i] = max([C(1,1),C(2,2),C(3,3)]);  % Compute the maximum element of k
    switch i
        case 1  % if( Mx == C(1,1) )  % Use column 1
            k(1) = sqrt((C(1,1) + 1.0) / 2.0);
            k(2) = C(2,1)/(2*k(1));
            k(3) = C(3,1)/(2*k(1));
        case 2 % if ( Mx == C(2,2) )  % Use column 2
            k(2) = sqrt((C(2,2) + 1.0) / 2.0);
            k(1) = C(1,2)/(2*k(2));
            k(3) = C(3,2)/(2*k(2));
        case 3 % if ( Mx == C(3,3) )  % Use column 3
            k(3) = sqrt((C(3,3) + 1.0) / 2.0);
            k(1) = C(1,3)/(2*k(3));
            k(2) = C(2,3)/(2*k(3));
    end
    theta = pi;
else    % Neither degenerate case: if -1 < trace(C) < 3
    theta = acos((Tr-1)/2);
    tmp = 1/(2 * sin(theta));
    k(1)    = (C(3,2) - C(2,3)) * tmp;
    k(2)    = (C(1,3) - C(3,1)) * tmp;
    k(3)    = (C(2,1) - C(1,2)) * tmp;
end