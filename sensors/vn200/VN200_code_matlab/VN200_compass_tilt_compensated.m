% EE 440 Modern Nav
% Code to 3D calibrate and implement a tilt-compensated compass
%  - Plots Gyro, accel, Barometer, and compass data
%
% Author: S. Bruder
%
% References:
%   https://www.ngdc.noaa.gov/geomag-web/#igrfwmm (for Prescott, AZ)
%   https://www.ngdc.noaa.gov/geomag/WMM/data/WMM2015/WMM2015_Report.pdf (Pg. 2)

clear;                          % Clear all variables from the workspace
close all;                      % Close all windows
clc;                            % "Clean" the command window

p = genpath(pwd);               % List all subfolders of the current folder
addpath(p);                     % Add all subfolders the the path (current session)

global KEY_IS_PRESSED

% [m_vector, HorizIntensity, Dec°, Inc°, TotalIntensity] = wrldmagm(height(m), lat°, lon°, dyear)
[MagVect, HorInt, DecDeg, IncDeg, TI] = wrldmagm(1532, 34.610023, -112.315720, decyear(2018,02,01), '2015'); %All in nT Nav (NED)
MagVect = MagVect *1e-5;    % Convert from nT to Gauss
TI = TI *1e-5;              % Convert from nT to Gauss

%--------------------------------------------------------------------------
%%  Calibrate the Magnetometer
%--------------------------------------------------------------------------
choice = menu('Magnetometer Calibration?:', 'No Calibration', 'Use Last 3D Calibration', 'New 3D Calibration');
switch choice
    case 1  % No Calibration
        compass_SF   = 1;                   % No scale factor correction
        compass_bias = [0, 0, 0];           % No bias correction
        fprintf('Using the un-calibrated compass!!\n\n')
        
    case 2  % Use Last Calibration
        load compass_cal_params.mat         % Obtain: Fs, compass_bias, compass_SF, SN
        fprintf('VN200 SN %s Prior Cal Parameters:\n', SN);
        fprintf('    Bias = [%4.2f, %4.2f, %4.2f] (Gauss)\n', compass_bias(1), compass_bias(2), compass_bias(3));
        fprintf('    SF   = %4.2f (Gaus/Gaus)\n\n', compass_SF);
                
    case 3  % New Calibration
        fprintf('Please rotate the compass slowly in 3D\n')
        Fs = 50;                            % Set Sample frequency <= 100 (Hz)
        dT = 1/Fs;                          % Sample interval (sec)
        nSec = 30;                          % Duration of data collection (sec)
        nSamples = Fs*nSec;                 % Number of samples to collect (dimless)
        compass_data = zeros(Fs*nSec, 3);   % Initialize the compass data array: 3 floats per sample (Gauss)
        
        [s, SN] = initialize_VN200_IMU(Fs); % Initialize the VN200
        for k = 1:nSec*Fs                   % Retrieve IMU data from VN200 - 
            [compass_data(k,:), accel, gyro, temp, baro] = read_VN200_IMU(s); % Get VN200 IMU data
            if ~mod(nSamples-k+1,Fs)
                fprintf('Continue for %i more secomds!!!\n', round((nSamples-k+1) / Fs));
            end
        end
        stop_VN200(s);                      % Terminates the VN200 IMU data transmission
        scatter3(compass_data(:,1), compass_data(:,2), compass_data(:,3), [], 'r');
        axis equal
        
        [Center, Radius] = sphereFit(compass_data);    % Fit the magnetometer data to a sphere
        compass_bias = Center;              % The offset of the sphere's center is Hard Iron bias
        compass_SF = TI / Radius;           % Compass Scale Factor

        save('compass_cal_params.mat', 'Fs', 'compass_bias', 'compass_SF', 'SN'); % Save the cal parameters
        
        % View the results of the calibration
        [x,y,z] = sphere;       % Generate points for a unit sphere
        figure,
        sh1 = surf(TI*x, TI*y, TI*z, 'FaceAlpha', 0.5); % Plot the ideal 3D sphere
        sh1.EdgeColor = 'none';
        sh1.FaceColor = 'c';
        title(['Calibration of VN200 Magnetometer SN ', num2str(SN)])
        hold on
        scatter3(compass_data(:,1), compass_data(:,2), compass_data(:,3), [], 'r');
        sh2 = surf(Radius*x + Center(1), Radius*y + Center(2), Radius*z + Center(3),'FaceAlpha', 0.2); % Plot the LS-fit 3D sphere
        sh2.EdgeColor = 'none';
        sh2.FaceColor = 'r';
        hold off
        axis equal
        
        fprintf('Ideal Local Magnetic Field: M = [%4.2f, %4.2f, %4.2f]  (Gauss in a NED frame)\n', MagVect(1), MagVect(2), MagVect(3));
        fprintf('VN200 SN %s Cal Parameters:\n', SN);
        fprintf('    Bias = [%4.2f, %4.2f, %4.2f] (Gauss)\n', compass_bias(1), compass_bias(2), compass_bias(3));
        fprintf('    SF   = %4.2f (Gaus/Gaus)\n', compass_SF);
        pause; fprintf('\nCalibration complete. Hit any key to continue!!\n')
