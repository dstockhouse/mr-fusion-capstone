%--------------------------------------------------------------------------
% Name: ControllerTest
% Desc: Tests the KinematicSim and ControlSim function using a variety of
%       scenarios. The robot's position is remembered between function
%       calls during a single run of this script.
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/21/2020
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
time_step = 0.05;

%--------------------------------------------------------------------------
% Time Constant
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

heading_act = 0;

for i = 0:time_step:14
    
    if (i>3) && (i<=6) % Turn right 90 degrees in 1 second
        
        heading_des = pi/2;
        delta_heading = heading_des - heading_act;
        
    elseif (i>6) && (i<12) % Turn left 90 degrees for 1 second
        
        heading_des = -pi/2;
        delta_heading = heading_des - heading_act;
    
    else % Move straight
       
        heading_des = 0;
        delta_heading = heading_des - heading_act;
        
    end % end of if() statement
    
    fprintf('%f\n', i)
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(delta_heading);
        
    % Feed angular velocities into kinematics using KinematicSim()
    [K, heading_act] = KinematicSim(theta_L, theta_R, time_step, scenario);
    
end % end of for() loop

