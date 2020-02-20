function warped = warp_image(pointCloud, transformation, constants)
% Input:
%    depth
%      Depth image captured from camera at "native" starting resolution
%    transformation
%      Matrix representation of the camera motion accumulated so far
%          looks like: [  R | v ;
%                        ___|___
%                         0 | 1 ;
%
%          where R is the rotation about the angular velocity: 
%                      [  0  -wz  wy]
%                      [  wz  0  -wx]
%                   e^ [ -wy  wx  0 ]*dt
%          and v is the linear velocity:
%                      [ vx ]
%                      [ vy ]
%                      [ vz ]*dt
%          Note: The Jaimez paper shifts the velocity state so that z is the first index
%    constants
%      Structure of the camera parameters
%
% Output:
%    warped - the warped image
%
% Warps input image pixels according to motion flow

% Get dimensions of original image
depth_dim = size(pointCloud);
rows = depth_dim(1);
cols = depth_dim(2);
focal_pt = cols / (2*tan(0.5*constants.fovh));
inv_f = 1/focal_pt;
% disp_u_i = 0.5 * (cols - 1);
% disp_v_i = 0.5 * (rows - 1);
disp_u_i = 0.5 * cols;
disp_v_i = 0.5 * rows;

% Pre-allocate warped image
warped = zeros(rows, cols, 3);
warped2 = zeros(rows, cols, 3);
wacu = zeros(rows, cols);

% Step through each pixel, warping based on previous estimated velocity (prev step in pyramid)
for i=1:cols
   for j = 1:rows
       x = pointCloud(j,i,1);
       y = pointCloud(j,i,2);
       z = pointCloud(j,i,3);
       if z > 0

           % In vector form: r(x,y,z) = (w {cross} r) + v
	   % The below can be done with a standard matrix multiplication in matlab
	   % warped_r = transformation * [x; y; z; 1];
	   % x_w     = warped_r(1);
	   % y_w     = warped_r(2);
	   % depth_w = warped_r(3);

           % Transfrom point to warped reference frame
	   %%%% IMPORTANT: If we keep the expanded matrix arithmetic, we need to swap z, x, y to match x, y, z instead
%            depth_w = transformation(1,1).*z + transformation(1,2).*x...
%                + transformation(1,3).*y + transformation(1,4);
%            x_w = transformation(2,1).*z + transformation(2,2).*x...
%                + transformation(2,3).*y + transformation(2,4);
%            y_w = transformation(3,1).*z + transformation(3,2).*x...
%                + transformation(3,3).*y + transformation(3,4);
           warped_r = transformation * [x; y; z; 1];
           x_w     = warped_r(1);
           y_w     = warped_r(2);
           depth_w = warped_r(3);

           % Calculate pixel that warped point belongs in (from cam parameters)
           uwarp = focal_pt .* x_w./depth_w + disp_u_i;
           vwarp = focal_pt .* y_w./depth_w + disp_v_i;
           if (uwarp >= 0) && (uwarp < (cols-1)) && (vwarp >= 0)...
                   && (vwarp < (rows-1))
              % Contribute the warped pixels to the surrounding ones
              uwarp_l = floor(uwarp);              % WHAT ARE THESE
              uwarp_r = uwarp_l + 1;
              vwarp_d = floor(vwarp);
              vwarp_u = vwarp_d + 1;
              delta_r = uwarp_r - uwarp;
              delta_l = uwarp - uwarp_l;
              delta_u = vwarp_u - vwarp;
              delta_d = vwarp - vwarp_d;
              
              % Warped pixel close to real value
              % Note: Pixel indices in matlab are 1-indexed, so plus one for each
              if (abs((round(uwarp) - uwarp))+abs((round(vwarp)-vwarp))) < 0.05
                  warped(round(vwarp)+1,round(uwarp)+1,3) = warped(round(vwarp)+1,round(uwarp)+1,3) + depth_w;
                  wacu(round(vwarp)+1,round(uwarp)+1) = wacu(round(vwarp)+1,round(uwarp)+1) + 1;
              else % Weight the closest pixels
                  w_ur = delta_l.^2 + delta_d.^2;
                  warped(vwarp_u+1, uwarp_r+1,3) = warped(vwarp_u+1, uwarp_r+1,3) + (w_ur .* depth_w);
                  wacu(round(vwarp_u)+1,round(uwarp_r)+1) = wacu(round(vwarp_u)+1,round(uwarp_r)+1) + w_ur;
                  
                  w_ul = delta_r.^2 + delta_d.^2;
                  warped(vwarp_u+1, uwarp_l+1,3) = warped(vwarp_u+1, uwarp_l+1,3) + (w_ul .* depth_w);
                  wacu(round(vwarp_u)+1,round(uwarp_l)+1) = wacu(round(vwarp_u)+1,round(uwarp_l)+1) + w_ul;
                  
                  w_dr = delta_l.^2 + delta_u.^2;
                  warped(vwarp_d+1, uwarp_r+1,3) = warped(vwarp_d+1, uwarp_r+1,3) + (w_dr .* depth_w);
                  wacu(round(vwarp_d)+1,round(uwarp_r)+1) = wacu(round(vwarp_d)+1,round(uwarp_r)+1) + w_dr;
                  
                  w_dl = delta_r.^2 + delta_u.^2;
                  warped(vwarp_d+1, uwarp_l+1,3) = warped(vwarp_d+1, uwarp_l+1,3) + (w_dl .* depth_w);
                  wacu(round(vwarp_d)+1,round(uwarp_l)+1) = wacu(round(vwarp_d)+1,round(uwarp_l)+1) + w_dl;
              end
           end
       end
   end
end

% Scale average depth and compute spatial coordinates
for u = 1:cols
   for v = 1:rows
       if wacu(v,u) > 0
           warped(v,u,3) = warped(v,u,3)/wacu(v,u);
           warped(v,u,1) = (u - disp_u_i) * warped(v,u,3) * inv_f;
           warped(v,u,2) = (v - disp_v_i) * warped(v,u,3) * inv_f;
           warped2(v,u,:) = warped(v,u,:);
           
       else
           warped(v,u,3) = 0;
           warped(v,u,1) = 0;
           warped(v,u,2) = 0;
       end
   end
   % Plot warped pixels one row at a time (for debugging)
%    plot_pixel_points_slow(warped2);
%    view(3);
end

end % warp_image
