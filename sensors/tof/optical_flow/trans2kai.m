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

% % Get linear and rotational parts from transformation matrix
% rot_mat = trans(1:3, 1:3);
% lin_trans = reshape(trans(1:3, 4), 3, 1);
% 
% % Use logarithm in SE(3) to turn rigid transformation matrix into twist
% % vector
% % Adapted from http://ethaneade.com/lie.pdf and http://karnikram.info/blog/lie/
% 
% theta = acos((trace(rot_mat) - 1)/2);
% if abs(theta) < 0.0000001
%     w = zeros(3,1);
%     v = lin_trans;
% else
%     w_pre = [rot_mat(3,2) - rot_mat(2,3);
%              rot_mat(1,3) - rot_mat(3,1);
%              rot_mat(2,1) - rot_mat(1,2)];
%     w = theta / (2*sin(theta)) * w_pre;
%     w_cross = vec2ss(w);
%     
%     V_inv = eye(3) - w_cross/2 + ...
%         w_cross^2 * (1 - theta*sin(theta)/(2*(1-cos(theta)))) / theta^2;
%     v = V_inv * lin_trans;
% end
% 

mat = logm(trans);
w = ss2vec(mat(1:3, 1:3));
v = mat(1:3, 4);
kai(1:3, 1) = v;
kai(4:6, 1) = w;

end
