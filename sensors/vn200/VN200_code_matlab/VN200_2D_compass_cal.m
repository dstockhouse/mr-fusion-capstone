% EE 440 Modern Nav
% Test code for the VN200 IMU
%  - Plots Gyro, accel, Barometer, and compass data
%
% Author: S. Bruder

clear;                          % Clear all variables from the workspace
close all;                      % Close all windows
clc;                            % "Clean" the command window

p = genpath(pwd);               % List all subfolders of the current folder
addpath(p);                     % Add all subfolders the the path (current session)

global KEY_IS_PRESSED

Fs = 50;                        % Set Sample frequency <= 100 (Hz)
dT = 1/Fs;                      % Sample interval (sec)
nSec = 10;                      % Duration of data collection (sec)
nSamples = Fs*nSec;             % Number of samples to collect (dimless)
    
choice = menu('Select an option:','Collect New Compass Data','Use Existing Compass Data');

if choice == 1
    fprintf('Collecting data for %2i sec at %d Hz\n\n', nSec, Fs);

    compass_data = zeros(Fs*nSec, 3);    % Initialize the compass data array: 3 floats per sample
    
    [s, SN] = initialize_VN200_IMU(Fs);   % Initialize the VN200
    
     for k = 1:nSec*Fs              % Retrieve IMU data from VN200
        [compass_data(k,:), accel, gyro, temp, baro] = read_VN200_IMU(s); % Get VN200 IMU data
        if ~mod(nSamples-k+1,Fs)
            fprintf('Please wait %i more secomds!!!\n', round((nSamples-k+1) / Fs));
        end
     end
    
    stop_VN200(s);                  % Terminates the VN200 IMU data transmission
    save('compass_cal.mat', 'Fs', 'compass_data')
else
    load('compass_cal.mat');
    dT = 1/Fs;                      % Sample interval (sec)
    nSamples = length(compass_data);
end

t = 0:dT:(nSamples-1)*dT;       % Time vector (sec)
    
% Plot the compass data (in Gauss)
figure,
plot(t, compass_data(:,1), 'r', t, compass_data(:,2), 'g', t, compass_data(:,3), 'b');
title('Plot of VN200 Compass Data', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Magnetic Field (Gauss)')
legend('m_x', 'm_y', 'm_z')
grid

% Plot the x-axis vs y-axis compass data
figure,
scatter(compass_data(:,1)*1e3, compass_data(:,2)*1e3, [], 'b')
title('Plot of the Uncompensated X/Y Compass Data')
xlabel('M_x (mGauss)')
ylabel('M_y (mGauss)')
grid
ax = gca;
ax.XAxisLocation = 'origin';  % setting x-axis location to origin
ax.YAxisLocation = 'origin';  % setting y-axis location to origin
axis equal;

hold on
[z, r, residual] = fitcircle(compass_data(:,1:2)*1e3, 'linear'); % Fit data to a circle
theta = 0:0.01:2*pi;
plot(z(1) + r*cos(theta), z(2) + r*sin(theta), 'r', z(1), z(2), 'r+')

%--------------------------------------------------------------------------
%%  Now run real-time demo
%--------------------------------------------------------------------------
fprintf('\n\nHit any key to continue to real-time demo!!\n\n'); pause
[MagVect, HorInt, DecDeg, IncDeg, TI] = wrldmagm(1532, 34.610023, -112.315720, decyear(2017,10,16), '2015'); %All in nT Nav (NED) frame
%https://www.ngdc.noaa.gov/geomag/WMM/data/WMM2015/WMM2015_Report.pdf Pg. 2
magXY = sqrt(MagVect(1,1)^2 + MagVect(2,1)^2);  % Magnitude of x and y mag field values
mag_PresAZXY = (magXY*10^-9)/0.0001;            % Convert to mG

SFXY = mag_PresAZXY / r; %Scale Factor

% Initialize the plot
figure,
hc = compass(1,0, 'r'); hc.LineWidth = 4;
title('Compass Heading','Color','red', 'FontSize',14);
xlabel('Heading (°)');
view([-90 90]); %% Rotate the axes so that North is up!!
set(gcf, 'KeyPressFcn', @myKeyPressFcn);
    
[s, SN] = initialize_VN200_IMU(10);   % Initialize the VN200
compass_data = zeros(1, 3);    % Initialize the compass data array: 3 floats per sample
KEY_IS_PRESSED = false;

     while ~KEY_IS_PRESSED              % Retrieve IMU data from VN200
        [compass_data, accel, gyro, temp, baro] = read_VN200_IMU(s); % Get VN200 IMU data
        calMagX = compass_data(1) - z(1)*SFXY;
        calMagY = compass_data(2) - z(2)*SFXY;
        Heading = atan2(calMagY, calMagX);
        hc = compass(cos(Heading), sin(Heading), 'r'); hc.LineWidth = 4;
        title(['Compass Heading = ', num2str(round(Heading*180/pi)), '°'],'Color','red', 'FontSize',14)
        view([-90 90]); %% Rotate the axes so that North is up!!
        pause(0.01);
     end
     
function myKeyPressFcn(hObject, event)
global KEY_IS_PRESSED
KEY_IS_PRESSED  = true;
disp('Stopped data collection');
end