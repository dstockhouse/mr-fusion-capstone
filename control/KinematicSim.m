function [K, heading_act] = KinematicSim(theta_L , theta_R, time_step, scenario)
%--------------------------------------------------------------------------
% Name: KinematicSim
% Desc: Simulates the robot's kinematics graphically
% Inputs: theta_L - angular velocity of left wheel
%         theta_R - angular velocity of right wheel
%         time_step - the desired amount of time passed between each
%                     iteration of the function call
%         scenario - a string to serve as the graph title (debug only)
% Outputs: K - a 3x1 matrix detailing the kinematics
%          heading_act - the heading of the robot
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/21/2020
%--------------------------------------------------------------------------

%--------------------------------------------------------------------------
% Declare the current heading as a persistent variable
%--------------------------------------------------------------------------
persistent heading;
if isempty(heading)
    heading = 0; 
end

%--------------------------------------------------------------------------
% Denote and draw starting position
%--------------------------------------------------------------------------
persistent Xpos;
if isempty(Xpos)
    Xpos = 0;
end

persistent Ypos;
if isempty(Ypos)
    Ypos = -1;
end

%--------------------------------------------------------------------------
% Define robot characteristics
%--------------------------------------------------------------------------
r = .5524; % radius of the wheels
R = .1524; % radius of the axle

%----------------------------------------------------------------------                           
% Remove previous figure instance
%----------------------------------------------------------------------
clf('reset')

%----------------------------------------------------------------------
% Create figure
%----------------------------------------------------------------------
xlim([-4 4]);
ylim([-4 4]);
grid on
daspect([1 1 1])
xlabel('Meters');
ylabel('Meters');
title(scenario);
axis square

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
% Update robot's position
%----------------------------------------------------------------------
Xpos = Xpos+K(1,1)*time_step; Ypos = Ypos+K(2,1)*time_step;
    
%----------------------------------------------------------------------
% Draw robot's new position
%----------------------------------------------------------------------
% Base
line([Xpos-0.24*cos(heading) Xpos+0.24*cos(heading)], ...
     [Ypos-0.24*sin(heading) Ypos+0.24*sin(heading)], 'Linewidth',3);
% Pointer
line([((Xpos+0.24*sin(heading))+(Xpos-0.24*sin(heading)))/2 ...
        Xpos+0.24*cos(heading+pi/2)] , ...
     [((Ypos+0.24*cos(heading))+(Ypos-0.24*cos(heading)))/2 ...
        Ypos+0.24*sin(heading+pi/2)], 'Linewidth',3);
    
%----------------------------------------------------------------------
% Update function
%----------------------------------------------------------------------
pause(time_step); % simulate time step
    
heading = heading + K(3,1)*time_step; % update heading

heading_act = heading; % return heading as output
                                 
end % End of function
