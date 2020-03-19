function plotty = PlotSim(K, heading, time_step, scenario, time, direction)
%--------------------------------------------------------------------------
% Name: PlotSim
% Desc: Plots movement of the robot given its kinematics
% Inputs: K - matrix containing the kinematics of the system
%         time_step - the desired amount of time passed between each
%                     iteration of the function call
%         scenario - a string to serve as the graph title
%         time - the time in seconds that the simulation has been running
%         direction - a flag variable used to determine which line to draw
%                     for the ideal path.
% Outputs: N/A
% Author: Connor Rockwell, Joy Fucella, Duncan Patel
% Last Modified: 3/01/2020
%--------------------------------------------------------------------------

%--------------------------------------------------------------------------                           
% Remove previous figure instance
%--------------------------------------------------------------------------
clf('reset')

%--------------------------------------------------------------------------                           
% Create increment value to traverse ideal path array
%--------------------------------------------------------------------------
persistent increment;
if isempty(increment)
    increment = 1;
end

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
        x = zeros(1,6/time_step);
        y = -1+time_step:time_step:5;
        
    case 2
        line([0 10*sin(pi/6)],[0 10*cos(pi/6)], 'LineStyle','--');
        x1 = zeros(1,1/time_step);
        x2 = 0:time_step*0.5:5;
        x = cat(2,x1,x2);
        y1 = -1+time_step:time_step:0;
        y2 = tan(pi/3)*x2;
        y = cat(2,y1,y2);
        
    case 3
        line([0 10*sin(pi/4)],[0 10*cos(pi/4)], 'LineStyle','--');
        x1 = zeros(1,1/time_step);
        x2 = 0:0.7*time_step:5;
        x = cat(2,x1,x2);
        y1 = -1+time_step:time_step:0;
        y2 = tan(pi/4)*x2;
        y = cat(2,y1,y2);
        
    case 4
        line([0 10*sin(pi/3)],[0 10*cos(pi/3)], 'LineStyle','--');
        x1 = zeros(1,1/time_step);
        x2 = 0:time_step*0.85:5;
        x = cat(2,x1,x2);
        y1 = -1+time_step:time_step:0;
        y2 = tan(pi/6)*x2;
        y = cat(2,y1,y2);
        
    case 5
        line([0 10*sin(pi/2)],[0 10*cos(pi/2)], 'LineStyle','--');
        x1 = zeros(1,1/time_step);
        x2 = 0:time_step*0.95:5;
        x = cat(2,x1,x2);
        y1 = -1+time_step:time_step:0;
        y2 = zeros(1,5/time_step);
        y = cat(2,y1,y2);
        
    case 6
        line([0 10*sin(2*pi/3)],[0 10*cos(2*pi/3)], 'LineStyle','--');
        x1 = zeros(1,1/time_step);
        x2 = 0:time_step*0.8:5;
        x = cat(2,x1,x2);
        y1 = -1+time_step:time_step:0;
        y2 = tan(-pi/6)*x2;
        y = cat(2,y1,y2);
        
    case 7
        line([0 10*sin(pi)],[0 10*cos(pi)], 'LineStyle','--');
        x = zeros(1,6/time_step);
        y1 = -1+time_step:time_step:0;
        y2 = 0-time_step:-time_step*0.85:-5;
        y = cat(2,y1,y2);
        
    otherwise
        line([0 10*sin(pi)],[0 10*cos(pi)], 'LineStyle','--');
        x = zeros(1,6/time_step);
        y1 = -1+time_step:time_step:0;
        y2 = zeros(1,(1/time_step-12));
        y3 = 0-time_step:-time_step:-5;
        y = cat(2,y1,y2,y3);
        
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
    
%--------------------------------------------------------------------------
% Print simulation info in figure title
%--------------------------------------------------------------------------
dist = sqrt((K(1,1)-x(increment))^2 + (K(2,1)-y(increment))^2);

str1 = ['Current Heading: ' num2str(-rad2deg(heading), '%.2f')];
str2 = ['Distance Error: ' num2str(dist, '%.2f')];
str = {str1,str2};
text(-3.75,3,str,'FontSize',10);

% Line between the center of the robot and the path it's following
% line([K(1,1) x(increment)],[K(2,1) y(increment)], 'Color','red');

%--------------------------------------------------------------------------
% Pause and increment
%--------------------------------------------------------------------------
increment = increment + 1;

pause(time_step); % simulate time step
    
end
