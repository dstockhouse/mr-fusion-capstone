function [theta_L, theta_R] = ControlSim(delta_heading)
%--------------------------------------------------------------------------
% Name: ControlSim
% Desc: Simulates the robot's control scheme. Reads in a rate of rotation
% and produces angular velocities for the left and right motors
% Inputs: delta_heading - change in heading in radians
% Outputs: theta_L - angular velocity of left wheel in rad/sec
%          theta_R - angular velocity of right wheel in rad/sec
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/21/2020
%--------------------------------------------------------------------------

%--------------------------------------------------------------------------
% Define robot characteristics
%--------------------------------------------------------------------------
r = .5524;          % radius of the wheels
R = .1524;          % radius of the axle

Vsys = 1;           % constant linear velocity of the system in m/s

Kp = 3;             % controller constant

%--------------------------------------------------------------------------
% Define controller
%--------------------------------------------------------------------------

% Used to determine the center of curvature based on a desired heading
% ICC = [Xpos - R*sin(heading),Ypos + R*cos(heading)];

% D = Vsys/omega;      % distance between center/curvature and center/axle

omega = delta_heading*Kp; % rate of angular change of robot

vL = Vsys - R*omega;      % linear velocity of left wheel
vR = Vsys + R*omega;      % linear velocity of right wheel

theta_L = vL/r;           % angular velocity of left wheel
theta_R = vR/r;           % angular velocity of right wheel
                                 
end % End of function
