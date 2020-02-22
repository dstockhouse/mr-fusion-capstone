%--------------------------------------------------------------------------
% Name: ControllerTest
% Desc: Tests the controller scheme by utilizing the ControlSim,
%       KinematicSim, and PlotSim functions. Mock Guidance information is
%       sent to the controller which then sends the required motor
%       velcocities to the plant. The resulting change is then recorded and
%       graphed. The robot's position is remembered between function calls
%       during a single run of this script.
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/22/2020
%--------------------------------------------------------------------------

close; clc; clear KinematicSim; clear ControlSim; clear PlotSim

%--------------------------------------------------------------------------
% Determine whether or not to save each scenario as an animation
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
% First Scenario
%--------------------------------------------------------------------------
scenario = 'First Scenario';

% Create animation capturing the simulation
if save_movie
    v = VideoWriter('Scenario_1.mp4', 'MPEG-4');
    v.FrameRate = 1/time_step;
    open(v);
    vfig = figure(1);
end

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Keep moving straight
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    else
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 1);
    
    % Save captured animation
    if save_movie
        frame = getframe(vfig);
        writeVideo(v, frame);
    end
    
end % end of for() loop

% Close video writer at the end of simulation
if save_movie
    close(v);
end

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

%--------------------------------------------------------------------------
% Second Scenario
%--------------------------------------------------------------------------
scenario = 'Second Scenario';

% Create animation capturing the simulation
if save_movie
    v = VideoWriter('Scenario_2.mp4', 'MPEG-4');
    v.FrameRate = 1/time_step;
    open(v);
    vfig = figure(1);
end

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Move 30 degrees to the right
        heading_des = -pi/6; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    else
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 2);
    
    % Save captured animation
    if save_movie
        frame = getframe(vfig);
        writeVideo(v, frame);
    end
    
end % end of for() loop

% Close video writer at the end of simulation
if save_movie
    close(v);
end

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

%--------------------------------------------------------------------------
% Third Scenario
%--------------------------------------------------------------------------
scenario = 'Third Scenario';

% Create animation capturing the simulation
if save_movie
    v = VideoWriter('Scenario_3.mp4', 'MPEG-4');
    v.FrameRate = 1/time_step;
    open(v);
    vfig = figure(1);
end

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Move 45 degrees to the right
        heading_des = -pi/4; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    else
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 3);
    
    % Save captured animation
    if save_movie
        frame = getframe(vfig);
        writeVideo(v, frame);
    end
    
end % end of for() loop

% Close video writer at the end of simulation
if save_movie
    close(v);
end

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

%--------------------------------------------------------------------------
% Fourth Scenario
%--------------------------------------------------------------------------
scenario = 'Fourth Scenario';

% Create animation capturing the simulation
if save_movie
    v = VideoWriter('Scenario_4.mp4', 'MPEG-4');
    v.FrameRate = 1/time_step;
    open(v);
    vfig = figure(1);
end

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Move 60 degrees to the right
        heading_des = -pi/3; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    else
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 4);
    
    % Save captured animation
    if save_movie
        frame = getframe(vfig);
        writeVideo(v, frame);
    end
    
end % end of for() loop

% Close video writer at the end of simulation
if save_movie
    close(v);
end

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)

%--------------------------------------------------------------------------
% Fifth Scenario
%--------------------------------------------------------------------------
scenario = 'Fifth Scenario';

% Create animation capturing the simulation
if save_movie
    v = VideoWriter('Scenario_5.mp4', 'MPEG-4');
    v.FrameRate = 1/time_step;
    open(v);
    vfig = figure(1);
end

heading_act = 0; % Set actual heading of the robot before simulation

for i = 0:time_step:5
    
    if (i>1) % Move 90 degrees to the right
        heading_des = -pi/2; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    else
        
        heading_des = 0; % Desired heading given by Guidance
        delta_heading = heading_des - heading_act; % Heading error
    end
    
    % Determine angular wheel velocities based on heading
    [theta_L, theta_R] = ControlSim(delta_heading);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step);
    
    % Plot kinematics
    PlotSim(K, heading_act, time_step, scenario, i, 5);
    
    % Save captured animation
    if save_movie
        frame = getframe(vfig);
        writeVideo(v, frame);
    end
    
end % end of for() loop

% Close video writer at the end of simulation
if save_movie
    close(v);
end

clear KinematicSim; clear ControlSim; clear PlotSim;
pause(1)
