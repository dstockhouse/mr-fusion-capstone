function hg = plot_frame(C, org, label, color)

% Description:
%   Function that plots  a rotation matrix
%
% Inputs:
%   C:      a 3X3 rotation matrix
%   label:  an axis label string
%   color:  a choice of color for the axes
%   org:    The location of the origin of the coordinate frame
%
% Outputs:
%   None
%
% Comments:
% 1. You may want to open a figure before calling this function
% 2. You may want to set the "size" of the 3D plot (i.e. xlim, ylim, and zlim
%    before or after calling this function

labloc = 1.1;       % Scale factor to locate the axis labels

x_axis = 10*C(:,1);    % Extract the x-axis from the DCM
y_axis = 10*C(:,2);    % Extract the y-axis from the DCM
z_axis = 10*C(:,3);    % Extract the z-axis from the DCM

%
%   Now plot and label the three axes
%
hg = hggroup;           % Create a group object and the handle to that parent

% Plot a line for the x-axis
quiver3(org(1),org(2),org(3),x_axis(1), x_axis(2), x_axis(3), 'Color', 'r','Parent',hg)
text(labloc*x_axis(1)+org(1), labloc*x_axis(2)+org(2), labloc*x_axis(3)+org(3), ['X_' label] , 'Color', 'r','Parent',hg);

% Plot a line for the y-axis
quiver3(org(1),org(2),org(3),y_axis(1), y_axis(2), y_axis(3), 'Color', 'g','Parent',hg)
text(labloc*y_axis(1)+org(1), labloc*y_axis(2)+org(2), labloc*y_axis(3)+org(3), ['Y_' label] , 'Color', 'g','Parent',hg);

% Plot a line for the z-axis
quiver3(org(1),org(2),org(3),z_axis(1), z_axis(2), z_axis(3), 'Color', 'b','Parent',hg)
text(labloc*z_axis(1)+org(1), labloc*z_axis(2)+org(2), labloc*z_axis(3)+org(3), ['Z_' label] , 'Color', 'b','Parent',hg);

% Label the origin {?}
%text(-0.2*z_axis(1)+org(1), -0.2*z_axis(2)+org(2), -0.2*z_axis(3)+org(3), ['\{', label, '\}'] , 'Color', color,'Parent',hg);

end % Close the Function