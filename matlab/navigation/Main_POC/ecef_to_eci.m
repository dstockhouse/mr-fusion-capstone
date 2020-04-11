function [w_b__i_b, f_b__i_b] = ecef_to_eci(t, r_e__e_b, v_e__e_b, a_e__e_b, C_e__b, w_e__eb, constants)
% Function Name: ecef_to_eci.m
% Description: Takes ground truth parameters in the ECEF frame to the ECI
% frame.


w_i__i_e   = constants.w_i__i_e;            % Angular velocity of {e} wrt {i} resolved in {i} (rad/s)
Ohm_i__i_e = constants.Ohm_i__i_e;          % Skew symmetric version of w_i__i_e (rad/s)

% Generate Orientation (i.e., Attitude)
theta_e = constants.w_ie * t + 0;           % Rotational angle theta_e = w_ie t + theta_GMST

C_i__e = [ cos(theta_e), -sin(theta_e), 0   % Orientation of {e} wrt {i}
           sin(theta_e),  cos(theta_e), 0
           0           , 0            , 1];

% Position of the b-frame wrt the i-frame resolved in {i} (m)
r_i__i_b = C_i__e * r_e__e_b;

% Velocity of the b-frame wrt the i-frame resolved in {i} (m/s)
v_i__i_b = C_i__e * (v_e__e_b + Ohm_i__i_e * r_e__e_b);
       
% Acceleration of the b-frame wrt the i-frame resolved in {i} (m/s^2)
a_i__i_b = C_i__e * (a_e__e_b + 2*Ohm_i__i_e * v_e__e_b + Ohm_i__i_e * Ohm_i__i_e * r_e__e_b);
        
% Orientation/Attitude of the Body-frame wrt the ECI-frame
C_i__b = C_i__e * C_e__b;     

% Angular velocity: Ideal Gyro Measurements
w_b__i_b = C_i__b' * (w_i__i_e + C_i__e * w_e__eb);       % Ideal gyro measurements

% Specific Force: Ideal Accel Measurements
[L_b, lambda_b, h_b] = xyz2llh(r_e__e_b, constants);    % Compute the lat, lon, and height of body
g_n__b = [0; 0; gravity(L_b, h_b, constants)];          % Compute the acceleration due to gravity
C_e__n = Lat_Lon_2C_e__n(L_b, lambda_b);                % Compute C_e__n
g_e__b = C_e__n * g_n__b;                               % Compute the gravity of the body in the {e} frame
f_b__i_b = C_i__b' * (a_i__i_b - C_i__e * g_e__b - Ohm_i__i_e * Ohm_i__i_e * r_i__i_b);      % Ideal accel measurements
end
