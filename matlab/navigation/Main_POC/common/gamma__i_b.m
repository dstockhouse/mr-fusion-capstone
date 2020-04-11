function [gamma_x__i_b] = gamma__i_b(r_x__i_b, constants)
% FUNCTION DESCRIPTION:
%   Computes gravitational attraction gamma_?_i_b depending on the r-vector
%   passed input to the fn
%
% INPUTS:
%   r_x__i_b  = Vector from the org of the {i} to {b} coordinatized in {x} frqme (meters)
%   constants = A structure containing many constant parameters (file: load_constants)
%
% OUTPUTS:
%   gamma_x__i_b = Gravitational mass attraction vector
%
% REFERENCE:
%  Navigation Systems: P. Groves

    mu = constants.mu;              % Earth's gravitational constant (in m^3/s^2)
    J2 = constants.J2;              % Earth's second gravitational constant
    R0 = constants.R0;              % Equatorial radius (in meters)

    r_x__i_b_norm = norm(r_x__i_b);                 % |r_*__i_b|
    t_temp = (r_x__i_b(3) / r_x__i_b_norm)^2;       % ( r_*__i_b,z / |r_*__i_b| )^2
    r_temp = [(1 - 5*(t_temp)) * r_x__i_b(1); ...
              (1 - 5*(t_temp)) * r_x__i_b(2); ...
              (3 - 5*(t_temp)) * r_x__i_b(3)];
    gamma_x__i_b = (-mu/r_x__i_b_norm^3) * ...
                   (r_x__i_b + (3/2)*J2*(R0^2/r_x__i_b_norm^2)*r_temp); % Eqn 2.91
end