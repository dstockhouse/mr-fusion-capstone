%--------------------------------------------------------------------------
% Name: ControllerTest
% Desc: Tests the KinematicSim and ControlSim function using a variety of
%       scenarios. The robot's position is remembered between function
%       calls during a single run of this script.
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/18/2020
%--------------------------------------------------------------------------

close; clc; clear KinematicSim; clear ControlSim

%--------------------------------------------------------------------------
% Declare timestep
%--------------------------------------------------------------------------

time_step = 0.2;

%--------------------------------------------------------------------------
% Controller Constant
%--------------------------------------------------------------------------

Kp = 1/3;

%--------------------------------------------------------------------------
% First Scenario - 90 degree left turn
%--------------------------------------------------------------------------

first_scenario = 'First Scenario: 90 Degree Left Turn';

% Turn right 90 degrees
for i = 0:1:9
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(pi/2*Kp);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, first_scenario);
    
end

% Drive straight
for i = 0:1:10
    
    theta_L = 0.5; theta_R = 0.5;
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, first_scenario);
    
end

pause(1);
clear KinematicSim; clear ControlSim

%--------------------------------------------------------------------------
% Second Scenario - Left U-turn (not a full 180)
%--------------------------------------------------------------------------

second_scenario = 'Second Scenario: Left U-turn';

% Turn left (almost) 180 degrees
for i = 0:1:9
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(-pi*Kp);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, second_scenario);
    
end

% Drive straight
for i = 0:1:10
    
    theta_L = 0.5; theta_R = 0.5;
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, second_scenario);
    
end

pause(1);
clear KinematicSim; clear ControlSim

%--------------------------------------------------------------------------
% Third Scenario - Serpentine Manuever
%--------------------------------------------------------------------------

third_scenario = 'Third Scenario - Serpentine Manuever';

% Turn right 90 degrees
for i = 0:1:9
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(pi/2*Kp);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, third_scenario);
    
end

% Turn left 90 degrees
for i = 0:1:9
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(-pi/2*Kp);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, third_scenario);
    
end

% Turn left 90 degrees
for i = 0:1:9
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(-pi/2*Kp);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, third_scenario);
    
end

% Turn right 90 degrees
for i = 0:1:9
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(pi/2*Kp);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, third_scenario);
    
end

% Drive straight
for i = 0:1:4
    
    theta_L = 0.5; theta_R = 0.5;
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, third_scenario);
    
end

pause(1);
clear KinematicSim; clear ControlSim

%--------------------------------------------------------------------------
% Third Scenario - Serpentine Manuever
%--------------------------------------------------------------------------

fourth_scenario = 'Fourth Scenario - Tighter Serpentine Manuever';

% Drive straight
for i = 0:1:2
    
    theta_L = 0.5; theta_R = 0.5;
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, fourth_scenario);
    
end

% Turn right 90 degrees
for i = 0:1:4
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(pi*Kp);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, fourth_scenario);
    
end

% Turn left 90 degrees
for i = 0:1:4
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(-pi*Kp);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, fourth_scenario);
    
end

% Turn left 90 degrees
for i = 0:1:4
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(-pi*Kp);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, fourth_scenario);
    
end

% Turn right 90 degrees
for i = 0:1:4
    
    % Determine angular wheel velocities from ControlSim()
    [theta_L, theta_R] = ControlSim(pi*Kp);
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, fourth_scenario);
    
end

% Drive straight
for i = 0:1:4
    
    theta_L = 0.5; theta_R = 0.5;
    
    % Feed angular velocities into system kinematics using KinematicSim()
    KinematicSim(theta_L, theta_R, time_step, fourth_scenario);
    
end
