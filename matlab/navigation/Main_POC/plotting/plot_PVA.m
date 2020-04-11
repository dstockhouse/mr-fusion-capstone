function plot_PVA(P_truth, V_truth, P_est, V_est, fig_label, constants)
%
% FUNCTION DESCRIPTION:
%   Plots PVA truth. If PVA estimates/measurements are provided will also plot
%   PVA and PVA errors
%
% INPUTS:
%   P_truth = True position (meters)
%   V_truth = True velocity (meters/sec)
%   A_truth = True attitude/orientation as a rotation matrix
%   P_est   = Estimated/measured position (meters)
%   V_est   = Estimated/measured velocity (meters/sec)
%   A_est   = Estimated/measured attitude/orientation as a rotation matrix
%
% OUTPUTS:

I3 = eye(3);                                % 3X3 Indentiy matrix
t_sec = (0:constants.dt:constants.t_end)';  % Simulation time vector
N = length(t_sec);                          % Length of the simulation time vector

%% 3D Plot of trajectory
n = 15;         % Reduce the number of points plotted by factor of 'n'
x =  P_truth(1:n:end, 1);
y = -P_truth(1:n:end, 2);
z = -P_truth(1:n:end, 3);

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

x =  P_est(1:n:end, 1);
y = -P_est(1:n:end, 2);
z = -P_est(1:n:end, 3);

h2 = scatter3(x, y, z, 'o', 'k');

legend([h1, h2], 'Truth', 'Est', 'Location', 'best')
hold off

%% Plot the position

figure('Name',fig_label,'Units', 'normalized', 'Position', [0.5 0.5 0.4 0.4])
subplot(3,2,1)
plot(t_sec, P_truth(:,1),'r', t_sec, P_est(:,1),':k')
ylabel('r_x (m)')
legend('Truth','Est','Location','best')
title(['Position r^t_{tb}'],'interpreter','tex');
subplot(3,2,3)
plot(t_sec, P_truth(:,2),'g', t_sec, P_est(:,2),':k')
ylabel('r_y (m)')
legend('Truth','Est','Location','best')
subplot(3,2,5)
plot(t_sec, P_truth(:,3),'b',t_sec, P_est(:,3),':k')
ylabel('r_z (m)')
legend('Truth','Est','Location','best')
xlabel('Time (sec)')

P_error = P_truth - P_est;  % Position error
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
%% Plot the velocity

figure('Name',fig_label,'Units', 'normalized', 'Position', [0.01 0.01 0.4 0.4])
subplot(3,2,1)
plot(t_sec, V_truth(:,1),'r', t_sec, V_est(:,1),':k')
ylabel('v_x (m/s)')
legend('Truth','Est','Location','best')
title(['Velocity v^t_{tb}'],'interpreter','tex');
subplot(3,2,3)
plot(t_sec, V_truth(:,2),'g', t_sec, V_est(:,2),':k')
ylabel('v_y (m/s)')
legend('Truth','Est','Location','best')
subplot(3,2,5)
plot(t_sec, V_truth(:,3),'b',t_sec, V_est(:,3),':k')
ylabel('v_z (m/s)')
legend('Truth','Est','Location','best')
xlabel('Time (sec)')

V_error = V_truth - V_est;  % Velocity error
V_RMS = [   sqrt(sum(V_error(:,1).^2)/N); 
            sqrt(sum(V_error(:,2).^2)/N); 
            sqrt(sum(V_error(:,3).^2)/N)]; % RMS error in m/s^2

subplot(3,2,2)
plot(t_sec, V_error(:,1),'r')
ylabel('\deltav_x (m/s)')
title(['Velocity Error \deltav^t_{tb}'],'interpreter','tex');
legend(['RMS_{error}=', num2str(V_RMS(1)),' m/s'],'Location','best');
subplot(3,2,4)
plot(t_sec, V_error(:,2),'g')
ylabel('\deltav_y (m/s)')
legend(['RMS_{error}=', num2str(V_RMS(2)),' m/s'],'Location','best');
subplot(3,2,6)
plot(t_sec, V_error(:,3),'b')
ylabel('\deltav_z (m/s)')
xlabel('Time (sec)')
legend(['RMS_{error}=', num2str(V_RMS(3)),' m/s'],'Location','best');

% Plot the attitude

% k_vec = zeros(3,N);                         % Angle/axis representation of A_truth (i.e. C matrix)
% for i=1:N
%     k_vec(:,i)     = dcm2k(A_truth(:,:,i)); % Convert Rotation matrix to an Angle/axis k-vector
% end
% k_vec_est = zeros(3,N);                     % Angle/axis representation of A_est (i.e. C matrix)
% for i=1:N
%     k_vec_est(:,i) = dcm2k(A_est(:,:,i));   % Convert Rotation matrix to an Angle/axis k-vector
% end
% 
% figure('Name',fig_label,'Units', 'normalized', 'Position', [0.5 0.01 0.4 0.4])
% subplot(3,2,1)
% plot(t_sec, k_vec(1,:),'r', t_sec, k_vec_est(1,:),':k')
% ylabel('k_x')
% legend('Truth','Est','Location','best')
% title([' Attitude ', 'C^t_b',' as an Angle/Axis k vector'],'interpreter','tex');
% subplot(3,2,3)
% plot(t_sec, k_vec(2,:),'g', t_sec, k_vec_est(2,:),':k')
% ylabel('k_y')
% legend('Truth','Est','Location','best')
% subplot(3,2,5)
% plot(t_sec, k_vec(3,:),'b',t_sec, k_vec_est(3,:),':k')
% ylabel('k_z')
% legend('Truth','Est','Location','best')
% xlabel('Time (sec)')
% 
% psi_ERR = zeros(3,N);                       % Initialize the attitude error vector
% for i=1:N                                   %                  T
%     dC = A_truth(:,:,i) * A_est(:,:,i)' ;   % Delta_C = C C_est
%     
%     Psi_ERR_cross = dC - I3;                % The approx skew-symmetric matrix version of Psi_ERR
%     psi_ERR(:,i)  = [Psi_ERR_cross(3,2);    % Row 3 Col 2 = x-component of vector
%                      Psi_ERR_cross(1,3);    % Row 1 Col 3 = y-component of vector
%                      Psi_ERR_cross(2,1)];   % Row 2 Col 1 = z-component of vector
% end
% 
% psi_ERR = psi_ERR*180/pi; % Convert to deg
% PHI_RMS = [ sqrt(sum(psi_ERR(1,:).^2)/N); 
%             sqrt(sum(psi_ERR(2,:).^2)/N); 
%             sqrt(sum(psi_ERR(3,:).^2)/N)]; % RMS error in m/s^2
% 
% subplot(3,2,2)
% plot(t_sec, psi_ERR(1,:),'r')
% ylabel('\psi_x (°)')
% title(['Attitude Error \DeltaC^t_b as a \psi vector'],'interpreter','tex');
% legend(['RMS_{error}=', num2str(PHI_RMS(1)),' °'],'Location','best');
% subplot(3,2,4)
% plot(t_sec, psi_ERR(2,:),'g')
% legend(['RMS_{error}=', num2str(PHI_RMS(2)),' °'],'Location','best');
% ylabel('\psi_y (°)')
% subplot(3,2,6)
% plot(t_sec, psi_ERR(3,:),'b')
% ylabel('\psi_z (°)')
% xlabel('Time (sec)')
% legend(['RMS_{error}=', num2str(PHI_RMS(3)),' °'],'Location','best');
end