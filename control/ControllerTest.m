%--------------------------------------------------------------------------
% Name: ControllerTest
% Desc: Tests the KinematicSim and ControlSim function using pre-determined angular
%       velocities. The robot's position is remembered between function
%       calls during a single run of this script. Creating a path for the
%       robot involves applying angular velocities to both motors.
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/17/2020
%--------------------------------------------------------------------------

close; clc; clear KinematicSim; clear ControlSim

%--------------------------------------------------------------------------
% Declare timestep
%--------------------------------------------------------------------------

time_step = 0.01;

%--------------------------------------------------------------------------
% Create Scenario
%--------------------------------------------------------------------------

for i = 0:0.01:.1
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(pi/2);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step);
    
end

for i = 0:0.01:1
    
    % Determine angular wheel velocities from ControlSim()
    theta_L = 0.5;
    theta_R = 0.5;
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step);
    
end
