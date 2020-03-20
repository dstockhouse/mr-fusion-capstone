function [L_b, lambda_b, h_b] = xyz2llh(r_e__e_b, constants)
%
% FUNCTION DESCRIPTION:
%   Converts ECEF rectangular coordinates to geodetic curvilinear coordinates
%
% INPUTS:
%   r_e__e_b = 3-tupple vector describing ECEF coordinates of the body frame
%			   origin wrt the e-frame origin coordinatized in the e-frame (meters)
%   constants= A structure containing many constant parameters
%
% OUTPUTS:
%   L_b		 = Geodetic body latitude (radians)
%	lambda_b = Geodetic body longitude (radians)
%	h_b		 = Geodetic height (meters)
%
%
% NOTES:
%   - None
%
% REFERENCE:
%   "Aided Navigatin" by Farrel Eqn 2.71 and page 34
%   
% REVISION HISTORY:
%
%global constants;           % Global variables

% Pull out the scalar components of position
x = r_e__e_b(1);        % (meters)
y = r_e__e_b(2);
z = r_e__e_b(3);

e  = constants.e;           % Eccentricity
R0 = constants.R0;          % Equatorial radius (meters)

lambda_b = atan2(y, x); % Longitude (rad)

rr = sqrt(x^2 + y^2);	% Length on the Equatorial plane (meters)

h_b = 0;    % Initialize the height to 0 (m)
RE = R0;    % Initialize the transverse radius of curvature (m)

for i=1:10   % A fixed number of iterations - A greater number gives better precision
    sin_L_b = z / ((1-e^2)*RE + h_b);
   %L_b     = asin(sin_L_b);                % Latitude (rad) - A first algorithm
    L_b     = atan((z+e*e*RE*sin_L_b) / rr);% Latitude (rad) - A better algorithm
    RE      = R0 / sqrt(1-e^2*sin_L_b^2);	% Transverse radius of curvature (m)
    h_b     = rr / cos(L_b) - RE;           % Height (meters)
end
end