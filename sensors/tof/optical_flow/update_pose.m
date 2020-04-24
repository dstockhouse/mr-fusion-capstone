function [new_pos, new_att] = update_pose(constants, old_pos, old_att, transformation)

kai_est = trans2kai(transformation);

v_b = kai_est(1:3);
w_b = kai_est(4:6);
theta = norm(w_b);
k = w_b/theta;

% Attitude update
rotation = k_theta_2dcm(k, theta);
new_att = rotation * old_att;

% Put velocity into tan frame
v_t = new_att * v_b;

% Integrate velocity to get position
dt = 1/constants.fps;
new_pos = old_pos + v_t*dt;

end
