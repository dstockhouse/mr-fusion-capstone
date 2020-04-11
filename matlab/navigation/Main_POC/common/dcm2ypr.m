function [yaw, pitch, roll] = dcm2ypr(C)
% Function Description:
%   Converts a relative-axis roll, pitch, then yaw rotation sequence to a 
%   3X3 DCM matrix C = Rz(yaw) Ry(pitch) Rx(roll)
%
% INPUTS:
%   C = 3x3 direction cosine matrix (i.e., rotation matrix)
%   
% OUTPUTS:
%   yaw   = Initial rotation about the common z-axis (rad)
%   pitch = rotation about the new y-axis (rad)
%   roll  = rotation about the new x-axis (rad)
%
% NOTES:
%   - Using MATLAB defined eps = 2.2204e-016 as ~ zero
    
roll  = atan2( C(3,2),C(3,3)); % (rad)
pitch = atan2(-C(3,1),sqrt(C(2,1)^2+C(1,1)^2)); % (rad)
yaw   = atan2( C(2,1),C(1,1)); % (rad)

end % Close function