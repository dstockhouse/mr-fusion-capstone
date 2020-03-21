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

%% just compass heading
B = sqrt(compass(:,1).^2 + compass(:,2).^2 + compass(:,3).^2);
thetaI = asin(compass(:,3)./B);
thetaD = asin(compass(:,2)./(B.*cos(thetaI)));
phi = atan2(-compass(:,2),compass(:,1))+thetaD;
heading = phi *180/pi;
figure,
plot(t,heading);
title('Compass Reading as direction changes N to S');
ylabel('degrees');
xlabel('Time (sec)');
grid on;

%% just gyro heading
for k=1:length(gyro(:,3))
    if k == 1
        phi(k) = gyro(k,3).*dT;
    else
        phi(k) = phi(k-1) + gyro(k,3).*dT;
    end
end
gheading = phi.*(180/pi);
figure,
plot(t,gheading);
title('Gyro Reading as direction changes N to S');
ylabel('degrees');
xlabel('Time (sec)');
grid on;
%% Weighted least square
for k=1:length(heading)
    wheading(k) = ((heading(k)/(heading(k)^2))+(gheading(k)/(gheading(k)^2)))/((1/(heading(k)^2))+(1/(gheading(k)^2)));
end
figure,
plot(t,wheading,t,gheading,t,heading);
title('Weighted Least-square Method');
ylabel('degrees');
xlabel('Time (sec)');
grid on;
legend('weighted','gyro','compass');
%% Complementary filter
for k=1:length(heading)
    input1(k) = heading(k)-gheading(k);
    lpf(k) = input1(k)*((heading(k))/((heading(k))+(gheading(k))));
    cheading(k) = gheading(k) + lpf(k);
end
figure,
plot(t,cheading,t,gheading,t,heading);
title('Complimentary Filter');
ylabel('degrees');
xlabel('Time (sec)');
grid on;
legend('Filter','gyro','compass');
%% Kalman filter
g = 9.8;
N = length(t);
b = 0.11;               % Viscous drag (1/s)
x = zeros(3,N);         % Assign memory for the "true" state-vector
F = [1      dT      (dT^2)/2
     0      1       dT
     0      -b      0];
G = eye(3);    
u = [0, 0, pi]';
x(:,1) = [0, 0, 0]';   % x(:,1) = x(0)  Initial Conditions for gyro
sigma_w1 = 0.1;     % Position uncertianty (1-sigma)
sigma_w2 = 0.01;  % Velocity uncertianty (1-sigma)
sigma_w3 = 0.05;  % Accel    uncertianty (1-sigma)

for k = 2:N         
    w = [gyro(k,1), gyro(k,2), gyro(k,3)]';
    x(:,k) = F * x(:,k-1) + G * u + w;                      % The discrete ss-model
end

x_hat = zeros(3,N);         % Assign memory for the state estimate vector
P     = zeros(3,3,N);       % Assign memory for the state error covariance matrix
y     = zeros(2,N);         % Assign memory for the GPS & speed measurent

sigma_compass = deg2rad(10);
sigma_gyro = 0.000611;         % Standard deviation of the gyro sensor
H = [1 0 0; 0 1 0];
for k = 1:N  
    v = [phi(k,1); gyro(k,3)];
    y(:, k) = H * x(:, k) + v;
end

R = diag([sigma_compass^2, sigma_gyro^2]); 

Q = diag([sigma_w1^2, sigma_w2^2, sigma_w3^2]);

x_hat(:, 1) = [0; 0; 0];

P(:,:,1) = diag([sigma_compass^2, sigma_gyro^2, 0]);

for k = 2:N  
    [x_hat(:,k), P(:,:,k)] = Kalman_Filter(x_hat(:,k-1), P(:,:,k-1), y(:,k), F, H, Q, R, G, u);
end
figure,
plot(t, x_hat(1,:),t,gheading,t,heading);
title('Kalman Filter');
ylabel('degrees');
xlabel('Time (sec)');
grid on;
legend('Kalman','gyro','compass');