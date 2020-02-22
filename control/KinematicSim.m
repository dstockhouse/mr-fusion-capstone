function [K, heading_act] = KinematicSim(theta_L , theta_R, time_step)
%--------------------------------------------------------------------------
% Name: KinematicSim
% Desc: Simulates the robot's kinematics. Reads in a desired angular
%       velocity for each motor and produces the robot's simulated change
%       in the X/Y directions as well as its orientation.
% Inputs: theta_L - angular velocity of left wheel
%         theta_R - angular velocity of right wheel
%         time_step - the desired amount of time passed between each
%                     iteration of the function call
% Outputs: K - a 3x1 matrix detailing the system kinematics
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/22/2020
%--------------------------------------------------------------------------

%--------------------------------------------------------------------------
% Declare the current heading as a persistent variable
%--------------------------------------------------------------------------
persistent heading;
if isempty(heading)
    heading = 0; 
end

%--------------------------------------------------------------------------
% Define robot characteristics
%--------------------------------------------------------------------------
r = .5524; % radius of the wheels
R = .1524; % radius of the axle

%----------------------------------------------------------------------
% Create kinematic model
%----------------------------------------------------------------------
transMatrix = [-r/2*sin(heading) -r/2*sin(heading);... 
                r/2*cos(heading)  r/2*cos(heading);...
                    r/(2*R)           -r/(2*R)   ];

inMat = [ theta_R ; theta_L ]; % input vector

                            %     |  X_dot  | -> velocity in x
K = transMatrix * inMat;    % K = |  Y_dot  | -> velocity in y
                            %     | PSI_dot | -> angular velocity
                               
%----------------------------------------------------------------------
% Update Kinematics
%----------------------------------------------------------------------
pause(time_step); % simulate time step
    
heading = heading + K(3,1)*time_step; % update heading

heading_act = heading; % return heading as output
                                 
end % End of function
