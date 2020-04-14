function [V_odo, A_odo] = Wheel_Odometry_Simplified(V_truth, A_truth, constants)
% Function Name: Wheel_Odometry_Simplified.m
% Description: Simulates Wheel Odometry based on actual velocity and
% attitude input. 
% Inputs: V_truth = Velocity Truth
%         A_truth = Attitude Truth
% Outputs: V_odo = Forward velocity Odometry
%          A_odo = Orientation about z-axis on robot body frame 

V_sigma = constants.odo.V_sigma;      % Radius of errors of velocity
A_sigma = constants.odo.A_sigma;      % Radius of errors of attittude in radians
V_error = normrnd(0,V_sigma,1,1);     % Error = radius of error * rand value
V_odo = V_truth + V_error;      % V_odo = Magnitude(Truth) + Error 
A_error = normrnd(0,A_sigma,1,1);     % Error = radius of error * rand value
A_odo = A_truth(3) + A_error;         % A_odo = Truth_z + Error
end
