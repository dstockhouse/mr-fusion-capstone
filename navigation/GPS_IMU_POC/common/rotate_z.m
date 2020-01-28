function C = rotate_z(theta)
% FUNCTION DESCRIPTION:
%   Realize a rotation about the z-axis by an angle of theta
%
% INPUTS:
%   theta = rotation angle (rad)
%
% OUTPUTS:
%   C   = 3X3 Rotation matrix (dimless)
%
% NOTES:
%
C  = [ cos(theta), -sin(theta), 0; ...
       sin(theta),  cos(theta), 0; ...
       0         ,  0         , 1];
end