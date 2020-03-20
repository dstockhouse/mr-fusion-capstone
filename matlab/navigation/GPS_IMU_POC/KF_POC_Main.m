%% File: KF_POC_Main.m
% Author :Emmett Hamman
% EE 421: Mr. Fusion Capstone
% Description: Main file for the EKF PoC Simulation

%% Initial Reset
close all               % Close all figures
clc                     % Clear the command window 
clear                   % Clear all variables

%% Adjust System Path
addpath("./common")     % Common code from Hwks and project folder
addpath("./plotting")   % Plotting code

%% Load Test Data 
load('VN200_GPS_Data.mat');
load('VN200_IMU_Data.mat');

%% Initialization
% IMU.Fs = 250;                                 % IMU sampliing frequency
t_len = 120;                                    % Duration of run
t_GPS = 0:1/vn200_gps_Fs:t_len;                 % GPS Timesteps
t_IMU = 0:1/vn200_imu_FS:t_len;                 % IMU Timesteps

% Zero error run
%gyro = gyro_error(0, 0, 0, 0, 0, 0);            % Gyro errors
%accel = accel_error(0, 0, 0, 0, 0, 0);      % Accel errors
% VN-200 Error run (Assume perfect calibration)
% gyro = gyro_error(0, 0, 10, 13, 1000, 0.05);            % Gyro errors
% accel = accel_error(0, 0, 0.04, 0.14, 5000, 0.05);
% gps = gps_error(2.5, 250);                                % GPS errors

load_constants;         % Load Constant values

%% Simulation

%% Plotting
