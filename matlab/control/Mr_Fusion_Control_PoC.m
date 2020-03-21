function [K , heading] = ControlSim(theta_L , theta_R , heading, t_step, t)
%--------------------------------------------------------------------------
% Name: ControlSim
% Desc: Simulates the robot's kinematics and control scheme graphically
% Inputs: theta_L - angular velocity of left wheel
%         thets_R - angular velocity of right wheel
%         heading - magnetic heading of the robot
%         t_step - time step for the robot to move
%         time - the time to run the simulation for
% Outputs: K - a 3x1 matrix detailing the kinematics
%          heading - magnetic heading of the robot
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/01/2020
%--------------------------------------------------------------------------

close; clc;

%--------------------------------------------------------------------------
% Define robot characteristics
%--------------------------------------------------------------------------
r = .25; % radius of the wheels
R = 1;   % radius of the axle

%--------------------------------------------------------------------------
% Draw World Graph
%--------------------------------------------------------------------------
figure
xlim([-5 5]);
ylim([-5 5]);
grid on
    
%--------------------------------------------------------------------------
% Denote and draw starting position
%--------------------------------------------------------------------------
Bx1 = 0; Bx2 = 0; By1 = 0; By2 = 0; % Base
% Lx1 = -1; Lx2 = -1; Ly1 = -.5; Ly2 = -.5; % Left Wheel
% Rx1 = 2; Rx2 = 2; Ry1 = -2; Ry2 = 2; % Right Wheel
% Px1 = 0; Px2 = 0; Py1 = 0; Py2 = 0; % Pointer

C1 = line([Bx1 Bx2],[By1 By2]); % Base
% C2 = line([Lx1 Lx2],[Ly1 Ly2]); % Left Wheel
% C3 = line([Rx1 Rx2],[Ry1 Ry2]); % Right Wheel
% C4 = line([Px1 Px2],[Py1 Py2]); % Pointer
                    
%--------------------------------------------------------------------------
% Simulate for denoted time
%--------------------------------------------------------------------------
for n = 0:t_step:t
    
    %----------------------------------------------------------------------
    % Create kinematic model
    %----------------------------------------------------------------------
    transMatrix = [-r/2*sin(heading) -r/2*sin(heading);... 
                    r/2*cos(heading)  r/2*cos(heading);...
                        r/(2*R)           -r/(2*R)    ];

    inMat = [ theta_R ; theta_L ]; % input matrix

                                %     |  X_dot  | -> velocity in x
    K = transMatrix * inMat;    % K = |  Y_dot  | -> velocity in y
                                %     | PSI_dot | -> angular velocity
                                
    %----------------------------------------------------------------------                           
    % Remove previous instance
    %----------------------------------------------------------------------
    delete(C1);
    % delete(C2);
    % delete(C3);
    % delete(C4);
    
    %----------------------------------------------------------------------
    % Update robot's position
    %----------------------------------------------------------------------
    Bx1 = Bx1+K(1,1); Bx2 = Bx2+K(1,1); By1 = By1+K(2,1); By2 = By2+K(2,1);
    % Lx1 = Lx1+K(1,1); Lx2 = Lx2+K(1,1); Ly1 = Ly1+K(2,1); Ly2 = Ly2+K(2,1);
    % Rx1 = Rx1+K(1,1); Rx2 = Rx2+K(1,1); Ry1 = Ry1+K(2,1); Ry2 = Ry2+K(2,1);
    % Px1 = Px1+K(1,1); Px2 = Px2+K(1,1); Py1 = Py1+K(2,1); Py2 = Py2+K(2,1);
    
    %----------------------------------------------------------------------
    % Draw Robot's new position
    %----------------------------------------------------------------------
    C1 = line([Bx1-cos(heading) Bx2+cos(heading)],[By1-sin(heading) By2+sin(heading)], 'Linewidth',5); % Base
    % C2 = line([Lx1+sin(heading) Lx2-sin(heading)],[Ly1+cos(heading) Ly2]-cos(heading)); % Left Wheel
    % C3 = line([Rx1 Rx2],[Ry1 Ry2]); % Right Wheel
    % C4 = line([Px1-cos(heading+pi/2) Px2+cos(heading+pi/2)],[Py1-sin(heading+pi/2) Py2+sin(heading+pi/2)]); % Pointer
    
    pause(t_step); % Simulate time step
    
    heading = heading + K(3,1); % Update heading
    
end % End of for loop
                                 
end % End of function
