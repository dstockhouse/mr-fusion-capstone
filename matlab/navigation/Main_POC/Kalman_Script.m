function [dP, dV, dA] = Kalman_Script(delta_P, f_b__i_b_tilde, r_t__t_b_hat, C_t__b_hat, constants)
% Function Name: Kalmna_Script.m
% Description: Sets parameters for and executes Kalman Filter Algorithm to
% fix the IMU Mechanized Position with GPS Sensor Aid

I3 = eye(3);
O3 = zeros(3);
sigma_gps = constants.gps.sigma;
dt         = constants.dt;          % Time step (sec)
Ohm_i__i_e = constants.Ohm_i__i_e;  % Skew symmetric version of w_i__i_e (rad/s)
C_e__t     = constants.C_e__t;   	% Attitude of t-frame wrt e-frame (a constant matrix)
C_t__e     = C_e__t';
r_e__e_t   = constants.r_e__e_t;    % Position of org of t-frame wrt e-frame resolved in e-frame
Df_b__i_b = constants.accel.BI.sigma_n;     % Assume error in accel is all BI
Dw_b__i_b = constants.gyro.BI.sigma_n;      % Assume error in gyro is all BI

persistent Delta_PVAk_1 P_k_1
   
if isempty(Delta_PVAk_1)    
    Delta_PVAk_1  = zeros(9,1);     % Initialize Delta_PVA(k-1)
    Delta_PVAk    = zeros(9,1);     % Initialize Delta_PVA(k-1)
    P_k_1         = 0.1 * eye(9);
    P_k           = 0.1 * eye(9);
else
%% Build the F matrix: See lecture 29 slide 7
    O3 = zeros(3,3);
% First row:
    Ohm_t__i_e = C_t__e * Ohm_i__i_e * C_e__t;  % Earth rate in the t-frame

% Second row:
    F21 = -vec2ss(C_t__b_hat * f_b__i_b_tilde); % Vector to Skew-Symmetric
  % Now build F23
    r_e__e_b_hat = r_e__e_t + C_e__t * r_t__t_b_hat;
    [L_b_hat, lambda_b_hat, h_b_hat] = xyz2llh(r_e__e_b_hat, constants);
    gamma_e__i_b = gamma__i_b(r_e__e_b_hat, constants);
    r_e__e_S = Geo_Radius(L_b_hat, constants);
 
    F23 = -C_t__e * 2 * gamma_e__i_b * r_e__e_b_hat' / (r_e__e_S * norm(r_e__e_b_hat)) * C_e__t;
     
    F = [ -Ohm_t__i_e   ,  O3               , O3
           F21          , -2* Ohm_t__i_e    , F23
           O3           ,  I3               , O3];

%% Build the B matrix: See lecture 29 slide 7
    B = [C_t__b_hat * Dw_b__i_b
         C_t__b_hat * Df_b__i_b
         O3];
%% Transform Eqns from continuous-time to discrete-time: Lecture 30 slides 9-10
    A = eye(9) + F*dt;                % Estimate of exp(F dt) = I + F dt + F^2 dt^2 /2! + ...
	Q = eye(9)*0.01;
    G  = (eye(9)*dt + F*dt^2/2) * B;    % Estimate of G = A^-1 (exp(F dt) - I ) B = (I dt + F dt^2 /2! + ...) B
    
    H = [I3, O3, O3];                   % Only have delta Position
    R = I3*sigma_gps;                       % R is based on the sigma of GPS
    [Delta_PVAk, P_k] = Kalman_Filter(Delta_PVAk_1, P_k_1, delta_P, A, H, Q, R, O3, O3);

    Delta_PVAk_1 = Delta_PVAk;
    P_k_1 = P_k;
end

    dP = Delta_PVAk(1:3,1);
    dV = Delta_PVAk(4:6,1);
    dA = Delta_PVAk(7:9,1);
end