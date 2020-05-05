function kai = trans2kai(trans)
% Converts an input SE<3> transformation matrix into a velocity vector 
%
% Inputs:
%   trans
%     Matrix with a rotation matrix of w and translation vector of v
%
% Outputs:
%   kai
%     Velocity vector of the form [vx vy vz wx wy wz]' where v is the
%     linear velocity and w is the angular velocity

% % Use logarithm in SE(3) to turn rigid transformation matrix into twist
% % vector
% % Adapted from http://ethaneade.com/lie.pdf and http://karnikram.info/blog/lie/

mat = logm(trans);
w = ss2vec(mat(1:3, 1:3));
v = mat(1:3, 4);
kai(1:3, 1) = v;
kai(4:6, 1) = w;

end
