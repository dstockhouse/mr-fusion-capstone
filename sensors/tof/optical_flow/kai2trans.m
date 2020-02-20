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

theta = norm(w);
if abs(theta) < 0.0000001
    rot_mat = eye(3);
    lin_trans = v;
else
    w_cross = vec2ss(w);
    k = w / theta;
    rot_mat = k_theta_2dcm(k, theta);
    
    V = eye(3) + (1-cos(theta))/theta^2 * w_cross + (theta - sin(theta))/theta^3 * w_cross^2;
    lin_trans = V * v;
end

% Populate transformation matrix with linear and angular velocities
trans = eye(4);
trans(1:3, 1:3) = rot_mat;
trans(1:3, 4) = lin_trans;

end