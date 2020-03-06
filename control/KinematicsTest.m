%--------------------------------------------------------------------------
% Name: KinematicsTest
% Desc: Tests the KinematicSim function using pre-determined angular
%       velocities. The robot's position is remembered between function
%       calls during a single run of this script. Creating a path for the
%       robot involves applying angular velocities to both motors.
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/10/2020
%--------------------------------------------------------------------------

close; clc; clear KinematicSim;

%--------------------------------------------------------------------------
% Declare timestep
%--------------------------------------------------------------------------

time_step = 0.01;

%--------------------------------------------------------------------------
% Create robot path
%--------------------------------------------------------------------------

for i = 0:time_step:.4
    KinematicSim(0.3, i, time_step)
end

for j = 0:time_step:4
    KinematicSim(0.3, 0.3, time_step)
end
