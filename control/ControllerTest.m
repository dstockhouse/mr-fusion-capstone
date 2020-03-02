%--------------------------------------------------------------------------
% Name: ControllerTest
% Desc: Tests the controller scheme by utilizing the ControlSim,
%       KinematicSim, and PlotSim functions. Mock Guidance information is
%       sent to the controller which then sends the required motor
%       velcocities to the plant. The resulting change is then recorded and
%       graphed. The robot's position is remembered between function calls
%       during a single run of this script.
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 3/01/2020
%--------------------------------------------------------------------------

close; clc; clear KinematicSim; clear ControlSim; clear PlotSim

%--------------------------------------------------------------------------
% Determine whether or not to save as an animation
%--------------------------------------------------------------------------
save_movie = false;

%--------------------------------------------------------------------------
% Define robot characteristics
%--------------------------------------------------------------------------
r = .5524;     % radius of the wheels
R = .1524;     % radius of the axle
C = pi*2*R;    % assumed circumference of the robot when spinning in place
Vsys = 1;      % linear velocity of overall system

%--------------------------------------------------------------------------
% Declare timestep
%--------------------------------------------------------------------------
time_step = 0.01;

%--------------------------------------------------------------------------
% Time Calculation for Spinning on the spot
%--------------------------------------------------------------------------
half_turn = (pi*C)/(2*pi*r*Vsys);      % time to run the robot to spin 180
quarter_turn = (pi/2*C)/(2*pi*r*Vsys); % time to run the robot to spin 90   

%--------------------------------------------------------------------------
% Declare movie object for animation
%--------------------------------------------------------------------------
if save_movie
    v = VideoWriter('ControlSim.mp4', 'MPEG-4');
    v.FrameRate = 1/time_step;
    open(v);
    vfig = figure(1);
end

%--------------------------------------------------------------------------
% First Scenario
%--------------------------------------------------------------------------
scenario = 'Straight Forward';

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Keep moving straight
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    else
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading, speed_des);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 1);
    
end % end of for() loop

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

%--------------------------------------------------------------------------
% Second Scenario
%--------------------------------------------------------------------------
scenario = '30 Deg Turn';

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Turn 30 degrees to the right
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = -pi/6; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    else
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading, speed_des);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 2);
    
end % end of for() loop

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

%--------------------------------------------------------------------------
% Third Scenario
%--------------------------------------------------------------------------
scenario = '60 Deg Turn';

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Turn 60 degrees to the right
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = -pi/3; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    else
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading, speed_des);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 4);
    
end % end of for() loop

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

%--------------------------------------------------------------------------
% Fifth Scenario
%--------------------------------------------------------------------------
scenario = '90 Deg Turn';

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Turn 90 degrees to the right
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = -pi/2; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    else
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading, speed_des);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 5);
    
end % end of for() loop

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

%--------------------------------------------------------------------------
% Sixth Scenario
%--------------------------------------------------------------------------
scenario = '120 Deg Turn';

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Turn 120 degrees to the right
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = -2*pi/3; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    else
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading, speed_des);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 6);
    
end % end of for() loop

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

%--------------------------------------------------------------------------
% Seventh Scenario
%--------------------------------------------------------------------------
scenario = 'U Turn';

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Turn 180 degrees to the right
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = -pi; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    else
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading, speed_des);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 7);
    
end % end of for() loop

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

%--------------------------------------------------------------------------
% Eighth Scenario
%--------------------------------------------------------------------------
scenario = 'Spot Spin';

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if ((i>1) && (i<=(1+half_turn))) % Turn on the spot
        speed_des = 0; % Desired speed given by guidance (1 or 0)
        heading_des = -pi; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    elseif i > (1+half_turn)
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = -pi; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
        
    else
        speed_des = 1; % Desired speed given by guidance (1 or 0)
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading, speed_des);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 8);
    
end % end of for() loop

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

% Save captured animation
if save_movie
    frame = getframe(vfig);
    writeVideo(v, frame);
end

% Close video writer at the end of simulation
if save_movie
    close(v);
end
