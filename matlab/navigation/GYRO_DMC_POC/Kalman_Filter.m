function [x_k, P_k] = Kalman_Filter(x_k_1, P_k_1, z_k, A, H, Q, R, G, u)
% The Kalman Filter Algorithm
% FUNCTION DESCRIPTION:
%   This function implements the Kalman Filter Algorithm
%
% INPUTS:
%   x_k_1   = prior state estimate vector: x_hat(k-1)+
%   P_k_1   = prior state error covariance estimate: P hat(k-1)+
%   z_k     = current measurement vector: z(k) = H x(k) + v(k)
%   A       = The state transition matrix: A = PHI(k,k-1)
%   H       = The measurement matrix: z(k) = H x(k) + v(k)
%   Q       = The state noise covariance matrix: E[w w^T ]
%   R       = The measurement noise covariance matrix: E[v v^T ]
%   G       = The system's input matrix
%   u       = The deterministic input to the dynamic system
%
% OUTPUTS:
%   x_k     = The updated state estimate vector: x_hat(k)+
%   P_k     = The updated state error covariance estimate: P hat(k)+
%
% Reference:  EE 440 Lecture notes: Kalman Filtering Part 1
%
% NOTES:        -                    +          
%    x_k_1_m = x      and x_k_1_p = x
%               k-1                  k-1
%
%--------------------------------------------------------------------------
% STEP # 1: Prediction

x_k_m = A * x_k_1 + G * u;              % x(k)- = A x(k-1) + G u(k)
P_k_m = A * P_k_1 * A' + Q;             % P(k)- = A (P(k-1)+) A' + Q(k)

%--------------------------------------------------------------------------
% STEP #2: Gain Calculation

K = P_k_m * H' * inv(H * P_k_m * H' + R);

%--------------------------------------------------------------------------
% STEP # 3: Measurement Update

x_k = x_k_m + K * (z_k - H * x_k_m);    % x(k)+ = (x(k)-) + K (z(k) - H (x(k)-))
P_k = P_k_m - K * H* P_k_m;             % P(k)+ = (I - K H) (P(k)-)

end