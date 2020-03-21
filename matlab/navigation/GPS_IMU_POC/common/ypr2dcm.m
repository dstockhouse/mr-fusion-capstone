function C = ypr2dcm(yaw, pitch, roll)
% Function Description:
%   Converts a relative-axis yaw, pitch, then roll rotation sequence to a 
%   3X3 DCM matrix C = Rz(yaw) Ry(pitch) Rx(roll)
%
% INPUTS:
%   yaw   = Initial rotation about the common z-axis (rad)
%   pitch = rotation about the new y-axis (rad)
%   roll  = rotation about the new x-axis (rad)
%   
% OUTPUTS:
%   C = 3x3 direction cosine matrix (i.e., rotation matrix)
%
    
C  = rotate_z(yaw) * rotate_y(pitch) * rotate_x(roll);

end % Close function