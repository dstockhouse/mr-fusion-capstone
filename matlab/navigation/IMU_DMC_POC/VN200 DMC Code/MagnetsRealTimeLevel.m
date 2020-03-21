%% EE 440 Final Project Script III
% Autor: Emmett Hamman
% Description: This script plots the true heading of the VN-200 using
% magnetometer data, "real-time"

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

% Collect VN200 measurements and plot on the compass
[s, SN] = initialize_VN200_IMU(Fs);   % Initialize the VN200
for k = 1:nSec*Fs              % Retrieve IMU data from VN200
    [magnet(k,:), accel(k,:), gyro(k,:), temp(k,:), baro(k,:)] = read_VN200_IMU(s); % Get VN200 IMU data
    
    psi_rad = atan2(-mean(magnet(:,2)), mean(magnet(:,1))) + dec_angle_rad;
    psi = psi_rad * (180/pi);
    [x,y] = pol2cart(-psi_rad, 1);
        
    if ~mod(N-k+1, 5)
        h2 = compass(x, y,'r');
        refreshdata(h2, 'caller');
        psi_rounded =round(psi);
        title(['True Heading:', num2str(psi_rounded), ' deg'], 'Color','red')
        view([-90 90])
        drawnow;

        if ~mod(N-k+1,Fs)
            fprintf('Please wait %i more secomds!!!\n', round((N-k+1) / Fs));
        end
    end
    
end
stop_VN200(s);                  % Terminates the VN200 IMU data transmission

