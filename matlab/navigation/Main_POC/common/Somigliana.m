function [g_0] = Somigliana(L_b, constants)
%
% FUNCTION DESCRIPTION:
%   Compute the "down" component of the accel due to gravity using the Somigliana model
%
% INPUTS:
%   L_b		 = Geodetic body latitude (radians)
%   constants= A structure containing many constant parameters
%
% OUTPUTS:
%   g_0		 =  "down" component of the accel due to gravity (m/s^2)

e  = constants.e;           % Eccentricity

% Somigliana model of acceleration due to gravity in m/s^2
g_0 = 9.7803253359*(1 + 0.001931853 * sin(L_b)^2) / sqrt(1 - e^2 * sin(L_b)^2);