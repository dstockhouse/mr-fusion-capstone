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

% The sample rate for the GPS is fixed at 5 Hz
Fs = 10;                        % The IMU update rate
dT = 1/Fs;                      % Sample interval (sec)
nSec = 10;                      % Duration of data collection (sec)
N = Fs*nSec;                    % Number of IMU samples to collect (dimless)
N_GPS = 5*nSec;                 % Number of GPS samples to collect (dimless)
fprintf('Collecting IMU & GPS data for %2i sec at %d Hz\n\n', nSec, Fs);

lat     = zeros(N_GPS,1);       % Initialize the latitude data array: 1 float per sample
lon     = zeros(N_GPS,1);       % Initialize the longitude data array
hb      = zeros(N_GPS,1);       % Initialize the height data array
t_GPS   = zeros(N_GPS,1);       % Initialize the GPS time data array
NumSats = zeros(N_GPS,1);       % Initialize the number of satellites in view array

compass = zeros(N, 3);          % Initialize the compass data array: 3 floats per sample
gyro    = zeros(N, 3);          % Initialize the gyro data array:    3 floats per sample
accel   = zeros(N, 3);          % Initialize the accel data array:   3 floats per sample
temp    = zeros(N,1);           % Initialize the temperature data array: 1 float per sample
baro    = zeros(N,1);           % Initialize the Barometric pressure data array: 1 float per sample

%% Collect VN200 GPS measurements
s = initialize_VN200_GPS_and_IMU(Fs);   % Initialize the VN200
k_imu = 1;
k_gps = 1;
tic
 while toc < nSec                % Retrieve GPS/IMU data from VN200
     vn200_data = read_VN200_GPS_and_IMU(s);
     if isfield(vn200_data, 'lat') % This is GPS data

        t(k_gps)        = vn200_data.t;
        NumSats(k_gps)  = vn200_data.NumSats;
        lat(k_gps)      = vn200_data.lat;
        lon(k_gps)      = vn200_data.lon;
        hb(k_gps)       = vn200_data.hb;
        k_gps = k_gps + 1;
     else       % This is IMU data
        compass(k_imu,:) = vn200_data.compass;
        accel(k_imu,:)   = vn200_data.accel;
        gyro(k_imu,:)    = vn200_data.gyro;
        temp(k_imu,:)    = vn200_data.temp;
        baro(k_imu,:)    = vn200_data.baro;
        k_imu = k_imu +1;
        
        if ~mod(N-k_imu+1,Fs)
            fprintf('Please wait %i more secomds!!!\n', round((N-k_imu+1) / Fs));
        end
     end

 end
stop_VN200(s);                  % Terminates the VN200 IMU data transmission

t = t - t(1);                   % Subtract start time (start at t = 0 secs)

%% Plot the GPS data (in lat, lon, height)
figure('Units', 'normalized','Position', [0.05, 0.05, 0.5, 0.8]);
subplot(4,1,1)
plot(t, lat, 'r+');
title('Plot of VN200 GPS Data', 'FontSize', 14);
ylabel('Lat (°)')
grid

subplot(4,1,2)
plot(t, lon, 'bo');
ylabel('Lon (°)')
grid

subplot(4,1,3)
plot(t, hb, 'kx');
ylabel('height (m)')
grid

subplot(4,1,4)
plot(t, NumSats, 'rd');
title('Number of Satellites in View')
ylabel('# of sats (m)')
xlabel('Time (sec)');
grid

%% Plot the IMU data
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