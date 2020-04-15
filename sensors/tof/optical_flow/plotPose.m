function plotPose(kai_est)
% Inputs



theta = kai_est(1); % needs to be overall turn?

grid on
daspect([1 1 1])
xlabel('Meters');
ylabel('Meters');
title('Visual Odometry Simulation');
axis square

% Draw same robot seen in Controls Sim
line([K(1,1)-0.24*cos(theta) K(1,1)+0.24*cos(theta)], ...
     [K(2,1)-0.24*sin(theta) K(2,1)+0.24*sin(theta)], ...
     'Color','black','Linewidth',3);

line([((K(1,1)+0.24*sin(theta))+(K(1,1)-0.24*sin(theta)))/2 ...
        K(1,1)+0.24*cos(theta+pi/2)] , ...
     [((K(2,1)+0.24*cos(theta))+(K(2,1)-0.24*cos(theta)))/2 ...
        K(2,1)+0.24*sin(theta+pi/2)], 'Color','black','Linewidth',3);
    
vel = kai_est(4); % need to change to display x,y,z of velocity
    
str1 = ['Angular Velocity: ' num2str(abs(rad2deg(theta)), '%.2f')];
str2 = ['Linear Velocity: ' num2str(vel, '%.2f') ' m/s'];
str = {str1,str2};
text(-3.75,3,str,'FontSize',10);
end