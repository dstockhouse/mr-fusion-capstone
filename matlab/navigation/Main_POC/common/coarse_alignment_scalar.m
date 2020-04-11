function [roll, pitch, yaw, C_n__b] = coarse_alignment_scalar(gyro, accel) 
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

fx = accel(1);
fy = accel(2);
fz = accel(3);

wx = gyro(1);
wy = gyro(2);
wz = gyro(3);

roll  = atan2(-fy, -fz);                % phi (rad)
pitch =  atan2(fx, sqrt(fy^2 + fz^2));  % theta (rad)
yaw = atan2(-wy*cos(roll)+wz*sin(roll), wx*cos(pitch)+sin(pitch)*(wy*sin(roll)+wz*cos(roll)));   % psi (rad)

C_n__b = rotate_z(yaw) * rotate_y(pitch) * rotate_x(roll);
end

