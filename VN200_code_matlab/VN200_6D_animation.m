% EE 440 Modern Nav
% Real-time animation of the 3D Position and 3D Orientation using VN200 gyros
%
% Author: S. Bruder

clear;                          % Clear all variables from the workspace
close all;                      % Close all windows
clc;                            % "Clean" the command window

p = genpath(pwd);               % List all subfolders of the current folder
addpath(p);                     % Add all subfolders the the path (current session)

load('LSE_cal_params.mat');     % Load the calibration parameters file

%% Perform a 10 sec data collection to compute the quiescent gyro bias
Fs = 50;                        % Set Sample frequency <= 100 (Hz)
dT = 1/Fs;                      % Sample interval (sec)
nSec = 10;                      % Duration of data collection (sec)
nSamples = Fs*nSec;             % Number of samples to collect (dimless)
fprintf('Collecting data for %2i sec at %d Hz\n\n', nSec, Fs);

w   = zeros(3, Fs*nSec);        % Initialize the gyro data array: 3 floats per sample
acc = zeros(3, Fs*nSec);        % Initialize the gyro data array: 3 floats per sample

[s, SN] = initialize_VN200_IMU(Fs);   % Initialize the VN200
 for k = 1:nSec*Fs              % Retrieve IMU data from VN200
    [compass, acc(:,k), w(:,k), temp, baro] = read_VN200_IMU(s); % Get VN200 IMU data
    w(:,k)   = inv(eye(3) + Mg)*(w(:,k)   - b_gFB); % Calibrated Gyro data (rad/s)
    acc(:,k) = inv(eye(3) + Ma)*(acc(:,k) - b_aFB); % Calibrated Accel data (m/s^2)
    if ~mod(nSamples-k+1,Fs)
        fprintf('Please wait %i more secomds!!!\n', round((nSamples-k+1) / Fs));
    end
 end
stop_VN200(s);                  % Terminates the VN200 IMU data transmission

t = 0:dT:(nSamples-1)*dT;   % Time vector (sec)
% Plot the Gyro data (in deg/s)
figure('Units', 'normalized','Position', [0.05, 0.05, 0.3, 0.7]);
subplot(2,1,1)
plot(t, w(1,:)*180/pi, 'r', t, w(2,:)*180/pi, 'g', t, w(3,:)*180/pi, 'b');
title('Plot of Quiescent VN200 Gyro Data', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Angular Rate (°/s)')
legend('\omega_x', '\omega_y', '\omega_z')
grid

% Plot the accel data (in m/s^2)
subplot(2,1,2)
plot(t, acc(1,:), 'r', t, acc(2,:), 'g', t, acc(3,:), 'b');
title('Plot of Quiescent VN200 Accel Data', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Accel (m/s^2)')
legend('a_x', 'a_y', 'a_z')
grid

w_bar = mean(w, 2);     % Mean of the quiescent gyro data (rad/s)
fprintf('\nGyro Mean = [%6.4f, %6.4f, %6.4f] (°/s)\n', w_bar(1)*180/pi, w_bar(2)*180/pi, w_bar(3)*180/pi);

a_bar = mean(acc, 2);   % Mean of the quiescent accel data (m/s^2)
g = norm(a_bar);        % Compute local gravity (m/s^2)
fprintf('Accel Mean = [%6.4f, %6.4f, %6.4f] and g = %6.4f (m/s^2)\n', a_bar(1), a_bar(2), a_bar(3), g);

roll  = asin(a_bar(2) / g);
pitch = asin(-a_bar(1) / g);
fprintf('Initial Roll = %6.4f° and Pitch = %6.4f° \n', roll*180/pi, pitch*180/pi);

fprintf('\n --- Now safe to move ---\n')

%% Now Read the IMU data and plot in real-time!!
C  = rotate_y(pitch) * rotate_x(roll);  % Initialize the graphics for coord system
figure('Units', 'normalized','Position', [0.05, 0.05, 0.6, 0.85]);
      plot_frame(eye(3), [0;0;0], '0', 'k', 'k', 'k');  % Plot the starting frame
hg1 = plot_frame(C, [0;0;0], '1', 'r', 'g', 'b');       % Plot the rotating frame
title('Real-Time Plot of the VN200 Coordinate Frame', 'FontSize', 14)
axis([-2 2 -2 2 -2 2]);
axis square;
grid
view(139, 30);          % Set azimuth and elevation of view
hgt = text(1.5, 1.5, 0, '0.0 sec','BackgroundColor',[.7 .9 .7], 'FontSize', 18);

ht = hgtransform;       % Handle to a Homogenous transform object
set(hg1, 'Parent', ht); % Assign the Homo transform to be the parent of rotating frame

% Run at a lower sample freq as we are plotting in real-time
nSec = 20;                      % Duration of data collection (sec)
Fs = 20;                        % Set Sample frequency <= 100 (Hz)
dT = 1/Fs;                      % Sample interval (sec)
t = 0:dT:nSec;                  % Time vector
vel = zeros(3,1);
pos = zeros(3,1);

[s, SN] = initialize_VN200_IMU(Fs);   % Initialize the VN200

for i = 1:length(t)             % Retrieve IMU data from VN200
    [compass, acc, w, temp, baro] = read_VN200_IMU(s); % Get VN200 IMU data
    acc = inv(eye(3) + Ma)*(acc' - b_aFB);  % Calibrated X-Accel data (m/s^2)
    w   = inv(eye(3) + Mg)*(w'   - b_gFB);   % Calibrated Gyro data (rad/s)
    w = w - w_bar;              % X, Y, Z Gyroscope data - bias (rad/s)
    k = w / norm(w);            % Normalized axis of rotation (dimless)
    theta = norm(w)*dT;         % Angle of rotation (rad)
    K = [   0, -k(3),  k(2);    % Skew-symmetric matrix form of the normalized axis of rotation
         k(3),     0, -k(1); 
        -k(2),  k(1),    0];
    delta_C = eye(3) + sin(theta) * K + (1 - cos(theta)) * K^2; % Rodrigues formula
    C =  C * delta_C;           % Incrementally update the orientation matrix
    
    % Normalize the rotation matrix (round off errors may cause problems)
    C = normalize_C(C);
    
    if i > 1
        acc = C*acc - [0;0;g]; % Rotate the accel meas into the reference coord frame
        vel = vel + dT*acc;
        pos = pos + dT*vel;
    end
   
   % Apply rotational transform on render side (faster)
   set(ht,'Matrix',[C, pos; [0,0,0,1]]);
   set(hgt, 'String', [num2str(t(i),'%4.2f'), ' sec']);
   drawnow;
end
stop_VN200(s);                  % Terminates the VN200 IMU data transmission