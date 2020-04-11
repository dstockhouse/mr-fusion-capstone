function plot_P(P_truth_HFs, P_gps, fig_label, constants)
%
% FUNCTION DESCRIPTION:
%   Plots PVA truth. If PVA estimates/measurements are provided will also plot
%   PVA and PVA errors
%
% INPUTS:
%   P_truth = True position (meters)
%   P_gps   = Estimated/measured position from GPS (meters)
%
% OUTPUTS:

imu_Fs = constants.Fs;                              % Sampling frequency of IMU/truth data
gps_Fs = constants.gps.Fs;                          % Sampling frequency of GPS
P_truth = downsample(P_truth_HFs, imu_Fs/gps_Fs);   % Downsampled Truth
P_truth = P_truth';

I3 = eye(3);                                        % 3X3 Indentiy matrix
t_sec = (0:constants.gps.dt:constants.t_end)';      % Simulation time vector
N = length(t_sec);                                  % Length of the simulation time vector

%% 3D Plot of trajectory
n = 1;         % Reduce the number of points plotted by factor of 'n'
x =  P_truth_HFs(1:n:end, 1);
y = -P_truth_HFs(1:n:end, 2);
z = -P_truth_HFs(1:n:end, 3);

figure('Name',fig_label,'Units', 'normalized', 'Position', [0.01 0.5 0.4 0.4])
h1 = stem3(x,y,z,'Marker', 'none');
title('Motion of the UAV (in a North-West-Up frame)')
view(-55, 50);
hold on;
u = gradient(x);   % Directional vector field
v = gradient(y);
w = gradient(z);
scale = 0;
quiver3(x,y,z, u, v, w, scale)
xlabel('x-dir (North in m)')
ylabel('y-dir (West in m)')
zlabel('z-dir (height in m)')

text(0, 0, 0, 'Start   End', 'HorizontalAlignment' , 'center' ,'Color', 'r')
axis equal

x =  P_gps(1:n:end, 1);
y = -P_gps(1:n:end, 2);
z = -P_gps(1:n:end, 3);

h2 = scatter3(x, y, z, 'o', 'k');

legend([h1, h2], 'Truth', 'Est', 'Location', 'best')
hold off

%% Plot the position

figure('Name',fig_label,'Units', 'normalized', 'Position', [0.5 0.5 0.4 0.4])
subplot(3,2,1)
plot(t_sec, P_truth_HFs(:,1),'r', t_sec, P_gps(:,1),':k')
ylabel('r_x (m)')
legend('Truth','Est','Location','best')
title(['Position r^t_{tb}'],'interpreter','tex');
subplot(3,2,3)
plot(t_sec, P_truth_HFs(:,2),'g', t_sec, P_gps(:,2),':k')
ylabel('r_y (m)')
legend('Truth','Est','Location','best')
subplot(3,2,5)
plot(t_sec, P_truth_HFs(:,3),'b',t_sec, P_gps(:,3),':k')
ylabel('r_z (m)')
legend('Truth','Est','Location','best')
xlabel('Time (sec)')

P_error = P_truth_HFs - P_gps;  % Position error
P_RMS = ([ sqrt(sum(P_error(:,1).^2)/N); 
           sqrt(sum(P_error(:,2).^2)/N); 
           sqrt(sum(P_error(:,3).^2)/N)]); % RMS error in m

subplot(3,2,2)
plot(t_sec, P_error(:,1),'r')
ylabel('\deltar_x (m)')
title(['Position Error \deltar^t_{tb}'],'interpreter','tex');
legend(['RMS_{error}=', num2str(P_RMS(1)),' m'],'Location','best');
subplot(3,2,4)
plot(t_sec, P_error(:,2),'g')
ylabel('\deltar_y (m)')
legend(['RMS_{error}=', num2str(P_RMS(2)),' m'],'Location','best');
subplot(3,2,6)
plot(t_sec, P_error(:,3),'b')
ylabel('\deltar_z (m)')
xlabel('Time (sec)')
legend(['RMS_{error}=', num2str(P_RMS(3)),' m'],'Location','best');

[m, i1] = max(abs(P_error(:,1)));
[m, i2] = max(abs(P_error(:,2)));
[m, i3] = max(abs(P_error(:,3)));

fprintf('\nFrom the TAN Mechanization (Indirect):\n')
fprintf('\tMaximum Pos error: = [ %5.2f, %5.2f, %5.2f ] m\n', P_error(i1,1), P_error(i2,2), P_error(i3,3))
fprintf('\tFinal   Pos error: = [ %5.2f, %5.2f, %5.2f ] m\n', P_error(end,1), P_error(end,2), P_error(end,3))
end