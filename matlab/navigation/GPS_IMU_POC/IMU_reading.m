function [w_b__i_b_tilde, f_b__i_b_tilde] = IMU_reading(w_b__i_b, f_b__i_b, constants)
%#codegen
% FUNCTION DESCRIPTION:
%   Simulation of an IMU (ideal error-free case)
%
% INPUTS:
%   w_i__i_b = Angular velocity of {b} wrt {i} resolved in {i} (rad/s)
%   a_i__i_b = Acceleration     of {b} wrt {i} resolved in {i} (m/s^2)
%   constants= A structure containing many constant parameters
%
% OUTPUTS:
%   w_b__i_b_tilde = Gyro  meas (rad/s)
%   f_b__i_b_tilde = Accel meas (m/s^2)
%
% NOTES:
%   - None
%
% REFERENCE:
%   http://mercury.pr.erau.edu/~bruders EE 440 Modern Navigation

persistent b_g_BI b_a_BI                            % Persistent storage class variables

if isempty(b_g_BI)
    b_g_BI       = [0; 0; 0];                       % Initialize the Markov Bias Instability Bias term (rad/s)
    b_a_BI       = [0; 0; 0];                       % Initialize the Markov Bias Instability Bias term (m/s^2)     
end

dt           = constants.dt;    % Time step
I3           = eye(3);          % Identity matrix
Fs           = constants.Fs;    % Sample frequency
 % Gyro  related terms   
b_g_BI_sigma_n = constants.gyro.BI.sigma_n;         % Bias - White noise driving the Bias Instability 1-sigma (rad/s)

tau_g        = constants.gyro.BI.correlation_time;  % Correlation time for the bias instability (sec)
b_g_FB       = constants.gyro.b_g_FB;               % Bias - Fixed Bias term (rad/s)

sigma_ARW   = constants.gyro.sigma_ARW;             % Standard deviation of white noise due to ARW (rad/s)
M_g         = constants.gyro.M_g;                   % The combined Gyro Misalignment/Scale-Factor matrix
G_g         = constants.gyro.G_g;                   % The gyro G-sensitivity matrix (rad/sec/g)

 % Accel related terms   
b_a_BI_sigma_n = constants.accel.BI.sigma_n;        % Bias - White noise driving the Bias Instability 1-sigma (m/s^2)

tau_a        = constants.accel.BI.correlation_time; % Correlation time for the bias instability (sec)
b_a_FB       = constants.accel.b_a_FB;              % Bias - Fixed Bias term (m/s^2)

sigma_VRW   = constants.accel.sigma_VRW;            % Standard deviation of white noise due to VRW (m/s^2)
M_a         = constants.accel.M_a;                  % The combined Gyro Misalignment/Scale-Factor matrix    

for i = 1:length(w_b__i_b)
    %++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    % Gyro Model:
    %++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    % Bias terms
    b_g_BS = constants.gyro.b_g_BS;             % Bias - Bias Stability Bias term (rad/s)
    b_g_BI = exp(-dt/tau_g) * b_g_BI + randn(3,1)*b_g_BI_sigma_n; % Bias - Markov Bias Instability Bias term (rad/s)
    b_g = b_g_BI + b_g_FB + b_g_BS;             % All three bias terms (rad/s)

    % Noise terms
    w_g = sigma_ARW * randn(3,1);        % Gyro noise (rad/sec)

    % Including all of the error terms
    w_b__i_b_tilde(i, :) = (b_g + (I3 + M_g) * w_b__i_b(i, :)' + G_g * f_b__i_b(i, :)' + w_g)';

    %++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    % Accelerometer Model
    %++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    % Bias terms
    b_a_BS = constants.accel.b_a_BS;             % Bias - Bias Stability Bias term (m/s^2)
    b_a_BI = exp(-dt/tau_a) * b_a_BI + randn(3,1)*b_a_BI_sigma_n; % Bias - Markov Bias Instability Bias term (m/s^2)
    b_a = b_a_BI + b_a_FB + b_a_BS;             % All three bias terms (m/s^2)

    % Noise terms
    w_a = sigma_VRW * randn(3,1);        % Accelerometer noise (in m/s^2)

    % Including all of the error terms
    f_b__i_b_tilde(i, :) = (b_a + (I3 + M_a) * f_b__i_b(i, :)' + w_a)'; 
end