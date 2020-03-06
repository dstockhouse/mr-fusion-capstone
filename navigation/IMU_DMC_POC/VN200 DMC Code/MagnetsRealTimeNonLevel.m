%% EE 440 Final Project Script IV
% Autor: Emmett Hamman
% Description: This script plots the true heading of the VN-200 using
% magnetometer data, "real-time", while not level

clear;
close all;
clc;

%%
p = genpath(pwd);               % List all subfolders of the current folder
addpath(p);                     % Add all subfolders the the path (current session)

Fs = 50;                        % Set Sample frequency <= 100 (Hz)
dT = 1/Fs;                      % Sample interval (sec)
nSec = 10;                      % Duration of data collection (sec)
N = Fs*nSec;                    % Number of samples to collect (dimless)
dec_angle_rad = 10.6 * (pi/180);
fprintf('Collecting data for %2i sec at %d Hz\n\n', nSec, Fs);

magnet  = zeros(N, 3);          % Initialize the compass data array: 3 floats per sample
gyro    = zeros(N, 3);          % Initialize the gyro data array:    3 floats per sample
accel   = zeros(N, 3);          % Initialize the accel data array:   3 floats per sample
temp    = zeros(N,1);           % Initialize the temperature data array: 1 float per sample
baro    = zeros(N,1);           % Initialize the Barometric pressure data array: 1 float per sample

figure
h2 = compass(NaN,NaN,'-');
roll_anno = annotation('textbox', [0.1, 0.9, 0, 0], 'string', ['Roll:  deg'], 'FitBoxToText','on');
pitch_anno = annotation('textbox', [0.1, 0.8, 0, 0], 'string', ['Pitch: deg'],'FitBoxToText','on');
title(['True Heading: deg'], 'Color','red')

% Collect VN200 measurements and plot on the compass
[s, SN] = initialize_VN200_IMU(Fs);   % Initialize the VN200
for k = 1:nSec*Fs              % Retrieve IMU data from VN200
    [magnet(k,:), accel(k,:), gyro(k,:), temp(k,:), baro(k,:)] = read_VN200_IMU(s); % Get VN200 IMU data
    
    theta_rad = atan(accel(k,1) / sqrt(accel(k,2)^2 + accel(k,3)^2));
    phi_rad = atan2(-accel(k,2), -accel(k,3));
    
    psi_rad = atan2(-mean(magnet(:,2))*cos(phi_rad) + mean(magnet(:,3))*sin(phi_rad), ...
                    mean(magnet(:,1))*cos(theta_rad) + mean(magnet(:,2))*sin(phi_rad)*sin(theta_rad)...
                    + mean(magnet(:,3))*cos(phi_rad)*sin(theta_rad));
    [x,y] = pol2cart(-psi_rad, 1);
        
    if ~mod(N-k+1, 10)
        h2 = compass(x, y,'r');
        refreshdata(h2, 'caller');
        psi_rounded =round(psi_rad*(180/pi));
        phi_rounded = round(phi_rad*(180/pi));
        theta_rounded = round(theta_rad*(180/pi));
        title(['True Heading:', num2str(psi_rounded), ' deg'], 'Color','red')
%         annotation('textbox', [0.1, 0.9, 0, 0], 'string', ['Roll: ', num2str(phi_rounded),' deg'], 'FitBoxToText','on')
%         annotation('textbox', [0.1, 0.8, 0, 0], 'string', ['Pitch: ', num2str(theta_rounded),' deg'],'FitBoxToText','on')
        set(roll_anno, 'string', ['Roll: ', num2str(phi_rounded),' deg']);
        set(pitch_anno, 'string', ['Pitch: ', num2str(theta_rounded),' deg']);
        view([-90 90])
        drawnow;

        if ~mod(N-k+1,Fs)
            fprintf('Please wait %i more secomds!!!\n', round((N-k+1) / Fs));
        end
    end
    
end
stop_VN200(s);                  % Terminates the VN200 IMU data transmission
