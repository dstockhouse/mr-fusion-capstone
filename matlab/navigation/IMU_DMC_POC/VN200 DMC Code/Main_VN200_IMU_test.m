% EE 440 Modern Nav
% Test code for the VN200 IMU
%  - Plots Gyro, accel, Barometer, compass data, and IMU temperature
%
% Author: S. Bruder

clear;                          % Clear all variables from the workspace
close all;                      % Close all windows
clc;                            % "Clean" the command window

p = genpath(pwd);               % List all subfolders of the current folder
addpath(p);                     % Add all subfolders the the path (current session)

Fs = 50;                        % Set Sample frequency <= 100 (Hz)
dT = 1/Fs;                      % Sample interval (sec)
nSec = 10;                      % Duration of data collection (sec)
N = Fs*nSec;                    % Number of samples to collect (dimless)
fprintf('Collecting data for %2i sec at %d Hz\n\n', nSec, Fs);

compass = zeros(N, 3);          % Initialize the compass data array: 3 floats per sample
gyro    = zeros(N, 3);          % Initialize the gyro data array:    3 floats per sample
accel   = zeros(N, 3);          % Initialize the accel data array:   3 floats per sample
temp    = zeros(N,1);           % Initialize the temperature data array: 1 float per sample
baro    = zeros(N,1);           % Initialize the Barometric pressure data array: 1 float per sample

%% Collect VN200 measurements
[s, SN] = initialize_VN200_IMU(Fs);   % Initialize the VN200
 for k = 1:nSec*Fs              % Retrieve IMU data from VN200
    [compass(k,:), accel(k,:), gyro(k,:), temp(k,:), baro(k,:)] = read_VN200_IMU(s); % Get VN200 IMU data
    if ~mod(N-k+1,Fs)
        fprintf('Please wait %i more secomds!!!\n', round((N-k+1) / Fs));
    end
 end
stop_VN200(s);                  % Terminates the VN200 IMU data transmission

t = 0:dT:(N-1)*dT;              % Time vector (sec)

% Plot the compass/magnetometer data (in Gauss)
figure,
plot(t, compass(:,1), 'r', t, compass(:,2), 'g', t, compass(:,3), 'b');
title('Plot of VN200 Compass Data', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Magnetic Field (Gauss)')
legend('m_x', 'm_y', 'm_z')
grid

% Plot the accel data (in m/s^2)
figure,
plot(t, accel(:,1), 'r', t, accel(:,2), 'g', t, accel(:,3), 'b');
title('Plot of VN200 Accel Data', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Accel (m/s^2)')
legend('a_x', 'a_y', 'a_z')
grid

% Plot the Gyro data (in deg/s)
figure,
plot(t, gyro(:,1)*180/pi, 'r', t, gyro(:,2)*180/pi, 'g', t, gyro(:,3)*180/pi, 'b');
title('Plot of VN200 Gyro Data', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Angular Rate (°/s)')
legend('\omega_x', '\omega_y', '\omega_z')
grid

% Plot the Temperature data (in deg C)
figure,
plot(t, temp, 'k');
title('Plot of VN200 Temperature Data', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Temperature (°C)')
grid

% Plot the Brometric pressure data (in kPa)
figure,
plot(t, baro, 'k');
title('Plot of VN200 Barometric Pressure Data', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Pressure (kPa)')
grid
