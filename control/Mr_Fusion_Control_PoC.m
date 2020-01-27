%--------------------------------------------------------------------------
% Name: Control Model Simulation
% Desc: Provides a model of the plant for simulink to use for simulations
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 1/16/2020
%--------------------------------------------------------------------------

clear; clc;

%--------------------------------------------------------------------------
% Robot Characteristics
%--------------------------------------------------------------------------

r = .25; % radius of the wheels
R = 3;   % radius of the axle

%--------------------------------------------------------------------------
% Controller Input
%--------------------------------------------------------------------------

nominal = 0;     % nominal velocity
fudgeFactor = .4; % needed change in velocity
heading = pi;    % heading of the robot

%--------------------------------------------------------------------------
% Controller Model
%--------------------------------------------------------------------------

theta_L = nominal + fudgeFactor;
theta_R = nominal - fudgeFactor;

controller = [theta_L ; theta_R];

%--------------------------------------------------------------------------
% Kinematic Model
%--------------------------------------------------------------------------

transformMatrix = [ -r/2*sin(heading) -r/2*sin(heading) ;... 
                     r/2*cos(heading)  r/2*cos(heading) ;...
                         r/(2*R)           -r/(2*R)     ];
                 
inputMatrix = [ theta_R ; theta_L ];
                 
Kinematics = transformMatrix * inputMatrix;