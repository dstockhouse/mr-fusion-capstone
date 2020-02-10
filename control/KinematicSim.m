function K = KinematicSim(theta_L , theta_R)
%--------------------------------------------------------------------------
% Name: KinematicSim
% Desc: Simulates the robot's kinematics graphically
% Inputs: theta_L - angular velocity of left wheel
%         theta_R - angular velocity of right wheel
% Outputs: K - a 3x1 matrix detailing the kinematics
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/10/2020
%--------------------------------------------------------------------------

%--------------------------------------------------------------------------
% Declare the heading as a persistent variable
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
    Ypos = 0;
end

%--------------------------------------------------------------------------
% Define robot characteristics
%--------------------------------------------------------------------------
r = .5524; % radius of the wheels
R = .1524; % radius of the axle

%----------------------------------------------------------------------                           
% Remove previous instance
%----------------------------------------------------------------------
clf('reset')

xlim([-5 5]);
ylim([-5 5]);
grid on
daspect([1 1 1])
axis normal

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
    Xpos = Xpos+K(1,1); Ypos = Ypos+K(2,1);
    
    %----------------------------------------------------------------------
    % Draw Robot's new position
    %----------------------------------------------------------------------
    % Base
    line([Xpos-0.24*cos(heading) Xpos+0.24*cos(heading)], ...
         [Ypos-0.24*sin(heading) Ypos+0.24*sin(heading)], 'Linewidth',3);
    % Pointer
    line([((Xpos+0.24*sin(heading))+(Xpos-0.24*sin(heading)))/2 ...
            Xpos+0.24*cos(heading+pi/2)] , ...
         [((Ypos+0.24*cos(heading))+(Ypos-0.24*cos(heading)))/2 ...
            Ypos+0.24*sin(heading+pi/2)], 'Linewidth',3);
    
    pause(0.5); % Simulate time step
    
    heading = heading + K(3,1); % Update heading
                                 
end % End of function