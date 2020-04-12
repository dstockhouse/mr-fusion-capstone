function odo = odo_error(V_sigma, A_sigma)
% Function Name: odo_error.m
% Description: Takes odo error source terms as inputs and outputs them 
% into an odo error source stuct 
    
    odo.V_sigma = V_sigma;      % Uncertainty of Odometry Velocity estimates (m/s)
    odo.A_sigma = A_sigma;      % Uncertainty of Odometry Attitude estimates (rad)
end