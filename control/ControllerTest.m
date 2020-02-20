%--------------------------------------------------------------------------
% Name: ControllerTest
% Desc: Tests the KinematicSim and ControlSim function using a variety of
%       scenarios. The robot's position is remembered between function
%       calls during a single run of this script.
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/20/2020
%--------------------------------------------------------------------------

close; clc; clear KinematicSim; clear ControlSim;

%--------------------------------------------------------------------------
% Define robot characteristics
%--------------------------------------------------------------------------
r = .5524;          % radius of the wheels
R = .1524;          % radius of the axle
C = pi*2*R;         % assumed circumference of the robot when spinning
Vsys = 1;           % linear velocity of overall system

%--------------------------------------------------------------------------
% Declare timestep
%--------------------------------------------------------------------------
time_step = 0.01;

%--------------------------------------------------------------------------
% Controller Constant
%--------------------------------------------------------------------------
Kp = 1/3;

%--------------------------------------------------------------------------
% Time Calculation for Spinning on the spot
%--------------------------------------------------------------------------
half_turn = (pi*C)/(2*pi*r*Vsys);      % time to run the robot to spin 180
quarter_turn = (pi/2*C)/(2*pi*r*Vsys); % time to run the robot to spin 90   

%--------------------------------------------------------------------------
% First Scenario
%--------------------------------------------------------------------------
scenario = 'First Scenario';

for i = 0:time_step:7*Kp
    
    if (i <= 3*Kp) % Move straight for 3 seconds
        
        % Move striaght at 2 meters per second
        theta_L = 2;
        theta_R = 2;
        
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
    
    elseif (i>3*Kp) && (i<=4*Kp) % Turn right 90 degrees in 1 second
        
        % Determine angular wheel velocities from ControlSim()
        [theta_L, theta_R] = ControlSim(pi/2);
    
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    elseif (i>4*Kp) && (i<5*Kp) % Turn left 90 degrees for 1 second
        
        % Determine angular wheel velocities from ControlSim()
        [theta_L, theta_R] = ControlSim(-pi/2);
    
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    else
        
        % Move striaght at 5 meters per second
        theta_L = 5;
        theta_R = 5;
        
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    end % end of if() statement
    
end % end of for() loop

pause(1);
clear KinematicSim; clear ControlSim;

%--------------------------------------------------------------------------
% Second Scenario
%--------------------------------------------------------------------------
scenario = 'Second Scenario';

for i = 0:time_step:10*Kp
    
    if (i <= 2*Kp)
        
        % Reverse at a rate of 2 m/s
        theta_R = -2;
        theta_L = -2;
        
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    elseif (i>3*Kp) && (i<=4*Kp)
        
        % Determine angular wheel velocities from ControlSim()
        [theta_L, theta_R] = ControlSim(-pi/2);
    
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    else
        
        % Move straight at a rate of 2 m/s
        theta_R = 2;
        theta_L = 2;
        
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    end % end of if() statement
        
end
        
pause(1);
clear KinematicSim; clear ControlSim;

%--------------------------------------------------------------------------
% Third Scenario
%--------------------------------------------------------------------------
scenario = 'Third Scenario';

for i = 0:time_step:7*Kp
    
    if (i <= half_turn)
        
        % Reverse at a rate of 2 rad/s
        theta_R = -1;
        theta_L = 1;
        
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    elseif (i>3*Kp) && (i<=4*Kp)
        
        % Determine angular wheel velocities from ControlSim()
        [theta_L, theta_R] = ControlSim(pi/2);
    
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    else
        
        % Move straight at a rate of 2 rad/s
        theta_R = 2;
        theta_L = 2;
        
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    end % end of if() statement
        
end

pause(1);
clear KinematicSim; clear ControlSim;

%--------------------------------------------------------------------------
% Fourth Scenario
%--------------------------------------------------------------------------
scenario = 'Fourth Scenario';

t = 8+5*quarter_turn;

for i = 0:time_step:t
    
    if (i <= quarter_turn)
        
        % Reverse at a rate of 2 rad/s
        theta_R = -1;
        theta_L = 1;
        
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    elseif (i > (quarter_turn+1)) && (i <= (1+quarter_turn*2))   || ...
           (i > (3+quarter_turn*2)) && (i <= (3+quarter_turn*3)) || ...
           (i > (5+quarter_turn*3)) && (i <= (5+quarter_turn*4)) || ...
           (i > (7+quarter_turn*4)) && (i <= (7+quarter_turn*5))
           
        % Reverse at a rate of 2 rad/s
        theta_R = 1;
        theta_L = -1;
        
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    else
        
        % Move straight at a rate of 2 rad/s
        theta_R = 2;
        theta_L = 2;
        
        % Feed angular velocities into kinematics using KinematicSim()
        KinematicSim(theta_L, theta_R, time_step, scenario);
        
    end % end of if() statement
        
end  
