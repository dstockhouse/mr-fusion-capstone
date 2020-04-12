function tof = tof_error(P_sigma, A_sigma)
% Function Name: tof_error.m
% Description: Takes tof error source terms as inputs and outputs them 
% into an tof error source stuct 
    
    tof.P_sigma = P_sigma;      % uncertainty of ToF Position estimates (m)
    tof.A_sigma = A_sigma;      % Uncertainty of ToF Attitude estimates (rad)
end