function [w] = ss2vec(Ohm)
% FUNCTION DESCRIPTION:
%   Extract the vector from a skew symmetric matrix: Ohm = Sk(w)
%
% INPUTS:
%   Ohm = 3X3 skew-symmetric matrix (dimless)
%
% OUTPUTS:
%   w   = 3X1 vector (dimless)
%
% NOTES:
% 
% Ohm = [  0   -w3   w2 ]
%       [  w3    0  -w1 ]
%       [ -w2   w1    0 ]
       
w = [Ohm(3,2);  % Use the positive elements
     Ohm(1,3); 
     Ohm(2,1)];
end