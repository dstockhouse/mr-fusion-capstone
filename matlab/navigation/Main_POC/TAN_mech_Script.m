function [r_t__t_b_K, v_t__t_b_K, C_t__b_K] = TAN_mech_Script(w_b__i_b_tilde, f_b__i_b_tilde, P_init, V_init, A_init, first_run_flag, constants)
% Function Name: TAN_Mech_Script.m
% Description: Takes simulated IMU measurements and implements tangential
% mechanization equations.

persistent r_t__t_b_K_1 v_t__t_b_K_1 C_t__b_K_1

fidelity = constants.fidelity;      % Set to '0' for Low-fidelity and '1' for High-fidelity
dt       = constants.dt;            % Time step (sec)
I3       = eye(3);                  % 3X3 Identity matrix
Ohm_i__i_e = constants.Ohm_i__i_e;  % Earth Rate skew-symmetric matrix (rad/s)
w_i__i_e   = constants.w_i__i_e;    % Angular velocity of {e} wrt {i} resolved in {i} (rad/s)
C_e__t = constants.C_e__t;          % Attitude of t-frame wrt e-frame (a constant matrix)
C_t__e = C_e__t';
r_e__e_t = constants.r_e__e_t;      % Position of org of t-frame wrt e-frame resolved in e-frame

% If this is the first call to the function then Initialize the PVA
if first_run_flag == 1      % Initialize the PVA (with zero for everything)
    r_t__t_b_K = P_init;
    v_t__t_b_K = V_init;
    C_t__b_K   = A_init;
    
    % Update PVA(k-1) = PVA(-) values
    r_t__t_b_K_1 = P_init; % Body and nav frame share common org
    v_t__t_b_K_1 = V_init;
    C_t__b_K_1   = A_init;
else
%==========================================================================
% Tangential Frame PVA Mechanization:
%==========================================================================
    Ohm_t__i_e = C_t__e * Ohm_i__i_e * C_e__t; % Earth rate in the t-frame
    
    %----------------------------------------------------------------------
    % STEP 1.) Attitude Update
    %----------------------------------------------------------------------    
    if ~fidelity                    % If "low-fidelity" eqns

    % First order approximation used (Lower fidelity eqns) ++++++++++++++++++++
    % Build skew-symmetric matrix Ohm from vector w
    Ohm_b__i_b_tilde = vec2ss(w_b__i_b_tilde); % (rad/s)
    %  t       t              b             t    t
    % C (k) = C (k-1) [I + Ohm (k) dt] - Ohm    C (k-1)
    %  b       b              ib            ie   b
    C_t__b_K = C_t__b_K_1 * (I3 + Ohm_b__i_b_tilde*dt) - Ohm_t__i_e * C_t__b_K_1*dt;

    else                            % If "high-fidelity" eqns
    % Higher fidelity equations +++++++++++++++++++++++++++++++++++++++++++++++
    %   t        t  b        t
    %  w  (k) = C  w  (k) - w 
    %   tb       b  ib       ie
    w_t__t_b = C_t__b_K_1 * w_b__i_b_tilde - C_t__e * w_i__i_e;  % (rad/s)
    %            t
    % Defining: w   dt = k d_theta, and K = Skew(k)
    %            tb
    
    %  t                                               2     t
    % C (k) = [I + sin(d_theta) K + (1 - cos(d_theta) K )]  C (k-1) 
    %  b                                                     b
    %
    C_t__b_K = Rodrigues_k(w_t__t_b*dt) * C_t__b_K_1; % 
    end
    
    %--------------------------------------------------------------------------
    % STEP 2.) Specific Force Update
    %----------------------------------------------------------------------
    %  t     t     b  
    % f   = C (k) f
    %  ib    b     ib
    f_t__i_b = C_t__b_K * f_b__i_b_tilde;   % (m/s^2)

    %--------------------------------------------------------------------------
    % STEP 3.) Velocity Update
    %----------------------------------------------------------------------
    % Gravity Calculation:
    % Since the body is above the ellipsoid use the "gravity" fn NOT Somigliana
    r_e__e_b = r_e__e_t + C_e__t * r_t__t_b_K_1;            % Compute r_e__e_b
    [L_b, lambda_b, h_b] = xyz2llh(r_e__e_b, constants);    % Compute the lat, lon, and height
    g_n__b = [0; 0; gravity(L_b, h_b, constants)];          % Compute the acceleration due to gravity
    C_e__n = Lat_Lon_2C_e__n(L_b, lambda_b);                % Compute C_e__n
    g_e__b = C_e__n * g_n__b;                               % Compute the gravity of the body in the {e} frame
    g_t__b = C_t__e * g_e__b;                               % Compute the gravity of the body in the {t} frame
    
    %  t    t     t        t   t
    % a  = f   + g  - 2 Ohm   v  (k-1)
    %  tb   ib    b        ie  tb
    a_t__t_b = f_t__i_b + g_t__b - 2*Ohm_t__i_e * v_t__t_b_K_1; % (m/s^2)

    %   t        t          t
    %  v  (k) = v  (k-1) + a   dt
    %   tb       tb         tb
    v_t__t_b_K = v_t__t_b_K_1 + a_t__t_b * dt;  % (m/s)

    %--------------------------------------------------------------------------
    % STEP 4.) Position Update
    %----------------------------------------------------------------------
    %   t        t          t             t         2
    %  r  (k) = r  (k-1) + v  (k-1) dt + a  (k-1) dt
    %   tb       tb         tb            tb
    r_t__t_b_K = r_t__t_b_K_1 + v_t__t_b_K_1*dt + a_t__t_b * dt^2 / 2; % slide 8 (m)
    
    % Store PVA(k-1) values for use in next iteration
    r_t__t_b_K_1 = r_t__t_b_K; % Body and nav frame share common org
    v_t__t_b_K_1 = v_t__t_b_K;
    C_t__b_K_1   = C_t__b_K;
end  % End of the t_sec == 0 block

end