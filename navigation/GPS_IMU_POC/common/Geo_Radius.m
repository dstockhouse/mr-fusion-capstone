function [r_e__e_S] = Geo_Radius(L_b, constants)
%
% FUNCTION DESCRIPTION:
%   Compute the Geocentric radius at the surface of the earth
%
% INPUTS:
%   L_b		 = Geodetic body latitude (radians)
%   constants= A structure containing many constant parameters
%
% OUTPUTS:
%   r_e__e_S =  Geocentric radius (m)
%
% REFERENCE:
%  Navigation Systems: P. Groves 2e 
%       Page 71 Eqn. 2.137

    e  = constants.e;           % Eccentricity

% Geocentric radius (m)
    r_e__e_S = RE(L_b, constants) * sqrt(cos(L_b)^2 +(1 - e^2)^2 * sin(L_b)^2);
end