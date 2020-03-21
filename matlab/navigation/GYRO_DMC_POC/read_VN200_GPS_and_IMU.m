function [vn200_data] = read_VN200_GPS_and_IMU(s)
% Function Description:
%   Reads the VN200 IMU data from the searial port
%
% INPUTS:
%   s = Searial port object
%   
% OUTPUTS:
%   Time, Number of Sats Used, latitude, longitude, and height

resp = fgets(s);            % Read the GPS data from the serial port as a string
% Example of the IMU data string:
% $VNIMU,-00.0350,+00.4222,-00.6643,-00.015,+00.019,+09.948,+00.002066,-00.008442,-00.044656,+20.7,+084.322*66
%       ,   Magnetomrter data      ,     Accel data        ,           Gyro       data      , Temp , baro
% The last three bytes are checksum

% Example of the GPS data string: VNGPE
% $VNGPE,341133.000288,2046,3,07,-2007261.070,-4857734.660,+3603694.140,+000.040,-000.110,+000.170,+013.110,+011.491,+006.837,+000.544,6.00E-09*04
% The last three bytes are checksum

% Offset Name       Format      Unit     Description
% 0     Tow         double      sec     GPS time of week.
% 8     Week        uint16      week    Current GPS week.
% 10    GpsFix      uint8   - GPS fix type. See table below.
% 11    NumSats     uint8   - Number of GPS satellites used in solution.
% 12    - - - --- 4 PADDING BYTES ---
% 16    PositionX   double      m       ECEF X coordinate.
% 24    PositionY   double      m       ECEF Y coordinate.
% 32    PositionZ   double      m       ECEF Z coordinate.
% 40    VelocityX   float       m/s     ECEF X velocity.
% 44    VelocityY   float       m/s     ECEF Y velocity.
% 48    VelocityZ   float       m/s     ECEF Z velocity.
% 52    PosAccX     float       m       ECEF X position accuracy estimate.
% 56    PosAccY     float       m       ECEF Y position accuracy estimate.
% 60    PosAccZ     float       m       ECEF Z position accuracy estimate.
% 64    SpeedAcc    float       m/s     Speed accuracy estimate.
% 68    TimeAcc     float       sec     Time accuracy estimate.

if strcmp(resp(1:6), '$VNGPE')
    idx = strfind(resp, ',');   % Find commas in the string
    sdata = zeros(1,7);         % Initialize a data holder array
    for k = 1:7
       sdata(k) = str2double(resp(idx(k)+1: idx(k+1)-1));
    end

    vn200_data.t       = sdata(1);          % GPS time of week in seconds.
    vn200_data.NumSats = sdata(4);          % Number of GPS satellites used in solution
    x                  = sdata(5);          % ECEF X coordinate (m)       
    y                  = sdata(6);          % ECEF y coordinate (m) 
    z                  = sdata(7);          % ECEF z coordinate (m)    
    [lat, lon, hb] = xyz2llh([x, y, z]);
    vn200_data.lat     = lat*180/pi;        % Latitude in deg
    vn200_data.lon     = lon*180/pi;        % East Longitude in deg
    vn200_data.hb      = hb;                % Altitude/Height in m
    
else        % If IMU
    imu_data = sscanf(resp(8:end-3), '%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f');

    vn200_data.compass  = imu_data(1:3)';   % X, Y, Z Compass data (Gauss)
    vn200_data.accel    = imu_data(4:6)';   % X, Y, Z Accelerometer data (m/s^2)
    vn200_data.gyro     = imu_data(7:9)';   % X, Y, Z Gyroscope data (rad/s)
    vn200_data.temp     = imu_data(10)';    % IMU temperature data (deg C)
    vn200_data.baro     = imu_data(11)';    % Brometric pressure data (kPa)
end

end         % End of function read_VN200_GPS

