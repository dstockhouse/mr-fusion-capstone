function plotty = PlotSim(K, heading, time_step, scenario)
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
    
end