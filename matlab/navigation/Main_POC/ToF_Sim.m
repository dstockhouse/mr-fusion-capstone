function [P_ToF, A_ToF] = ToF_Sim(P_truth, A_truth, constants)
% Function Name: ToF_Sim.m
% Description: Simulates ToF errors to create ToF readings based on true
% position and attitude readings. 
% Inputs: P_truth = Position Truth
%         A_truth = Attitude Truth
% Outputs: P_ToF = ToF Position Reading
%          A_ToF = ToF Attitude Reading

P_sigma = constants.tof.P_sigma;    % Radius of position error
A_sigma = constants.tof.A_sigma;    % Radius of attitude error
P_error = normrnd(0,P_sigma,3,1);   % P_error = radius of error * rand value
P_ToF = P_truth + P_error;          % P_ToF = Truth + Error 
A_error = normrnd(0,V_sigma,3,1);   % A_error = radius of error * rand value
A_ToF = A_truth + A_error;          % A_ToF = Truth + Error
end
