function P_gps = GPS_Sim(P_truth, constants)
% Function Name: GPS_Sim.m
% Description: Simulates GPS errors to create GPS readings based on true
% position input. 

sigma = constants.gps.sigma;        % Radius of error
P_error = normrnd(0,sigma,3,1);     % Error = radius of error * rand value
P_gps = P_truth + P_error;          % GPS = Truth + Error 

end
