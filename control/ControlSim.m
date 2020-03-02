function [theta_L, theta_R] = ControlSim(delta_heading, speed)
%--------------------------------------------------------------------------
% Name: ControlSim
% Desc: Simulates the robot's control scheme. Reads in a desired heading
%       change and produces angular velocities for the motors.
% Inputs: delta_heading - change in heading in radians
%         speed - a command to determine whether or not the robot should be
%                 moving while turning
% Outputs: theta_L - angular velocity of left wheel in rad/sec
%          theta_R - angular velocity of right wheel in rad/sec
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 3/01/2020
%--------------------------------------------------------------------------

%--------------------------------------------------------------------------
% Define robot characteristics
%--------------------------------------------------------------------------
r = .5524;          % radius of the wheels
R = .1524;          % radius of the axle

Vsys = speed;       % constant linear velocity of the system in m/s

Kp = 3;             % controller constant

%--------------------------------------------------------------------------
% Define controller
%--------------------------------------------------------------------------

if Vsys == 0                  % Set wheel velocities manually if a system 
    theta_L = 1;             % velocity of 0 is received
    theta_R = -1;
    
else
    omega = delta_heading*Kp; % rate of angular change of robot

    vL = Vsys - R*omega;      % linear velocity of left wheel
    vR = Vsys + R*omega;      % linear velocity of right wheel

    theta_L = vL/r;           % angular velocity of left wheel
    theta_R = vR/r;           % angular velocity of right wheel
    
end

end % End of function
