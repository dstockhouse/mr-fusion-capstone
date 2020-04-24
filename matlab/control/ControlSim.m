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
% Last Modified: 4/22/2020
%--------------------------------------------------------------------------

%--------------------------------------------------------------------------
% Define robot characteristics
%--------------------------------------------------------------------------
r = .5524;          % radius of the wheels
L = .1524;          % radius of the axle

time_step = 0.01;

Vsys = speed;       % constant linear velocity of the system in m/s

%--------------------------------------------------------------------------
% Declare persitent variables
%--------------------------------------------------------------------------
persistent delta_heading_previous_sum;
if isempty(delta_heading_previous_sum)
    delta_heading_previous_sum = 0;
end

persistent delta_heading_previous;
if isempty(delta_heading_previous)
    delta_heading_previous = 0;
end

%--------------------------------------------------------------------------
% Define controller
%--------------------------------------------------------------------------
Kp = 4;          % proportional controller constant
Ki = 0.2;        % integral controller constant

if Vsys == 0
    delta_heading_previous_sum = 0;
end

% PI controller components
P = Kp*delta_heading;
I = Ki*(delta_heading + delta_heading_previous_sum);

omega = P + I; % PI controller block
    
vL = Vsys - omega*L/(2*r); % linear velocity of left wheel
vR = Vsys + omega*L/(2*r); % linear velocity of right wheel

theta_L = vL/r;            % angular velocity of left wheel
theta_R = vR/r;            % angular velocity of right wheel

% Update cumulative error
delta_heading_previous_sum = delta_heading_previous_sum + delta_heading;

end % End of function
