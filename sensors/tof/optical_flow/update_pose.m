function [new_pose] = update_pose(old_pose, transformation)

rotation = old_pose(1:3, 1:3);

kai_est = trans2kai(transformation);
v_tan = rotation * kai_est(1:3);
w_tan = rotation * kai_est(4:6);

% Follow Jaimez CPP procedure for the rest

end