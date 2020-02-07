
clear;
close all;

% Read image files
% depth = imread('sample5_depthmap.ppm');
rgb = imread('sample1_rgb.bmp');
% rawpoints = dlmread('sample5_pointcloud.txt');
depth = imread('..\optical_flow\samples\startpoint_depthmap.ppm');
rawpoints = dlmread('..\optical_flow\samples\startpoint_pointcloud.txt');
rawpc = pointCloud(rawpoints);


% Generate point cloud from depth map
% Adapted from 
% https://www.mathworks.com/matlabcentral/answers/285895-how-do-i-directly-covert-a-depth-image-to-3-d-point-cloud
A = double(depth);
n=0;
[s1, s2] = size(A);
% points = ones(s1*s2, 3);
% pointcolor = ones(s1*s2, 3);
for ii = 1:s1
  for jj = 1:s2
      if(A(ii,jj) > 0)
          n = n+1;
          points(n,1) = ii;
          points(n,2) = s2 - jj;
          points(n,3) = A(ii,jj);
          
%           pointcolor(n,1) = 
      end
  end
end

figure(1);
pc = pointCloud(points);
pcshow(pc);
view(0,0);


% tic
% Cut into vertical slices. "Up" is the negative y-axis
[xmin, xmax] = bounds(rawpoints(:,1));
[ymin, ymax] = bounds(rawpoints(:,2));
[zmin, zmax] = bounds(rawpoints(:,3));

xdiff = xmax - xmin;
zdiff = zmax - zmin;

div = 30;
xrange = xmin:(xdiff/div):xmax;
zrange = zmin:(zdiff/div):zmax;

slicecount = zeros(div,div);
for ii = 1:length(rawpoints)
    
    p = rawpoints(ii,:);
    slicex = floor((p(1) - xmin) / xdiff * div) + 1;
    slicez = floor((p(3) - zmin) / zdiff * div) + 1;
    
    if slicex >= div
        slicex = div;
    end
    if slicez >= div
        slicez = div;
    end
    
    slicecount(slicex, slicez) = slicecount(slicex, slicez) + 1;
    
end

%% Determine which slices contain obstacles and indicate
figure(2);
clf
pcshow(rawpc);
xlim([-300 300]);
ylim([-220 120])
zlim([300 1200]);
xlabel('x (mm)');
ylabel('y (mm)');
zlabel('z (mm)');

upperthreshold = 150;
lowerthreshold = 20;
hold on;
for ii = 1:div
    for jj = 1:div

        xlow = (ii-1) * xdiff / div + xmin;
        xhigh = (ii) * xdiff / div + xmin;
        zlow = (jj-1) * zdiff / div + zmin;
        zhigh = (jj) * zdiff / div + zmin;
        
        % Closer spots naturally see more points (?)
        % in a quadratic relationship with distance.
        % Adjust the number of points to weight distant clusters higher
        adjustedcount = slicecount(ii, jj)*(zlow^2+xlow^2)/1000000;
        
        if adjustedcount > upperthreshold || adjustedcount < lowerthreshold
            patch([xlow xlow xhigh xhigh], ymax*ones(1,4), [zlow zhigh zhigh zlow], 'red');%,...
%                  'FaceColor', 'red', 'FaceAlpha', .99);
        end

    end
end

view(0,-90);

% toc
%%
view(0,-90);

%% Animate rotation

if true
    v = VideoWriter('terrain_animation.mp4', 'MPEG-4');
    open(v);
    vfig = figure(2);
    
    for ii=-90:0
        view(0,ii);
        frame = getframe(vfig);
        writeVideo(v, frame);
        pause(.1);
%         zoom on
    end
    close(v);
end


%% display depth map 4

image = imread('sample4_depthmap.ppm');
normimage = mat2gray(image);
figure(3);
imshow(normimage);



rawpoints4 = dlmread('sample4_pointcloud.txt');
rawpc4 = pointCloud(rawpoints4);
figure(4);
clf
pcshow(rawpc4);


