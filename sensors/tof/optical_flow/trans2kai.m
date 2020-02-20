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

% Get linear and rotational parts from transformation matrix
rot_mat = trans(1:3, 1:3);
lin_trans = reshape(trans(1:3, 4), 3, 1);

% Use logarithm in SE(3) to turn rigid transformation matrix into twist
% vector
% Adapted from http://ethaneade.com/lie.pdf and http://karnikram.info/blog/lie/

theta = acos((trace(R) - 1)/2);
if abs(theta) < 0.0000001
    w = zeros(3,1);
    v = lin_trans;
else
    w_pre = [rot_mat(3,2) - rot_mat(2,3);
             rot_mat(1,3) - rot_mat(3,1);
             rot_mat(2,1) - rot_mat(1,2)];
    w = theta / (2*sin(theta)) * w_pre;
    w_cross = vec2ss(w);
    V_inv = (eye(3) - w_cross/2 + (w_cross^2 * 
    v = 
end

% [k, theta] = dcm2k_theta(rot_mat);
% w = k*theta;
% w_cross = vec2ss(w);

if abs(theta) < .00001
    % Approximate to avoid singularities if theta very small
    A = 1;
    B = 0;
    V_inv = eye(3) - 1/2*w_cross + w_cross^2;
else
    A = sin(theta) / theta;
    B = 1 - cos(theta) / theta^2;
    V_inv = eye(3) - 1/2*w_cross + 1/theta^2 * (1 - A/(2*B)) * w_cross^2;
end
v = V_inv * lin_trans;

kai(1:3, 1) = v;
kai(4:6, 1) = w;

end
