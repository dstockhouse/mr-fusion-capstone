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

w = kai(4:6);
w_cross = vec2ss(w);
theta = norm(w);
k = w / theta;
rotation = k_theta_2dcm(k, theta);

vel = kai(1:3);
V = eye(3) + (1-cos(theta))/theta^2 * w_cross + (theta - sin(theta))/theta^3 * w_cross^2;

% Populate transformation matrix with linear and angular velocities
trans = eye(4);
trans(1:3, 1:3) = rotation;
trans(1:3, 4) = V * vel;

end