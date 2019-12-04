% EE 440 Modern Nav
% Test code for the VN200 GPS receiver
%  - Plots Lat, Lon, height, and number of sats in view
%
% Author: S. Bruder

clear;                          % Clear all variables from the workspace
close all;                      % Close all windows
clc;                            % "Clean" the command window

p = genpath(pwd);               % List all subfolders of the current folder
addpath(p);                     % Add all subfolders the the path (current session)

% DO NOT change the sample rate for the GPS!!!
Fs = 5;                         % The GPS updates ONLY at 5 Hz
dT = 1/Fs;                      % Sample interval (sec)
nSec = 10;                      % Duration of data collection (sec)
N = Fs*nSec;                    % Number of samples to collect (dimless)
fprintf('Collecting GPS data for %2i sec at %d Hz\n\n', nSec, Fs);

lat     = zeros(N,1);           % Initialize the latitude data array: 1 float per sample
lon     = zeros(N,1);           % Initialize the longitude data array
hb      = zeros(N,1);           % Initialize the height data array
t       = zeros(N,1);           % Initialize the time data array
NumSats = zeros(N,1);           % Initialize the number of satellites in view array

%% Collect VN200 GPS measurements
s = initialize_VN200_GPS(Fs);   % Initialize the VN200
 for k=1:nSec*Fs                % Retrieve GPS data from VN200
    [t(k), NumSats(k), lat(k), lon(k), hb(k)] = read_VN200_GPS(s); % Get VN200 GPS data

    if ~mod(N-k+1,Fs)
        fprintf('Please wait %i more secomds!!!\n', round((N-k+1) / Fs));
    end
 end
stop_VN200(s);                  % Terminates the VN200 IMU data transmission

fprintf('\nNumber of Satellites in view = %d\n', NumSats);
t = t - t(1);                   % Subtract start time (start at t = 0 secs)

%% Plot the GPS data (in lat, lon, height)
figure('Units', 'normalized','Position', [0.05, 0.05, 0.5, 0.8]);
subplot(4,1,1)
plot(t, lat, 'r');
title('Plot of VN200 GPS Data', 'FontSize', 14);
ylabel('Lat (°)')
grid

subplot(4,1,2)
plot(t, lon, 'b');
ylabel('Lon (°)')
grid

subplot(4,1,3)
plot(t, hb, 'k');
ylabel('height (m)')
grid

subplot(4,1,4)
plot(t, NumSats, 'g*');
title('Number of Satellites in View')
ylabel('# of sats (m)')
xlabel('Time (sec)');
grid