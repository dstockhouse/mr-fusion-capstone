function warped = warp_image(pointCloud, transformation, constants)
% Input:
%    depth
%      Depth image captured from camera at "native" starting resolution
%    transformation
%      Matrix representation of the camera motion accumulated so far
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
disp_u_i = 0.5 * (cols - 1);
disp_v_i = 0.5 * (rows - 1);

% Step through each pixel
for j=1:length(cols)-1
   for i = 1:length(rows)-1
       x = pointCloud(i,j,1);
       y = pointCloud(i,j,2);
       z = pointCloud(i,j,3);
       if z > 0
           % Transfrom point to warped reference frame
           depth_w = transformation(1,1).*z + transformation(1,2).*x...
               + transformation(1,3).*y + transformation(1,4);
           x_w = transformation(2,1).*z + transformation(2,2).*x...
               + transformation(2,3).*y + transformation(2,4);
           y_w = transformation(3,1).*z + transformation(3,2).*x...
               + transformation(3,3).*y + transformation(3,4);
           % Calculate warping
           uwarp = focal_pt .* x_w./depth_w + disp_u_i;
           vwarp = focal_pt .* y_w./depth_w + disp_v_i;
           if (uwarp >= 0) && (uwarp < length(cols)-1) && (vwarp >= 0)...
                   && (vwarp < length(rows)-1)
              % Contribute the warped pixels to the surrounding ones
              uwarp_l = uwarp;              % WHAT ARE THESE
              uwarp_r = uwarp_l + 1;
              vwarp_d = vwarp;
              vwarp_u = vwarp_d + 1;
              delta_r = uwarp_r - uwarp;
              delta_l = uwarp - uwarp_l;
              delta_u = vwarp_u - vwarp;
              delta_d = vwarp - vwarp_d;
              % Warped pixel close to real value
              if (abs((round(uwarp) - uwarp))+abs((round(vwarp)-vwarp))) < 0.05
                  
              else
                  
              end
           end
       end
   end
end
end

