function [s, SN] = initialize_VN200_IMU( Fs )
% Function Description:
%   Initializes the VN200 IMU and configures the serial port to run from
%   the VN200's internal clock (async mode) at a freq of Fs Hz.
%
% INPUTS:
%   Fs = VN200 sample rate (Hz)  Must be < 200
%   
% OUTPUTS:
%   s = Serial port object
%   SN = Device serial number
%
% NOTES:
%   The default BaudRate of the VN200 is 115200.
%   This limits the sample rate to about 100 Hz

delete(instrfindall);           % Close all open Ports

%% Connect to the VN200 IMU
BaudRate = 115200;              % BaudRate: 115200, 128000, 230400, 460800, or 921600
serialInfo = instrhwinfo('serial');

% Determine available COM ports
comPort = serialInfo.AvailableSerialPorts;  % This may get assigned differently at each computer

% Create a serial port object
s = serial(comPort(end), 'BaudRate', BaudRate);     
%s.InputBufferSize = 2^10;       % Set the input Buffer to 1024 (default is 512)
fopen(s);                       % Open the serial port

% Send configuration commands to the VN200
serial_cmd(s, 'VNRRG,03');      % Request the IMU's serial number
SN = fgets(s);                  % Save the serial number
SN = SN(11:end-5);              
serial_cmd(s, 'VNWRG,6,0');     % Disable asynchronous data output
serial_cmd(s, ['VNWRG,7,',num2str(Fs)]);    % Set the asynchronous data output freq to Fs Hz
pause(0.1);                     % A "small" delay
flushinput(s);                  % Clear input buffers
pause(0.1);                     % A "small" delay
serial_cmd(s, 'VNWRG,6,19');    % Enable async IMU Measurements on VN200
fgets(s);                       % Read from COMM port - Should echo command
fgets(s);                       % Read from COMM port - Should be valid IMU string (example below)
% $VNIMU,-00.0350,+00.4222,-00.6643,-00.015,+00.019,+09.948,+00.002066,-00.008442,-00.044656,+20.7,+084.322*66
%          Magnetomrter data           
end

