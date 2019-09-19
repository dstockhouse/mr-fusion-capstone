function [t, NumSats, lat, lon, hb] = read_VN200_GPS(s)
% Function Description:
%   Reads the VN200 IMU data from the searial port
%
% INPUTS:
%   s = Searial port object
%   
% OUTPUTS:
%   Time, Number of Sats Used, latitude, longitude, and height

resp = fgets(s);            % Read the GPS data from the serial port as a string

% Example of the GPS data string:
% $VNGPS,342123.000168,1890,3,05,+34.61463270,-112.45087270,+01559.954,+000.450,+000.770,-001.290,+002.940,+005.374,+007.410,+001.672,2.10E-08*23
% The last three bytes are checksum

% Offset Name      Format      Unit     Description
%
% 0     Time		double		sec		GPS time of week in seconds.
% 8     Week		uint16		week    GPS week.
% 10	GpsFix		uint8		-		GPS fix type. See table below.
% 11	NumSats		uint8		-		Number of GPS satellites used in solution.
% 16	Latitude	double		deg		Latitude in degrees.
% 24	Longitude	double		deg		Longitude in degrees.
% 32	Altitude	double		m		Altitude above ellipsoid. (WGS84)
% 40	NedVelX		float		m/s		Velocity measurement in north direction.
% 44	NedVelY		float		m/s		Velocity measurement in east direction.
% 48	NedVelZ		float		m/s		Velocity measurement in down direction.
% 52	NorthAcc	float		m		North position accuracy estimate. (North)
% 56	EastAcc		float		m		East position accuracy estimate. (East)
% 60	VertAcc		float		m		Vertical position accuracy estimate. (Down)
% 64	SpeedAcc	float		m/s		Speed accuracy estimate.
% 68	TimeAcc		float		sec		Time accuracy estimate.  

idx = strfind(resp, ',');   % Find commas in the string
sdata = zeros(1,7);         % Initialize a data holder array
for k = 1:7
   sdata(k) = str2double(resp(idx(k)+1: idx(k+1)-1));
end

t       = sdata(1);         % GPS time of week in seconds.
NumSats = sdata(3);         % Number of GPS satellites used in solution
lat     = sdata(5);         % Latitude in deg
lon     = sdata(6);         % East Longitude in deg
hb      = sdata(7);         % Altitude/Height in m

end         % End of function read_VN200_GPS

