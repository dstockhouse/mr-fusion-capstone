%% EE 440 Final Project Script II
% Autor: Emmett Hamman
% Description: This script plots the magnetometer data collected from the
%              VN-200 INS 

clear;
close all;
clc;

%% Library Test
%Load the data from VN-200
load('magnetTestLibrary.mat');
dT = 1/Fs;
nSec = 10;                    
N = Fs*nSec;
t = 0:dT:(N-1)*dT;  

% Plot the Given Data
figure();
plot(t, compass(:,1), 'r', t, compass(:,2), 'g', t, compass(:,3), 'b');
title('Plot of VN200 Compass Data (Library)', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Magnetic Field (Gauss)')
legend('m_x', 'm_y', 'm_z')
grid

% Givens
dec_angle = 10.6;
dec_angle_rad = dec_angle * (pi/180);
% Calculate Values
compass_mean = [0, 0, 0];
compass_mean(1) = mean(compass(:,1));
compass_mean(2) = mean(compass(:,2));
compass_mean(3) = mean(compass(:,3));

full_inten_meas = sqrt(compass_mean(1)^2 + compass_mean(2)^2 + compass_mean(3)^2);

% Determine True Heading
psi_rad = atan2(-compass_mean(2), compass_mean(1)) + dec_angle_rad;
psi = psi_rad * (180/pi);
fprintf('Library - True Heading of the VN-200 (deg): %.2f\n',psi);

% Create a Plot for That Ish
figure();
h3Plot = plot3(NaN,NaN,NaN,'-');
hold

%Plot the Mag Axes
plot3([0 1], [0 0], [0 0], 'red')
plot3([0 0], [-1 0], [0 0], 'green')
plot3([0 0], [0 0], [-1 0], 'blue')

text(1, 0, 0, 'X_b')
text(0, -1, 0, 'Y_b')
text(0, 0, -1, 'Z_b')

%Plot Measured Mag Vecotr
plot3([0 compass_mean(1)],[0 compass_mean(2)],[0 compass_mean(3)], 'black')
text(compass_mean(1), compass_mean(2), compass_mean(3), 'm^n_b_,_l_l')

%Plot ideal Mag Vector
geo_Lat = 34.6163;
geo_Lon = -112.4490;
[m_n_i, hor_inten, dec_angle, inc_angle, full_inten] = wrldmagm(0, geo_Lat, geo_Lon, decyear(2015,1,2));

plot3([0  m_n_i(1)*1e-5], [0, -m_n_i(2)*1e-5], [0, -m_n_i(3)*1e-5], 'black');
text(m_n_i(1)*1e-5, -m_n_i(2)*1e-5, -m_n_i(3)*1e-5, 'm^n_i')

% Plot Properties
view(3)
grid on
title('Earth''s Magnetic Field Vector (Library)')
xlabel('X')
ylabel('Y')
zlabel('Z')
axis([-2 2 -2 2 -2 2])

% Rename compass array
magnet_Library = compass;
clear compass

% Plot true heading on compass
psi_rounded_lib = round(psi);
[x, y] = pol2cart(-psi_rad, 1);
figure();
compass(x, y, 'r');
title(['True Heading (Library): ' num2str(psi_rounded_lib), ' deg'],'Color','red');
view([-90 90]);

% Calculate Differences
per_err_lib = ((full_inten_meas - (full_inten*1e-5)) / (full_inten*1e-5)) *100;
fprintf('Measured Total Intensity (Library): %0.2f Gauss\n', full_inten_meas);
fprintf('Percent Error From Ideal (Library): %0.2f%%\n',per_err_lib);
%% King Engineering Test
clear;

%Load the data from VN-200
load('magnetTestKing.mat');
dT = 1/Fs;
nSec = 10;                    
N = Fs*nSec;
t = 0:dT:(N-1)*dT;  

% Plot the Given Data
figure();
plot(t, compass(:,1), 'r', t, compass(:,2), 'g', t, compass(:,3), 'b');
title('Plot of VN200 Compass Data (King)', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Magnetic Field (Gauss)')
legend('m_x', 'm_y', 'm_z')
grid

% Givens
dec_angle = 10.6;
dec_angle_rad = dec_angle * (pi/180);

% Calculate Values
compass_mean = [0, 0, 0];
compass_mean(1) = mean(compass(:,1));
compass_mean(2) = mean(compass(:,2));
compass_mean(3) = mean(compass(:,3));

full_inten_meas = sqrt(compass_mean(1)^2 + compass_mean(2)^2 + compass_mean(3)^2);

% Determine True Heading
psi_rad = atan2(-compass_mean(2), compass_mean(1)) + dec_angle_rad;
psi = psi_rad * (180/pi);
fprintf('King - True Heading of the VN-200 (deg): %.2f\n',psi);

% Plot That Ish
figure();
h3Plot = plot3(NaN,NaN,NaN,'-');
hold

%Plot the Mag Axes
plot3([0 1], [0 0], [0 0], 'red')
plot3([0 0], [-1 0], [0 0], 'green')
plot3([0 0], [0 0], [-1 0], 'blue')

text(1, 0, 0, 'X_b')
text(0, -1, 0, 'Y_b')
text(0, 0, -1, 'Z_b')

%Plot the VN-200 Data
plot3([0 compass_mean(1)],[0 compass_mean(2)],[0 compass_mean(3)], 'black')
text(compass_mean(1)-.5, compass_mean(2), compass_mean(3), 'm^n_b_,_l_l')

%Plot ideal Mag Vector
geo_Lat = 34.6143;
geo_Lon = -112.4511;
[m_n_i, hor_inten, dec_angle, inc_angle, full_inten] = wrldmagm(0, geo_Lat, geo_Lon, decyear(2015,1,2));
plot3([0  m_n_i(1)*1e-5], [0, -m_n_i(2)*1e-5], [0, -m_n_i(3)*1e-5], 'black');
text(m_n_i(1)*1e-5, -m_n_i(2)*1e-5, -m_n_i(3)*1e-5, 'm^n_i')

view(3)
grid on
title('Earth''s Magnetic Field Vector (King)')
xlabel('X')
ylabel('Y')
zlabel('Z')
axis([-2 2 -2 2 -2 2])

% Rename compass array
magnet_King = compass;
clear compass

% Plot true heading on compass
psi_rounded_king = round(psi);
[x, y] = pol2cart(-psi_rad, 1);
figure();
compass(x, y, 'r');
title(['True Heading (King): ' num2str(psi_rounded_king), ' deg'],'Color','red');
view([-90 90]);

% Calculate Differences
per_err_king = ((full_inten_meas - (full_inten*1e-5)) / (full_inten*1e-5)) * 100;
fprintf('Measured Total Intensity (King): %0.2f Gauss\n', full_inten_meas);
fprintf('Percent Error From Ideal (King): %0.2f%%\n',per_err_king);

%% Outside Test
%Load the data from VN-200
load('magnetTestOutside.mat');
dT = 1/Fs;
nSec = 10;                    
N = Fs*nSec;
t = 0:dT:(N-1)*dT;  

% Plot the Given Data
figure();
plot(t, compass(:,1), 'r', t, compass(:,2), 'g', t, compass(:,3), 'b');
title('Plot of VN200 Compass Data (Outside)', 'FontSize', 12);
xlabel('Time (sec)');
ylabel('Magnetic Field (Gauss)')
legend('m_x', 'm_y', 'm_z')
grid

% Givens
dec_angle = 10.6;
dec_angle_rad = dec_angle * (pi/180);
% Calculate Values
compass_mean = [0, 0, 0];
compass_mean(1) = mean(compass(:,1));
compass_mean(2) = mean(compass(:,2));
compass_mean(3) = mean(compass(:,3));

full_inten_meas = sqrt(compass_mean(1)^2 + compass_mean(2)^2 + compass_mean(3)^2);

% Determine True Heading
psi_rad = atan2(-compass_mean(2), compass_mean(1)) + dec_angle_rad;
psi = psi_rad * (180/pi);
fprintf('Library - True Heading of the VN-200 (deg): %.2f\n',psi);

% Create a Plot for That Ish
figure();
h3Plot = plot3(NaN,NaN,NaN,'-');
hold

%Plot the Mag Axes
plot3([0 1], [0 0], [0 0], 'red')
plot3([0 0], [-1 0], [0 0], 'green')
plot3([0 0], [0 0], [-1 0], 'blue')

text(1, 0, 0, 'X_b')
text(0, -1, 0, 'Y_b')
text(0, 0, -1, 'Z_b')

%Plot Measured Mag Vecotr
plot3([0 compass_mean(1)],[0 compass_mean(2)],[0 compass_mean(3)], 'black')
text(compass_mean(1), compass_mean(2), compass_mean(3), 'm^n_b_,_l_l')

%Plot ideal Mag Vector
geo_Lat = 34.6165;
geo_Lon = -112.4492;
[m_n_i, hor_inten, dec_angle, inc_angle, full_inten] = wrldmagm(0, geo_Lat, geo_Lon, decyear(2015,1,2));

plot3([0  m_n_i(1)*1e-5], [0, -m_n_i(2)*1e-5], [0, -m_n_i(3)*1e-5], 'black');
text(m_n_i(1)*1e-5, -m_n_i(2)*1e-5, -m_n_i(3)*1e-5, 'm^n_i')

% Plot Properties
view(3)
grid on
title('Earth''s Magnetic Field Vector (Outside)')
xlabel('X')
ylabel('Y')
zlabel('Z')
axis([-2 2 -2 2 -2 2])

% Rename compass array
magnet_Library = compass;
clear compass

% Plot true heading on compass
psi_rounded_lib = round(psi);
[x, y] = pol2cart(-psi_rad, 1);
figure();
compass(x, y, 'r');
title(['True Heading (Outside): ' num2str(psi_rounded_lib), ' deg'],'Color','red');
view([-90 90]);

% Calculate Differences
per_err_lib = ((full_inten_meas - (full_inten*1e-5)) / (full_inten*1e-5)) *100;
fprintf('Measured Total Intensity (Outside): %0.2f Gauss\n', full_inten_meas);
fprintf('Percent Error From Ideal (Outside): %0.2f%%\n',per_err_lib);