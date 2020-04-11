function [C_out] = normalize_C(C)
% Description:
%   Function that normalizes a rotation matrix
%
% Inputs:
%   C:          a 3X3 rotation matrix (may NOT be orthonormal)
%
% Outputs:
%   C_out:      a 3X3 rotation matrix (will be orthonormal)

%% Normalize using C -> k,theta -> C
% k = zeros(3,1);     % Initialize k (axis of rotation)
% 
% % Convert DCM -> angle axis
%     Tr  = C(1,1)+C(2,2)+C(3,3);     % Trace(C) = 1 + 2Cos(theta)
%     theta = acos((Tr-1)/2);
%     tmp = 1/(2 * sin(theta));
%     k(1)    = (C(3,2) - C(2,3)) * tmp;
%     k(2)    = (C(1,3) - C(3,1)) * tmp;
%     k(3)    = (C(2,1) - C(1,2)) * tmp;
%         
%     % Convert angle axis -> DCM
%      K = [   0, -k(3),  k(2);        % Skew-symmetric matrix form of the normalized axis of rotation
%           k(3),     0, -k(1); 
%          -k(2),  k(1),    0];
%      
%    % Rodrigues formula
%    C_out = eye(3) + sin(theta) * K + (1 - cos(theta)) * K^2;
   
   
%% Normalize using C -> Quarternion -> C
Tr0 =  C(1,1) + C(2,2) + C(3,3);    % = 4 qo^2 - 1
Tr1 =  C(1,1) - C(2,2) - C(3,3);    % = 4 q1^2 - 1
Tr2 = -C(1,1) + C(2,2) - C(3,3);    % = 4 q2^2 - 1
Tr3 = -C(1,1) - C(2,2) + C(3,3);    % = 4 q3^2 - 1

[Trn,i] = max([Tr0, Tr1, Tr2, Tr3]);
qn      = sqrt(Trn+1)/2;

switch i
 case 1  % Qn = q0
  q = [qn; (C(3,2)-C(2,3))/(4*qn); (C(1,3)-C(3,1))/(4*qn); (C(2,1)-C(1,2))/(4*qn)];
 case 2  % Qn = q1
  q = [(C(3,2)-C(2,3))/(4*qn); qn; (C(2,1)+C(1,2))/(4*qn); (C(1,3)+C(3,1))/(4*qn)];
 case 3  % Qn = q2
  q = [(C(1,3)-C(3,1))/(4*qn); (C(2,1)+C(1,2))/(4*qn); qn; (C(3,2)+C(2,3))/(4*qn)];
 case 4  % Qn = q3
  q = [(C(2,1)-C(1,2))/(4*qn); (C(1,3)+C(3,1))/(4*qn); (C(3,2)+C(2,3))/(4*qn); qn];
end   
% Normalize
Qmag = sqrt( q(1)^2 + q(2)^2 + q(3)^2 + q(4)^2 );
q = q / Qmag;
q0 = q(1); q1 = q(2); q2 = q(3); q3 = q(4);
C_out = [ q0*q0+q1*q1-q2*q2-q3*q3   2*(q1*q2-q0*q3)         2*(q1*q3+q0*q2);
       2*(q1*q2+q0*q3)          q0*q0-q1*q1+q2*q2-q3*q3 2*(q2*q3-q0*q1);
       2*(q1*q3-q0*q2)          2*(q2*q3+q0*q1)         q0*q0-q1*q1-q2*q2+q3*q3];
end     % End function normalize_C