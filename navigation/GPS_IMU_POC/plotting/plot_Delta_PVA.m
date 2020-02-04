function plot_Delta_PVA(P_error, V_error, psi_ERR, constants)
%
% FUNCTION DESCRIPTION:
%   Plots PVA truth. If PVA estimates/measurements are provided will also plot
%   PVA and PVA errors
%
% INPUTS:
%   P_error = Error in position (meters)
%   V_error = Error in velocity (meters/sec)
%   psi_ERR = Attitude/orientation error as a psi vector
%
% OUTPUTS:

I3 = eye(3);                                % 3X3 Indentiy matrix
t_sec = (0:constants.dt:constants.t_end)';  % Simulation time vector
N = length(t_sec);                          % Length of the simulation time vector


%P_error = P_truth - P_est;  % Position error
P_RMS = ([ sqrt(sum(P_error(:,1).^2)/N); 
           sqrt(sum(P_error(:,2).^2)/N); 
           sqrt(sum(P_error(:,3).^2)/N)]); % RMS error in m

figure('Name','Delta PVA due only to Gyro/Accel Errors','Units', 'normalized', 'Position', [0.25 0.1 0.7 0.4])       
subplot(3,3,1)
plot(t_sec, P_error(:,1),'r')
ylabel('\deltar_x (m)')
title(['Position Error \deltar^t_{tb}'],'interpreter','tex');
legend(['RMS_{error}=', num2str(P_RMS(1)),' m'],'Location','best');
subplot(3,3,4)
plot(t_sec, P_error(:,2),'g')
ylabel('\deltar_y (m)')
legend(['RMS_{error}=', num2str(P_RMS(2)),' m'],'Location','best');
subplot(3,3,7)
plot(t_sec, P_error(:,3),'b')
ylabel('\deltar_z (m)')
xlabel('Time (sec)')
legend(['RMS_{error}=', num2str(P_RMS(3)),' m'],'Location','best');

[m, i1] = max(abs(P_error(:,1)));
[m, i2] = max(abs(P_error(:,2)));
[m, i3] = max(abs(P_error(:,3)));

fprintf('\nFrom the TAN ERROR Mechanization:\n')
fprintf('\tMaximum Pos error: = [ %5.2f, %5.2f, %5.2f ] m\n', P_error(i1,1), P_error(i2,2), P_error(i3,3))
fprintf('\tFinal   Pos error: = [ %5.2f, %5.2f, %5.2f ] m\n', P_error(end,1), P_error(end,2), P_error(end,3))
%% Plot the velocity

%V_error = V_truth - V_est;  % Velocity error
V_RMS = [   sqrt(sum(V_error(:,1).^2)/N); 
            sqrt(sum(V_error(:,2).^2)/N); 
            sqrt(sum(V_error(:,3).^2)/N)]; % RMS error in m/s^2

subplot(3,3,2)
plot(t_sec, V_error(:,1),'r')
ylabel('\deltav_x (m/s)')
title(['Velocity Error \deltav^t_{tb}'],'interpreter','tex');
legend(['RMS_{error}=', num2str(V_RMS(1)),' m/s'],'Location','best');
subplot(3,3,5)
plot(t_sec, V_error(:,2),'g')
ylabel('\deltav_y (m/s)')
legend(['RMS_{error}=', num2str(V_RMS(2)),' m/s'],'Location','best');
subplot(3,3,8)
plot(t_sec, V_error(:,3),'b')
ylabel('\deltav_z (m/s)')
xlabel('Time (sec)')
legend(['RMS_{error}=', num2str(V_RMS(3)),' m/s'],'Location','best');

% Plot the attitude

psi_ERR = psi_ERR'*180/pi;                   % Convert from rad to deg
PHI_RMS = [ sqrt(sum(psi_ERR(1,:).^2)/N); 
            sqrt(sum(psi_ERR(2,:).^2)/N); 
            sqrt(sum(psi_ERR(3,:).^2)/N)];  % RMS error in deg

subplot(3,3,3)
plot(t_sec, psi_ERR(1,:),'r')
ylabel('\psi_x (°)')
title(['Attitude Error \delta\psi as a vector'],'interpreter','tex');
legend(['RMS_{error}=', num2str(PHI_RMS(1)),' °'],'Location','best');
subplot(3,3,6)
plot(t_sec, psi_ERR(2,:),'g')
legend(['RMS_{error}=', num2str(PHI_RMS(2)),' °'],'Location','best');
ylabel('\psi_y (°)')
subplot(3,3,9)
plot(t_sec, psi_ERR(3,:),'b')
ylabel('\psi_z (°)')
xlabel('Time (sec)')
legend(['RMS_{error}=', num2str(PHI_RMS(3)),' °'],'Location','best');
end