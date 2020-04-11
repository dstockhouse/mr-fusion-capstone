function [C_n__b] = coarse_alignment_vector(gyro, accel, Lb, g, w_ie) 
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
Sb = sin(Lb);
Cb = cos(Lb);
M = [ 0   cos(Lb)*w_ie            0
      0      0             -g*cos(Lb)*w_ie
     -g  -sin(Lb)*w_ie            0        ];
fxw = cross(accel, gyro);
 
C_n__b = ([accel', gyro', fxw'] * inv(M))';

% Alternatively: Direct approach
% fx = accel(1);
% fy = accel(2);
% fz = accel(3);
% 
% wx = gyro(1);
% wy = gyro(2);
% wz = gyro(3);
% 
%  C_n__b = 1/(g*w_ie*cos(Lb)) * ...
%      [ g*wx-fx*w_ie*Sb    g*wy-fy*w_ie*Sb   g*wz-fz*w_ie*Sb
%          fz*wy-fy*wz        fx*wz-fz*wx       fy*wx-fx*wy
%         -Cb*fx*w_ie        -Cb*fy*w_ie       -Cb*fz*w_ie];
end

