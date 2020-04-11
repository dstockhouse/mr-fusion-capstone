function [r_e__e_b, v_e__e_b, a_e__e_b, C_e__b, w_e__eb] = tan_to_ecef(r_t__t_b, v_t__t_b, a_t__t_b, C_t__b, w_t__tb, constants)
% Function Name: tan_to_ecef.m
% Description: Converts ground truth parameters from the TAN frame to the
% ECEF frame. 

% Position of the t-frame wrt the e-frame resolved in {e} (m) - A constant vector
r_e__e_t = constants.r_e__e_t;

% Orientation/Attitude of the Tangential-frame wrt the ECEF-frame
C_e__t = constants.C_e__t ;

% Position of the b-frame wrt the e-frame resolved in {e} (m)
r_e__e_b = r_e__e_t + C_e__t * r_t__t_b;

% Velocity of the b-frame wrt the e-frame resolved in {e} (m/s)
v_e__e_b = C_e__t * v_t__t_b;
       
% Acceleration of the b-frame wrt the e-frame resolved in {e} (m/s^2)
a_e__e_b = C_e__t * a_t__t_b;
        
% Orientation/Attitude of the Body-frame wrt the ECEF-frame
 C_e__b = C_e__t * C_t__b;         

% Angular velocity of the b-frame wrt the e-frame resolved in {e} (rad/s)
w_e__eb = C_e__t * w_t__tb;
end