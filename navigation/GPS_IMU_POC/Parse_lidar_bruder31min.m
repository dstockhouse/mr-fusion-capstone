%% File: Parse_lidar_bruder_31min.m
% Author :Emmett Hamman
% EE 421: Mr. Fusion Capstone
% Description: Script will parse .csv file of 31 min VN200 log file and
%              save the Accel, Gyro, and GPS data
%              NOTE: Does not save the checksum from each sample

clear;
close all;
clc;

% Preallocate Space
vn200_data = zeros(41822, 14);
vn200_txt_tag = zeros(41822, 1);
vn200_accel = zeros(38020, 3);
vn200_gyro = zeros(38020, 3);
vn200_gps_ECEF = zeros(3802, 3);
% vn200_baro = zeros(38020, 1);
% vn200_mag_compass = zeros(38020, 3);

%% Read Data in from File, store to temp variables

% Change i to read in different amounts of data!!
for i = 0:20
   file_name = strcat('.\lidar_bruder_31min.log.csv');
   [vn200_data, vn200_txt_tag] = xlsread(file_name, 1, 'A60000:O60020');
end

%% Parse through data in temp variables
vn200_accel = zeros(1,3);
vn200_gyro = zeros(1,3);
vn200_gps_ECEF = zeros(1,3);

for i = 1:length(vn200_txt_tag)
    if vn200_txt_tag(i,1) == "$VNIMU"
        if size(vn200_accel,1) == 1 && vn200_accel(1,1) == 0;
          vn200_accel(1, :) = vn200_data(i, [4,5,6]);
          vn200_gyro(1, :) = vn200_data(i, [7,8,9]); 
        else 
            vn200_accel(size(vn200_accel,1)+1, :) = vn200_data(i, [4,5,6]);
            vn200_gyro(size(vn200_gyro,1)+1, :) = vn200_data(i, [7,8,9]);
        end
    elseif vn200_txt_tag(i,1) == "$VNGPE"
        if size(vn200_gps_ECEF, 1) == 1 && vn200_gps_ECEF(1,1) == 0;
           vn200_gps_ECEF(1, :) = vn200_data(i, [5,6,7]);
        else
            vn200_gps_ECEF(size(vn200_gps_ECEF,1)+1, :) = vn200_data(i, [5,6,7]);
        end
    end
end



