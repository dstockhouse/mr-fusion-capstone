function C = rotate_y(theta)
% FUNCTION DESCRIPTION:
%   Realize a rotation about the y-axis by an angle of theta
%
% INPUTS:
%   theta = rotation angle (rad)
%
% OUTPUTS:
%   C   = 3X3 Rotation matrix (dimless)
%
% NOTES:
%
C  = [ cos(theta),   0         ,  sin(theta) ; ...
       0         ,   1         ,  0          ; ...
      -sin(theta),   0         ,  cos(theta)];
end