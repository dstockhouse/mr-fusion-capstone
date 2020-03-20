function [C_e__n] = Lat_Lon_2C_e__n(L_b, lambda_b)
%
% FUNCTION DESCRIPTION:
%   Computes the orientation of the n-frame wrt the e-frame
%
% INPUTS:
%   L_b		 = Geodetic body latitude (radians)
%	lambda_b = Geodetic body longitude (radians)
%
% OUTPUTS:
%   C_e__e_b = 3X3 matrix describing Nav-frame orientation wrt the ECEF frame
%
% NOTES:
%   - None
%
% REFERENCE:
%   EE 595 course notes
%

C_e__n = [ -cos(lambda_b)*sin(L_b), -sin(lambda_b) , -cos(lambda_b)*cos(L_b);
           -sin(lambda_b)*sin(L_b),  cos(lambda_b) , -sin(lambda_b)*cos(L_b);
                          cos(L_b),  0             ,               -sin(L_b)];
end