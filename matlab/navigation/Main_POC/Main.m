% Mr. Fusion
% EE 421
% Spring 2020
% Kalman Filter Model - MATLAB Scripts Verison

%% Initial Resets 
close all;  % Close all figures
clear all;      % Clear all variables
clc;        % Clear Command window

%% Adjust System Path 
addpath("./common")     % Common code from Hwks and project folder
addpath("./plotting")   % Plotting code

%% Initializations
IMU.Fs = 50;    % Set IMU Sampling Frequency (Hz)
dt = 1/IMU.Fs;  % Set discrete time step
t_len = 180;    % Set length of time sim will run (seconds)


gyro = gyro_error(0, 0, 10, 13, 1000, 0.05);        % Set errors for gyros
accel = accel_error(0, 0, 0.04, 0.14, 5000, 0.05);  % Set errors for accels
gps = gps_error(2.5, 50);                           % Set errors for GPS
odo = odo_error(0.1, 0.1);                          % Set errors for odometry
tof = tof_error(1, 0.1);                            % Set errors for ToF
    
load_constants; % Run script to load constant values struct

%% Loop to Run Model
t = 0:dt:180;                       %Build Time Vector
P_truth = zeros(3, length(t));
V_truth = zeros(3, length(t));
Accel_truth = zeros(3, length(t));
Gyro_truth = zeros(3, length(t));
P_gps_array = zeros(3, length(t));
w_b__i_b_tilde_array = zeros(3, length(t));
f_b__i_b_tilde_array = zeros(3, length(t));
P_imu = zeros(3, length(t));
V_imu = zeros(3, length(t));
V_odo_array = zeros(3, length(t));
dP_array = zeros(3, length(t));
dV_array = zeros(3, length(t));

for i = 1:length(t)
    if i == 1
        % Generate Ground Truth and set re-used vars
        [w_b__i_b, f_b__i_b, r_t__t_b, v_t__t_b, C_t__b, C_t__bm_out] = ground_truth_generation(t(i),1,eye(3), constants);
        C_t__bm_in = C_t__bm_out;
        
        % Generate IMU Measurements and set re-used vars
        [w_b__i_b_tilde, f_b__i_b_tilde, b_g_BI, b_a_BI] = IMU_Sim(w_b__i_b, f_b__i_b, 1, [0; 0; 0], [0; 0; 0], constants);
        b_g_BI_in = b_g_BI;
        b_a_BI_in = b_a_BI;
        
        % Mechanize IMU Measurements 
        [r_t__t_b_K, v_t__t_b_K, C_t__b_K] = TAN_mech_Script(w_b__i_b_tilde, f_b__i_b_tilde, r_t__t_b, v_t__t_b, C_t__b, 1, constants);
    else
        % Generate Ground Truth and set re-used vars
        [w_b__i_b, f_b__i_b, r_t__t_b, v_t__t_b, C_t__b, C_t__bm_out] = ground_truth_generation(t(i),0,C_t__bm_in, constants);
        C_t__bm_in = C_t__bm_out;
        
        % Generate IMU Measurements and set re-used vars
        [w_b__i_b_tilde, f_b__i_b_tilde, b_g_BI, b_a_BI] = IMU_Sim(w_b__i_b, f_b__i_b, 0, b_g_BI_in, b_a_BI_in, constants);
        b_g_BI_in = b_g_BI;
        b_a_BI_in = b_a_BI;
        
        % Mechanize IMU Measurements
        [r_t__t_b_K, v_t__t_b_K, C_t__b_K] = TAN_mech_Script(w_b__i_b_tilde, f_b__i_b_tilde, r_t__t_b, v_t__t_b, C_t__b, 0, constants);
    end
    
    % Simulate GPS Receiver 
    P_gps = GPS_Sim(r_t__t_b, constants);
    
    % Simulate Wheel Odometry (Simplified)
    [V_odo, A_odo] = Wheel_Odometry_Simplified(v_t__t_b, C_t__b, constants);
    
    % Kalman Filter Setup and Execution
    delta_P = P_gps - r_t__t_b_K;
    [dP, dV, dA] = Kalman_Script(delta_P, f_b__i_b_tilde, r_t__t_b_K, C_t__b_K, constants);
    
    % Store Single Values in Arrays
    % Truths
    P_truth(:,i) = r_t__t_b;
    V_truth(:,i) = v_t__t_b;
    Accel_truth(:,i) = f_b__i_b;
    Gyro_truth(:,i) = w_b__i_b;
    
    % IMU Measurements
    w_b__i_b_tilde_array(:,i)= w_b__i_b_tilde;
    f_b__i_b_tilde_array(:,i)= f_b__i_b_tilde;
    
    % Mechanized IMU Stuffs
    P_imu(:,i) = r_t__t_b_K;
    V_imu(:,i) = v_t__t_b_K;
    
    % GPS
    P_gps_array(:,i) = P_gps;
    
    % Wheel Odometry 
    V_odo_array(:,i) = V_odo;
    
    % Filtered Stuffs
    dP_array(:,i) = dP;
    dV_array(:,i) = dV;
end

% Apply Corrections
P_KF = P_imu + dP_array;
V_KF = V_imu + dV_array;
%% Plot Results of Model

fprintf("GPS\n")
plot_P(P_truth', P_gps_array', "P GPS", constants);           % Plot GPS measurements

fprintf("IMU\n")
plot_PVA(P_truth', V_truth', P_imu', V_imu', "PVA IMU", constants)   % Plot IMU

fprintf("KF\n")
plot_PVA(P_truth', V_truth', P_KF', V_KF', "P KF", constants)   % Plot KF data