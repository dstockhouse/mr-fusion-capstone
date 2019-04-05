function [] = stop_VN200(s)
% Function Description:
%   Terminates the VN200 IMU data transmission
%
% INPUTS:
%   s = Searial port object
%   
% OUTPUTS:
%   None
%
% NOTES:

serial_cmd(s, 'VNWRG,6,0');     % Disable asynchronous data output
pause(0.25);                    % "small" delay
flushinput(s);                  % Clear input buffers
fclose('all');                  % Close all open files and Ports
delete(instrfindall);           % Close all open Ports

end         % end function

