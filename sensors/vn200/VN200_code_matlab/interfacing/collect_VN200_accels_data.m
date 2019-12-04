% EE 440 Modern Nav
% Script to collect VN200 IMU accel data
% Author: S. Bruder

clear;                  % Clear all variables from the workspace
close all;              % Close all windows
clc;                    % "Clean" the command window

% Define text message strings
txt_msg(1,:) = 'Z-Up   configuration';  % Cube face#3 up
txt_msg(2,:) = 'Z-Down configuration';  % Cube face#1 up
txt_msg(3,:) = 'Y-Up   configuration';  % Cube face#2 up
txt_msg(4,:) = 'Y-Down configuration';  % Cube face#4 up
txt_msg(5,:) = 'X-Up   configuration';  % Cube face#5 up
txt_msg(6,:) = 'X-Down configuration';  % Cube face#6 up

%% Connect to the VN200 IMU via USB to serial cable
Fs = 50;                       % Default Sample frequency <= 200 (Hz)
dT = 1/Fs;                      % Sample interval (sec)
nSec = 10;                      % Duration of data collection (sec)
nSamples = Fs*nSec;             % Number of samples to collect (dimless)

accel   = zeros(Fs*nSec, 3);    % Initialize the accel data array: 3 floats per sample    
%% Place the IMU in the 6-configurations 
for j = 1:6
    disp(['Place the IMU in the ', txt_msg(j,:)]);
    fprintf('Hit any key when you are ready to start the %3i sec accel data collection at %d Hz!!\n\n', nSec, Fs); pause;
    beep                            % make a "beep" sound
    
    [s, SN] = initialize_VN200_IMU(Fs); % Initialize the VN200
    for k = 1:nSec*Fs                   % Retrieve IMU data from VN200
        [compass, accel(k,:), gyro, temp, baro] = read_VN200_IMU(s); % Get VN200 IMU data
        if ~mod(nSamples-k+1,Fs)
            fprintf('Please wait %i more secomds!!!\n', round((nSamples-k+1) / Fs));
        end
    end
    stop_VN200(s);                      % Terminates the VN200 IMU data transmission

    % Plot the accel data (in m/s^2)
    t = 0:dT:(nSamples-1)*dT;           % Time vector (sec)
    figure,
    plot(t, accel(:,1), 'r', t, accel(:,2), 'g', t, accel(:,3), 'b');
    title(['Uncalibrated VN200 Accel (SN:',SN,'): ',txt_msg(j,:)]);
    xlabel('Time (sec)');
    ylabel('Accel (m/s^2)')
    legend('a_x', 'a_y', 'a_z')
    grid
    
    switch j
        case 1
            save('z_up_accel.mat',   'accel', 'Fs', 'SN');
        case 2
            save('z_down_accel.mat', 'accel', 'Fs', 'SN');
        case 3
            save('y_up_accel.mat',   'accel', 'Fs', 'SN');
        case 4
            save('y_down_accel.mat', 'accel', 'Fs', 'SN');
        case 5
            save('x_up_accel.mat',   'accel', 'Fs', 'SN');
        case 6
            save('x_down_accel.mat', 'accel', 'Fs', 'SN');            
    end
    disp(' ');
end
fprintf('\n Accel CALIBRATION data collection Completed!!\n')