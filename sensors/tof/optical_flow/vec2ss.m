function [Ohm] = vec2ss(w)
% FUNCTION DESCRIPTION:
%   Build a skew symmetric matrix from a vector: Ohm = Sk(w)
%
% INPUTS:
%   w   = 3X1 vector (dimless)
%
% OUTPUTS:
%   Ohm = 3X3 skew-symmetric matrix (dimless)
%
% NOTES:
% 
% Ohm = [  0   -w3   w2 ]
%       [  w3    0  -w1 ]
%       [ -w2   w1    0 ]

Ohm = [    0,  -w(3),  w(2);
         w(3),     0, -w(1);
        -w(2),  w(1),    0];
end