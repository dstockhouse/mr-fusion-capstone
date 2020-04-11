function k = dcm2k(C)
% Function Description:
%   Converts a 3X3 Rotation matrix C to an angle/axis (k,theta)
%	
% INPUTS:
%   C = 3x3 Rotation matrix
%
% OUTPUTS:
%   k      = 3X1 normalized axis of rotation (dimensionless)
%   theta  = scalar angle of rotation (rad)
%
% NOTES:
%   - The algorithm detects the special cases of theta = 0 or 180
%	- Use the MATLAB "small" number eta

  k  = [0; 0; 0];                   % Initialize k
  Tr  = C(1,1)+C(2,2)+C(3,3);		% Trace(C) = 1 + 2 Cos(theta)
  
  if(Tr >= (3-eps))					% degenerate case Tr=3 => theta = 0
        k  = [0; 0; 0];
		theta = 0;					% in radians
  elseif (Tr <= (-1+eps))			% degenerate case Tr=-1 => theta = Pi
	  theta = pi;
        [Mx,i] = max([C(1,1),C(2,2),C(3,3)]);  % Compute the maximum element of k
        switch i
            case 1  % if( Max = C(1,1) = -1+2*kx*kx )  % Use column 1
                k(1) = sqrt((C(1,1) + 1.0) / 2.0);
                k(2) = C(2,1)/(2*k(1));  
                k(3) = C(3,1)/(2*k(1));
            case 2 % if ( Max = C(2,2) = -1+2*ky*ky)  % Use column 2
                k(2) = sqrt((C(2,2) + 1.0) / 2.0);
                k(1) = C(1,2)/(2*k(2));  
                k(3) = C(3,2)/(2*k(2));
            case 3 % if ( Max = C(3,3) = -1+2*kz*kz)  % Use column 3
                k(3) = sqrt((C(3,3) + 1.0) / 2.0);
                k(1) = C(1,3)/(2*k(3));  
                k(2) = C(2,3)/(2*k(3));
        end
  else    % Neither degenerate case: if -1 < trace(C) < 3
        theta	= acos((Tr-1)/2);
        k(1)    = (C(3,2) - C(2,3)) / (2 * sin(theta));
        k(2)    = (C(1,3) - C(3,1)) / (2 * sin(theta));
        k(3)    = (C(2,1) - C(1,2)) / (2 * sin(theta));
  end
        k     = theta * k;  % Combine the unit axis of rotation & angle of rotation
  end