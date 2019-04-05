function [compass, accel, gyro, temp, baro] = read_VN200_IMU(s)
% Function Description:
%   Reads the VN200 IMU data from the searial port
%
% INPUTS:
%   s = Searial port object
%   
% OUTPUTS:
%   Compass, Accel, Gyro, Temp, and Baro data
%
% NOTES:
%   This fn monitors the Number of Bytes available in the serial Port's
%   512 Byte buffer.  It the Number is > 400 you are trying to sample too
%   fast and need to lower Fs.

% if s.BytesAvailable > 400 
%     error(' Your sample rate is too high - Please lower Fs !!');
% end

resp = fgets(s);            % Read the IMU data from the serial port as a string

% Example of the IMU data string:
% $VNIMU,-00.0350,+00.4222,-00.6643,-00.015,+00.019,+09.948,+00.002066,-00.008442,-00.044656,+20.7,+084.322*66
%       ,   Magnetomrter data      ,     Accel data        ,           Gyro       data      , Temp , baro
% The last three bytes are checksum

imu_data = sscanf(resp(8:end-3), '%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f');

compass= imu_data(1:3)';    % X, Y, Z Compass data (Gauss)
accel  = imu_data(4:6)';    % X, Y, Z Accelerometer data (m/s^2)
gyro   = imu_data(7:9)';    % X, Y, Z Gyroscope data (rad/s)
temp   = imu_data(10)';     % IMU temperature data (deg C)
baro   = imu_data(11)';     % Brometric pressure data (kPa)

end         % End of function read_VN200

