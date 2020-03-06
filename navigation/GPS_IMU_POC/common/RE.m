function [R_E] = RE(L_b, constants)
%
% FUNCTION DESCRIPTION:
%   Compute the Transverse radius of curvature
%
% INPUTS:
%   L_b		 = Geodetic body latitude (radians)
%   constants= A structure containing many constant parameters
%
% OUTPUTS:
%   RE		 =  Transverse radius of curvature (m)
%
% REFERENCE:
%  Navigation Systems: P. Groves 2e 
%       Page 59 Eqn. 2.106

    e  = constants.e;           % Eccentricity
    R0 = constants.R0;          % Earth's equatorial radius (meters)

% Transverse radius of curvature (m)
    R_E = R0 / sqrt(1 - e^2*(sin(L_b))^2); 
end