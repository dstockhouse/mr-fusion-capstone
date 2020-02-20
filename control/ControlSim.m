function [theta_L, theta_R] = ControlSim(omega)
%--------------------------------------------------------------------------
% Name: ControlSim
% Desc: Simulates the robot's control scheme. Reads in a rate of rotation
% and produces angular velocities for the left and right motors
% Inputs: omega - rate of rotation for the robot in rad/sec
% Outputs: theta_L - angular velocity of left wheel in rad/sec
%          theta_R - angular velocity of right wheel in rad/sec
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/20/2020
%--------------------------------------------------------------------------

%--------------------------------------------------------------------------
% Define robot characteristics
%--------------------------------------------------------------------------
r = .5524;          % radius of the wheels
R = .1524;          % radius of the axle

Vsys = 1;           % constant linear velocity of the system in m/s

%--------------------------------------------------------------------------
% Define controller
%--------------------------------------------------------------------------

% Used to determine the center of curvature based on a desired heading
% ICC = [Xpos - R*sin(heading),Ypos + R*cos(heading)];

D = Vsys/omega;     % distance between center/curvature and center/axle

vL = omega*(D + R); % linear velocity of left wheel
vR = omega*(D - R); % linear velocity of right wheel

theta_L = vL/r*3;   % angular velocity of left wheel
theta_R = vR/r*3;   % angular velocity of right wheel
                                 
end % End of function
