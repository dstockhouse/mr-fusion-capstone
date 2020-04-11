function gps = gps_error(sigma, Fs)
% Function Name: gps_error.m
% Description: Takes gps error source terms as inputs and outputs them 
% into a gps error source stuct
    gps.sigma = sigma;          % GPS error radius 1-sigma (m)
    gps.Fs = Fs;                % GPS sampling frequency
end