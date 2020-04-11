function [r_t__t_b, v_t__t_b, a_t__t_b, C_t__b, w_t__tb, C_t__bm_out] = body_in_tan(t, first_run_flag,C_t__bm, constants) 
% Function Name: body_in_tan.m
% Description: Generates ground truth parameters in the body frame. 

dT = constants.dt;                  % Sample interval (sec)

Tx = constants.t_end;               % Period of the figure eight (sec) 
fx = 1/Tx;                          % Freq of the Fig eight (Hz)
wx = 4*pi*fx;                       % Freq of the Fig eight (rad/s)

Ax = 20;                            % Amplitude of fig eight in x-dir
Ay = 100;                           % Amplitude of fig eight in y-dir
Az = 50;                            % Amplitude of fig eight in z-dir

% Position of the b-frame wrt the t-frame resolved in {t} (m)
r_t__t_b = [ Ax      * sin(wx*t)   % A North-East-Down (NED) frame
            -Ay      * sin((wx/2)*t)
            -Az      * sin((wx/4)*t)];

% Velocity of the b-frame wrt the t-frame resolved in {t} (m)
v_t__t_b = [ Ax*wx   * cos(wx*t)        % A North-East-Down (NED) frame
            -Ay*wx/2 * cos(t*wx/2)
            -Az*wx/4 * cos(t*wx/4)];  
       
% Accel of the org of the b-frame wrt the t-frame described in the t-frame
a_t__t_b = [-Ax*wx^2    * sin(wx*t)     % A North-East-Down (NED) frame
             Ay*wx^2/4  * sin(t*wx/2)
             Az*wx^2/16 * sin(t*wx/4)];       
        
% Orientation of the b-frame wrt the t-frame (a forward-right-down frame)
x_axis = v_t__t_b / norm(v_t__t_b);     % x-axis points "forward"
% See generic form of the RPY matrix
pitch = asin(-x_axis(3));               % pitch angle (rad)
roll  = 0;
yaw   = atan2(x_axis(2), x_axis(1));    % yaw angle (rad)
C_t__b = ypr2dcm(yaw, pitch, roll);

% Add some roll based on the y-axis accel in the body frame
a_temp = C_t__b' * a_t__t_b;
roll = atan2(a_temp(2), 10);            % Relative to ~g
C_t__b = ypr2dcm(yaw, pitch, roll);     % Update DCM  

if first_run_flag == 1
    w_t__tb = [0;0;0];  % Initialize at 0
    C_t__bm = eye(3);
else 
    Delta_C = C_t__b * C_t__bm';    % C_t__b(k) = exp(OHMC_t__tb*dT) C_t__b(k-1)
    w_t__tb = dcm2k(Delta_C) / dT;  % C_t__b(k) = C_t__b(k-1) exp(OHMC_b__tb*dT)
end
C_t__bm_out = C_t__b; 
end