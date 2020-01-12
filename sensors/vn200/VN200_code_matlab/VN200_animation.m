% EE 440 Modern Nav
% Real-time animation of the 3D orientation using VN200 gyros
%
% Author: S. Bruder

clear;                          % Clear all variables from the workspace
close all;                      % Close all windows
clc;                            % "Clean" the command window

p = genpath(pwd);               % List all subfolders of the current folder
addpath(p);                     % Add all subfolders the the path (current session)

%% Perform a 10 sec data collection to compute the quiescent gyro bias
Fs = 50;                        % Set Sample frequency <= 100 (Hz)
dT = 1/Fs;                      % Sample interval (sec)
nSec = 10;                      % Duration of data collection (sec)
nSamples = Fs*nSec;             % Number of samples to collect (dimless)
fprintf('Collecting data for %2i sec at %d Hz\n\n', nSec, Fs);

w = zeros(Fs*nSec, 3);          % Initialize the gyro data array: 3 floats per sample

[s, SN] = initialize_VN200_IMU(Fs);   % Initialize the VN200
 for k = 1:nSec*Fs              % Retrieve IMU data from VN200
    [compass, accel, w(k,:), temp, baro] = read_VN200_IMU(s); % Get VN200 IMU data
    if ~mod(nSamples-k+1,Fs)
        fprintf('Please wait %i more secomds!!!\n', round((nSamples-k+1) / Fs));
    end
 end
stop_VN200(s);                  % Terminates the VN200 IMU data transmission

t = 0:dT:(nSamples-1)*dT;   % Time vector (sec)
% Plot the Gyro data (in deg/s)
plot(t, w(:,1)*180/pi, 'r', t, w(:,2)*180/pi, 'g', t, w(:,3)*180/pi, 'b');
title('Plot of Quiescent VN200 Gyro Data', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Angular Rate (deg/s)')
legend('\omega_x', '\omega_y', '\omega_z')
grid

% Compute the Quiescent mean
w_bar = mean(w, 1); % Mean of the quiescent data rad/s
disp(['Gyro Mean = [', num2str(w_bar(1)*180/pi,'%6.4f'), ', ', num2str(w_bar(2)*180/pi,'%6.4f'), ', ', num2str(w_bar(3)*180/pi,'%6.4f'),'] deg/s']);

fprintf('\n --- Now safe to move ---\n')

%% Now Read the IMU data and plot in real-time!!
% Initialize the graphics for coord system
C = eye(3);     % Identity matrix
figure('Position', [100, 100, 800, 800]);
      plot_frame(C, [0;0;0], '0', 'k', 'k', 'k');    % Plot the starting frame
hg1 = plot_frame(C, [0;0;0], '1', 'r', 'g', 'b');    % Plot the rotating frame
title('Real-Time Plot of the VN200 Coordinate Frame', 'FontSize', 14)
axis([-1.2 1.2 -1.2 1.2 -1.2 1.2]);
grid
view(137, 36);          % Set azimuth and elevation of view
hgt = text(1.5, 1.5, 0, '0.0 sec','BackgroundColor',[.7 .9 .7], 'FontSize', 18);

ht = hgtransform;       % Handle to a Homogenous transform object
set(hg1, 'Parent', ht); % Assign the Homo transform to be the parent of rotating frame

% Run at a lower sample freq as we are plotting in real-time
nSec = 30;                      % Duration of data collection (sec)
Fs = 20;                        % Set Sample frequency <= 100 (Hz)
dT = 1/Fs;                      % Sample interval (sec)

[s, SN] = initialize_VN200_IMU(Fs);   % Initialize the VN200

for t = 0:dT:nSec               % Retrieve IMU data from VN200
    [compass, accel, w, temp, baro] = read_VN200_IMU(s); % Get VN200 IMU data
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
   
   % Apply rotational transform on render side (faster)
   set(ht,'Matrix',[C, [0;0;0]; [0,0,0,1]]);
   set(hgt, 'String', [num2str(t,'%4.2f'), ' sec']);
   drawnow;
end
stop_VN200(s);                  % Terminates the VN200 IMU data transmission