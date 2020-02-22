function plotty = PlotSim(K, heading, time_step, scenario, time, direction)
%--------------------------------------------------------------------------
% Name: PlotSim
% Desc: Plots movement of the robot given its kinematics
% Inputs: K - matrix containing the kinematics of the system
%         time_step - the desired amount of time passed between each
%                     iteration of the function call
%         scenario - a string to serve as the graph title
% Outputs: N/A
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 2/22/2020
%--------------------------------------------------------------------------

%--------------------------------------------------------------------------                           
% Remove previous figure instance
%--------------------------------------------------------------------------
clf('reset')

%--------------------------------------------------------------------------
% Create figure
%--------------------------------------------------------------------------
xlim([-4 4]);
ylim([-4 4]);
grid on
daspect([1 1 1])
xlabel('Meters');
ylabel('Meters');
title([scenario ' - Time: ' num2str(time, '%.2f')]);
axis square

%--------------------------------------------------------------------------
% Create target paths
%--------------------------------------------------------------------------

line([0 0],[-4,0], 'LineStyle','--');

switch direction
    case 1
        line([0 10*sin(0)],[0 10*cos(0)], 'LineStyle','--');
    case 2
        line([0 10*sin(pi/6)],[0 10*cos(pi/6)], 'LineStyle','--');
    case 3
        line([0 10*sin(pi/4)],[0 10*cos(pi/4)], 'LineStyle','--');
    case 4
        line([0 10*sin(pi/3)],[0 10*cos(pi/3)], 'LineStyle','--');
    case 5
        line([0 10*sin(pi/2)],[0 10*cos(pi/2)], 'LineStyle','--');
end
    
%--------------------------------------------------------------------------
% Draw robot's new position
%--------------------------------------------------------------------------
% Base
line([K(1,1)-0.24*cos(heading) K(1,1)+0.24*cos(heading)], ...
     [K(2,1)-0.24*sin(heading) K(2,1)+0.24*sin(heading)], ...
     'Color','black','Linewidth',3);
% Pointer
line([((K(1,1)+0.24*sin(heading))+(K(1,1)-0.24*sin(heading)))/2 ...
        K(1,1)+0.24*cos(heading+pi/2)] , ...
     [((K(2,1)+0.24*cos(heading))+(K(2,1)-0.24*cos(heading)))/2 ...
        K(2,1)+0.24*sin(heading+pi/2)], 'Color','black','Linewidth',3);
    
pause(time_step); % simulate time step
    
end
