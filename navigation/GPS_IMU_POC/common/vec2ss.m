function [Ohm] = vec2ss(w)
% build skew-symmetric matrix Ohm from vector w
Ohm = [0, -w(3), w(2); w(3), 0, -w(1); -w(2), w(1), 0];
end