end

%--------------------------------------------------------------------------
%%  Now run real-time demo
%--------------------------------------------------------------------------
Fs = 10;                        % Set Sample frequency <= 100 (Hz)

figure,                         % Initialize the plot
hc = compass(1,0, 'r'); hc.LineWidth = 4;
title('Compass Heading','Color','red', 'FontSize',14);
xlabel('Heading (°)');
view([-90 90]); %% Rotate the axes so that North is up!!
set(gcf, 'KeyPressFcn', @myKeyPressFcn);
    
[s, SN] = initialize_VN200_IMU(Fs);   % Initialize the VN200
compass_data = zeros(1, 3);     % Initialize the compass data array: 3 floats per sample
KEY_IS_PRESSED = false;         % Initialize global var

     while ~KEY_IS_PRESSED      % Retrieve IMU data from VN200
        [compass_data, accel, gyro, temp, baro] = read_VN200_IMU(s); % Get VN200 IMU data
        mx = compass_SF * (compass_data(1) - compass_bias(1));
        my = compass_SF * (compass_data(2) - compass_bias(2));
        mz = compass_SF * (compass_data(3) - compass_bias(3));
        
        ax = accel(1);
        ay = accel(2);
        az = accel(3);
        
        % Compute roll and pitch angles using accels:
        pitch = atan2(ax, sqrt(ay^2 + az^2));
        roll  = atan2(-ay, -az);
        
        % Compute tilt compensate mag measurements:
        mxll = cos(pitch)*mx + sin(pitch)*sin(roll)*my + sin(pitch)*cos(roll)*mz;
        myll =                            cos(roll)*my -            sin(roll)*mz;
        
        Heading = atan2(-myll, mxll) + DecDeg*pi/180;  % Compute heading (to True North)
        
        hc = compass(cos(Heading), -sin(Heading), 'r'); hc.LineWidth = 4;
        title(['Compass Heading = ', num2str(round(Heading*180/pi)), '°'],'Color','red', 'FontSize',14)
        text(1  , 1.5, ['pitch = ', num2str(pitch*180/pi,'%+5.0f'), '°'],'Color','blue', 'FontSize',14)
        text(0.9, 1.5, ['roll  = ', num2str(roll*180/pi,'%+5.0f'), '°'],'Color','blue', 'FontSize',14)
        view([-90 90]);         % Rotate the axes so that North is up!!
        pause(0.01);
     end

% A function to interactively stop the script running
function myKeyPressFcn(hObject, event)
    global KEY_IS_PRESSED
    
    KEY_IS_PRESSED  = true;
    disp('Stopped data collection');
end