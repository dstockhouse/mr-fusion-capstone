function [gps_TAN_xyz] = transform_GPS_ECEF_to_TAN(vn200_gps_ECEF_xyz, constants);

    addpath("./common")     % Common code from Hwks and project folder

    % Define Tan Frame Origin
    % GPS Locked @ ~1 min
    % From t_GPS - 60 sec = index 301
    r_e__e_t = vn200_gps_ECEF_xyz(:,301);
    
    % Calculate the difference vector
    r_e__t_b = vn200_gps_ECEF_xyz - r_e__e_t;
    
    % Calculate Rotation Matrix from ECEF to Tan Frame using Lat, Lon, H of
    % Tan Origin
    [Lat_tan, Lon_tan, h_t] = xyz2llh(r_e__e_t, constants);
    C_e__n = Lat_Lon_2C_e__n(Lat_tan,Lon_tan);
    C_t__e = inv(C_e__n);
    
    % Transform difference vector into Tan Frame and set outputs
    gps_TAN_xyz = mtimes(C_t__e, r_e__t_b);
    
    % Set constant values in the Constants Struct
    constants.C_e__t = C_e__n;
    constants.r_e__e_t = r_e__e_t;
end