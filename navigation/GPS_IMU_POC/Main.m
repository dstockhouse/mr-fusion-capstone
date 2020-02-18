% Joseph Kroeker
% EE 440
% Final Project
% 12/10/2019
% Project attempt 2 with backups saved a lot so that it doesnt corrupt

%% Initial Reset
close all               % Close all figures
clc                     % Clear the command window 
clear                   % Clear all variables

%% Adjust System Path
addpath("./common")     % Common code from Hwks and project folder
addpath("./plotting")   % Plotting code

%% Initialization
IMU.Fs = 250;           % IMU sampliing frequency
t_len = 120;             % Duration of run

% Zero error run
%gyro = gyro_error(0, 0, 0, 0, 0, 0);            % Gyro errors
%accel = accel_error(0, 0, 0, 0, 0, 0);      % Accel errors
% VN-200 Error run (Assume perfect calibration)
gyro = gyro_error(0, 0, 10, 13, 1000, 0.05);            % Gyro errors
accel = accel_error(0, 0, 0.04, 0.14, 5000, 0.05);
gps = gps_error(2.5, 250);                                % GPS errors

load_constants;         % Load Constant values

%% Simulation
eval('Main_Sim');
sim('Main_Sim')

% Plotting 
fprintf("GPS\n")
plot_P(P_truth, P_gps, "P GPS", constants);           % Plot GPS measurements
fprintf("Comp\n")
plot_P(P_truth, P_COMP, "P Complementary filter", constants);
fprintf("IMU\n")
plot_PVA(P_truth, V_truth, A_truth, P_imu, V_imu, A_imu, "PVA IMU", constants)   % Plot IMU
fprintf("KF\n")
plot_PVA(P_truth, V_truth, A_truth, P_KF, V_KF, A_KF, "P KF", constants)   % Plot KF data
fprintf("LSR\n")
plot_P(P_truth, P_LSR, "P LSR", constants)
%% Functions
% Gyro given error sources
function gyro = gyro_error(FB, BS, BI, PSD, sfs, m_g)
    gyro.FB = FB;       % Bias - Fixed Bias (deg/hr)
    gyro.BS = BS;       % Bias - Bias Stability term 1-sigma (deg/hr)
    gyro.BI = BI;       % Bias - Bias Instability term 1-sigma (deg/hr)
    gyro.PSD = PSD;     % Gyro PSD (deg/hr)/rt_Hz
    gyro.sfs = sfs;     % Scale factor stability (ppm)
    gyro.m_g = m_g;     % Misalignment term (deg)
end

% Accel given error sources
function accel = accel_error(FB, BS, BI, PSD, sfs, m_a)
    accel.FB = FB;      % Bias - Fixed Bias term (mg)
    accel.BS = BS;      % Bias - Bias Stability term 1-sigma (mg)
    accel.BI = BI;      % Bias - Bias Instability term 1-sigma (mg)
    accel.PSD = PSD;    % Accel PSD (mg/rt-Hz) 
    accel.sfs = sfs;    % Scale factor stability (ppm)
    accel.m_a = m_a;    % Misalignment term (deg)
end

% GPS given error source
function gps = gps_error(sigma, Fs)
    gps.sigma = sigma;                  % GPS error radius 1-sigma (m)
    gps.Fs = Fs;                % GPS sampling frequency
end



