function trans = kai2trans(kai)
% Converts an input velocity vector into a SE<3> transformation matrix
%
% Inputs:
%   kai
%     Velocity vector of the form [vx vy vz wx wy wz]' where v is the
%     linear velocity and w is the angular velocity
%
% Outputs:
%   trans
%     Matrix with a rotation matrix of w and translation vector of v

v = kai(1:3);
w = kai(4:6);

mat = zeros(4);
mat(1:3, 1:3) = vec2ss(w);
mat(1:3, 4) = v;

trans = expm(mat); % takes the exponent of the skew-symmetric matrix

end