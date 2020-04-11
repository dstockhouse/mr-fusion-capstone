function plot_IMU(w_b__i_b, f_b__i_b, w_b__i_b_tilde, f_b__i_b_tilde, constants)
%
% FUNCTION DESCRIPTION:
%   Plots IMU measurements vs true angular velociyt and true specific force
%
% INPUTS:
%   w_b__i_b        = Ideal (error-free) Gyro measurements (rad/s)
%   f_b__i_b        = Ideal (error-free) Accel measurements (m/s^2)
%   w_b__i_b_tilde  = Gyro measurements (rad/s)
%   f_b__i_b_tilde  = Accel measurements (m/s^2)
%

t_sec = (constants.t_start:constants.dt:constants.t_end)';        % Simulation time vector

% Plot the angular velocity
w_b__i_b        = w_b__i_b       * 180/pi;  % Convert to deg/s
w_b__i_b_tilde  = w_b__i_b_tilde * 180/pi;  % Convert to deg/s

figure('Units', 'normalized', 'Position', [0.25 0.5 0.4 0.4])
subplot(3,2,1)
plot(t_sec, w_b__i_b_tilde(1,:),'k.',t_sec, w_b__i_b(1,:),'r', 'MarkerSize',2, 'LineWidth', 2)
ylabel('\omega_x (°/s)')
legend('Meas','Truth')
title(['Ground Truth vs Meas Angular vel'],'interpreter','tex');
subplot(3,2,3)
plot(t_sec, w_b__i_b_tilde(2,:),'k.', t_sec, w_b__i_b(2,:),'g', 'MarkerSize',2, 'LineWidth', 2)
ylabel('\omega_y (°/s)')
legend('Meas','Truth')
subplot(3,2,5)
plot(t_sec, w_b__i_b_tilde(3,:),'k.', t_sec, w_b__i_b(3,:),'b', 'MarkerSize',2, 'LineWidth', 2)
ylabel('\omega_z (°/s)')
legend('Meas','Truth')
xlabel('Time (sec)')

subplot(3,2,2)
plot(t_sec, w_b__i_b(1,:) - w_b__i_b_tilde(1,:),'r.', 'MarkerSize',2)
ylabel('\delta\omega_x (°/s)')
title(['\delta\omega^b_{ib} = (Ground Truth - Meas Angular vel)'],'interpreter','tex');
subplot(3,2,4)
plot(t_sec, w_b__i_b(2,:) - w_b__i_b_tilde(2,:),'g.', 'MarkerSize',2)
ylabel('\delta\omega_y (°/s)')
subplot(3,2,6)
plot(t_sec, w_b__i_b(3,:) - w_b__i_b_tilde(3,:),'b.', 'MarkerSize',2)
ylabel('\delta\omega_z (°/s)')
xlabel('Time (sec)')

% Plot the specific force
figure('Units', 'normalized', 'Position', [0.25 0.01 0.4 0.4])
subplot(3,2,1)
plot(t_sec, f_b__i_b_tilde(1,:),'k.', t_sec, f_b__i_b(1,:),'r', 'MarkerSize',2, 'LineWidth', 2)
ylabel('a_x (m/s^2)')
legend('Meas','Truth')
title(['Ground Truth vs Meas Specific Force'],'interpreter','tex');
subplot(3,2,3)
plot(t_sec(2:end), f_b__i_b_tilde(2,2:end),'k.', t_sec(2:end), f_b__i_b(2,2:end),'g', 'MarkerSize',2, 'LineWidth', 2)
ylabel('a_y (m/s^2)')
legend('Meas','Truth')
subplot(3,2,5)
plot(t_sec, f_b__i_b_tilde(3,:),'k.', t_sec, f_b__i_b(3,:),'b', 'MarkerSize',2, 'LineWidth', 2)
ylabel('a_z (m/s^2)')
legend('Meas','Truth')
xlabel('Time (sec)')


subplot(3,2,2)
plot(t_sec, f_b__i_b(1,:) - f_b__i_b_tilde(1,:),'r.', 'MarkerSize',2)
ylabel('\deltaa_x (m/s^2)')
title(['\deltaf ^b_{ib} = (Ground Truth - Meas Specific Force)'],'interpreter','tex');
subplot(3,2,4)
plot(t_sec, f_b__i_b(2,:) - f_b__i_b_tilde(2,:),'g.', 'MarkerSize',2)
ylabel('\deltaa_y (m/s^2)')
subplot(3,2,6)
plot(t_sec, f_b__i_b(3,:) - f_b__i_b_tilde(3,:),'b.', 'MarkerSize',2)
ylabel('\deltaa_z (m/s^2)')
xlabel('Time (sec)')

end