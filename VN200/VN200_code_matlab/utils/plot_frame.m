function hg = plot_frame(C, org, label, colorx, colory, colorz)

% Description:
%   Function that plots  a rotation matrix
%
% Inputs:
%   C:          a 3X3 rotation matrix
%   label:      an axis label string
%   colorx,y,z: a choice of colors for the x, y, and z axes
%   org:        The location of the origin of the coordinate frame
%
% Outputs:
%   None
%
% Comments:
% 1. You may want to open a figure before calling this function
% 2. You may want to set the "size" of the 3D plot (i.e. xlim, ylim, and zlim
%    before or after calling this function

labloc = 1.1;       % Scale factor to locate the axis labels

x_axis = C(:,1);    % Extract the x-axis from the DCM
y_axis = C(:,2);    % Extract the y-axis from the DCM
z_axis = C(:,3);    % Extract the z-axis from the DCM

%
%   Now plot and label the three axes
%
hg = hggroup;           % Create a group object and the handle to that parent
% Plot a line for the x-axis
line([org(1), x_axis(1)+org(1)], [org(2), x_axis(2)+org(2)], [org(3), x_axis(3)+org(3)], 'Color', colorx,'Parent',hg, 'LineWidth', 3);
text(labloc*x_axis(1)+org(1), labloc*x_axis(2)+org(2), labloc*x_axis(3)+org(3), ['X_' label] , 'Color', colorx,'Parent',hg, 'FontSize', 14);

% Plot a line for the y-axis
line([org(1), y_axis(1)+org(1)], [org(2), y_axis(2)+org(2)], [org(3), y_axis(3)+org(3)], 'Color', colory,'Parent',hg, 'LineWidth', 3);
text(labloc*y_axis(1)+org(1), labloc*y_axis(2)+org(2), labloc*y_axis(3)+org(3), ['Y_' label] , 'Color', colory,'Parent',hg, 'FontSize', 14);

% Plot a line for the z-axis
line([org(1), z_axis(1)+org(1)], [org(2), z_axis(2)+org(2)], [org(3), z_axis(3)+org(3)], 'Color', colorz,'Parent',hg, 'LineWidth', 3);
text(labloc*z_axis(1)+org(1), labloc*z_axis(2)+org(2), labloc*z_axis(3)+org(3), ['Z_' label] , 'Color', colorz,'Parent',hg, 'FontSize', 14);

% Label the origin {?}
%text(-0.2*z_axis(1)+org(1), -0.2*z_axis(2)+org(2), -0.2*z_axis(3)+org(3), ['\{', label, '\}'] , 'Color', colorx,'Parent',hg, 'FontSize', 14);

end % Close the Function