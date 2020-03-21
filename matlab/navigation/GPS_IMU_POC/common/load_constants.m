% Script/Function Description:
%   Script which defines many constant parameters

% Set to '0' for Low-fidelity and set to '1' for High-fidelity
constants.fidelity = 1;             % Decide to run high-fidelity or low-fidelity code

%------------------------------------------------------------------------------
%% Sampling rate variables and simulation time
%------------------------------------------------------------------------------
constants.Fs  = 100;                % Sample frequency (Hz)
constants.dt  = 1/constants.Fs;     % Sample interval (sec)
constants.t_start = 0;              % Simulation start time (sec)
constants.t_end = 180;              % Simulation end time (sec)

%------------------------------------------------------------------------------
%% WGS84 Earth model parameters
%------------------------------------------------------------------------------
w_ie = 72.92115167e-6;
constants.w_ie = w_ie;              % Earth's rotational rate (rad/s)
constants.w_i__i_e = [0; 0; w_ie];  % Angular velocity of {e} wrt {i} resolved in {i} (rad/s)
constants.Ohm_i__i_e = [ 0   , -w_ie, 0; ...  % Skew symmetric version of w_i__i_e (rad/s)
                         w_ie,  0   , 0; ...
                         0   ,  0   , 0];
constants.mu = 3.986004418e14;      % Earth's gravitational constant (m^3/s^2)
constants.J2 = 1.082627e-3;         % Earth's second gravitational constant
constants.R0 = 6378137.0;           % Earth's equatorial radius (meters)
constants.Rp = 6356752.314245;      % Earth's polar radius (meters)
constants.e = 0.0818191908425;      % Eccentricity
constants.f = 1 / 298.257223563;    % Flattening