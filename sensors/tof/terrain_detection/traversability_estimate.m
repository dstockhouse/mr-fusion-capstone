% Environmental Traversability Estimation using 3D ToF
% Joy Fucella and David Stockhouse

clear; clc; close all;


%% Read image file
addpath('../samples');
addpath('../optical_flow');
% From video captured from dragonboard
samples_path = '../samples';
addpath(samples_path);
% filename = [samples_path '/far_noise_reduce_60_rotate.tof'];
% filename = [samples_path '/far_noise_reduce_60_linear_move.tof'];
filename = [samples_path '/medium_noise_reduce_60_rotate.tof'];
% filename = [samples_path '/medium_noise_reduce_60_linear_move.tof'];
% filename = [samples_path '/far_move_forward_then_back.tof'];
% filename = [samples_path '/medium_apt_rotate.tof'];
% filename = [samples_path '/far_apt_stationary.tof'];
% filename = [samples_path '/medium_apt_stationary.tof'];
% filename = [samples_path '/cart2_very_long_path.tof'];

% Ensure file exists where expected
if ~isfile(filename)
    fprintf('File ''%s'' does not exist. Look at the README in ''%s'' for instructions on downloading sample.\n',...
        filename, samples_path);
    return
end

% Read metadata from file header
fprintf('Reading image header from %s...\n', filename);
[fd, constants] = read_tof_header(filename);

% Set additional ToF dev kit parameters
constants.fov_horizontal = 90    *pi/180;
constants.fov_vertical   = 69.2  *pi/180;
constants.fovh = constants.fov_horizontal;
constants.fovv = constants.fov_vertical;

% Print some of the stats:
fprintf('\trows: %d\n', constants.rows);
fprintf('\tcols: %d\n', constants.cols);
fprintf('\tfps:  %d\n', constants.fps);
fprintf('\t %d frames for %.3f second video duration\n',...
    constants.num_frames, constants.duration);
rows = constants.rows;
cols = constants.cols;
fps = constants.fps;
num_frames = constants.num_frames;
focal_length = cols / (2 * tan(0.5 * constants.fovh));
constants.focal_length = focal_length;


% Thresholding constants for image filtering
ir_min_thresh = 30;
ir_max_thresh = 4096;

% Read starting frame
[depth_frame, ir_frame] = read_tof_frame(fd, constants.frame_size, constants);

% Reduce integer depth (mm) measurements to double (m) measurements
depth_frame = double(depth_frame) / 1000;


%%%%%%%% Start Traversability Estimate Calculation %%%%%%%%
%% Traversability Estimate Calculation

% Desample to remove noise and speed up computation
gaussian_levels = 2;

% Start video recording
[path, fname, ext] = fileparts(filename);
moviename = [fname '_traversability.mp4'];
v = VideoWriter(moviename, 'MPEG-4');
v.FrameRate = constants.fps;
open(v);

for frame_index = 2:constants.num_frames
    
    fprintf('Computing traversability on frame %d of %d\n', frame_index, constants.num_frames);
    
    % Read next frame
    [new_depth, ir_frame] = read_tof_frame(fd, constants.frame_size, constants);
    new_depth = double(new_depth) / 1000;
    rows = constants.rows;
    cols = constants.cols;
    
    %% Gaussian pyramid
    
    % Construct pyramid with new frames
    denoise_window = 6;
    denoise_threshold = .7;
    new_depth = fuse_ir_depth(new_depth, ir_frame, ir_max_thresh, ir_min_thresh);
    new_depth = depth_denoise(new_depth, focal_length, denoise_window, denoise_threshold);
    [p_depth_new, p_points_new] = gaussian_pyramid(new_depth, gaussian_levels, constants);
    
    rows = rows/(2^(gaussian_levels-1));
    cols = cols/(2^(gaussian_levels-1));
    rawdepth = reshape(p_depth_new(gaussian_levels, 1:rows, 1:cols), rows, cols);
    rawpoints = reshape(p_points_new(gaussian_levels, 1:rows, 1:cols, :), rows, cols, 3);
    
    % Linearize point cloud into vector format
    vecdepth = reshape(rawdepth, rows*cols, 1);
    vecpoints = reshape(rawpoints, rows*cols, 3);
    vecpointsy = reshape(vecpoints(:,2), rows*cols, 1);
    y_select = vecpointsy > -1;
    z_select = (vecdepth > .001) & (vecdepth < .99*max(vecdepth));
    vecdepth = vecdepth(z_select);
    rawdepthclean = rawdepth;
    rawdepthclean(~z_select) = 0;
    vecpoints = reshape(vecpoints([z_select z_select z_select]), length(vecdepth), 3);
    
    
    %% Cut into vertical slices. "Up" is the negative y-axis
    
    % Determine spread of data
%     p_valid = 
    xmin = min(min(vecpoints(:,1)));
    xmax = max(max(vecpoints(:,1)));
    ymin = min(min(vecpoints(:,2)));
    ymax = max(max(vecpoints(:,2)));
%     zmin = min(min(vecpoints(:,3)));
    zmin = 0;
    zmax = max(max(vecpoints(:,3)));
    xdiff = xmax - xmin;
    zdiff = zmax - zmin;
    
    vecdist = sqrt(vecpoints(:, 1).^2 + vecpoints(:, 2).^2 + vecpoints(:, 3).^2);
    distmin = min(vecpoints(:,3));
    distmax = max(vecpoints(:,3));
    distdiff = distmax - distmin;
    
    angmin = -constants.fovh/2;
    angmax = constants.fovh/2;
    angdiff = angmax - angmin;
    
    % Number of divisions, and the range of those divisions
    div = 15;
    xrange = xmin:(xdiff/div):xmax;
    zrange = zmin:(zdiff/div):zmax;
    
    distrange = distmin:(distdiff/div):distmax;
    angrange = angmin:(angdiff/div):angmax;
    
    % Determine which 2D slice the point belongs in
    slicecount = zeros(div,div);
    slicecountquad = zeros(div,div);
    slicecountpolar = zeros(div,div);
    for ii = 1:length(vecpoints)
        
        if vecdepth(ii) > .001
            
            % Identify which bin it should go in
            p = reshape(vecpoints(ii,:), 1, 3);
            
            % Only count as 'obstacle' if it's less than 1 meter above
            % the robot
            if p(2) > -1
                
                slicex = floor((p(1) - xmin) / xdiff * div) + 1;
                slicez = floor((p(3) - zmin) / zdiff * div) + 1;
                if slicex >= div
                    slicex = div;
                end
                if slicez >= div
                    slicez = div;
                end
                
                % It belongs in this slice, so count it
                slicecount(slicex, slicez) = slicecount(slicex, slicez) + 1;
                slicecountquad(slicex, slicez) = slicecountquad(slicex, slicez) + norm(p).^2;
                
                
                % For polar slices
                slicedist = floor((norm(p) - distmin) / distdiff * div) + 1;
                p_angle = atan(p(1)/p(3));
                sliceang = floor((p_angle - angmin) / angdiff * div) + 1;
                if slicedist >= div
                    slicedist = div;
                end
                if sliceang >= div
                    sliceang = div;
                end
                slicecountpolar(slicedist, sliceang) = slicecountpolar(slicedist, sliceang) + 1;

%                 slicecountpolar
%                 p
%                 pause(.001)
            end
        end
    end
    
    %% Determine which slices contain obstacles and make a Nice Plot
    
    vfig = figure(1);
    clf;
    
    % Top left plot
    subplot(2, 2, 1);
    min_depth = min(min(vecdepth));
    max_depth = max(max(vecdepth));
    imshownorm(rawdepthclean, [min_depth max_depth]);
    xlabel('Depth', 'Color', 'w', 'FontWeight', 'bold');
    
    % Top right plot
    subplot(2, 2, 2);
    imshownorm(ir_frame);
    xlabel('IR Intensity', 'Color', 'w', 'FontWeight', 'bold');
    
    % Bottom left plot
    subplot(2, 2, 3);
    pcshow(vecpoints);
    title('Point Cloud (front)');
    view([0 0 -1]);
    xlabel('x (m)');
    ylabel('y (m)');
    zlabel('z (m)');
    axis([ -4 4 -4 3 0 6]);
    
    
    % Bottom right plot
    subplot(2, 2, 4);
    pcshow(vecpoints);
    title(['Traversable Regions'], 'color', 'w');
    grid on;
    xlabel('x (m)');
    ylabel('y (m)');
    zlabel('z (m)');
    
    upperthreshold = mean(slicecountpolar, 'all') + 1*std(slicecountpolar, 1, 'all');
    lowerthreshold = mean(slicecountpolar, 'all') - 1*std(slicecountpolar, 1, 'all');
    hold on;
    adjustedcount = zeros(div);
    for ii = 1:div
        for jj = 1:div

            % Rectangular dimensions of this region
            distlow = (ii-1) * distdiff / div + distmin;
            disthigh = (ii) * distdiff / div + distmin;
            anglow = (jj-1) * angdiff / div + angmin;
            anghigh = (jj) * angdiff / div + angmin;

            xlowclose = distlow*sin(anglow);
            xlowfar = disthigh*sin(anglow);
            xhighclose = distlow*sin(anghigh);
            xhighfar = disthigh*sin(anghigh);
            zlowclose = distlow*cos(anglow);
            zlowfar = disthigh*cos(anglow);
            zhighclose = distlow*cos(anghigh);
            zhighfar = disthigh*cos(anghigh);
            
            % If the region has too many points, plot red, else plot green
            if slicecountpolar(ii, jj) > upperthreshold || slicecountpolar(ii, jj) < lowerthreshold
                color = 'red';
            else
                color = 'green';
            end
            patch([xlowclose xlowfar xhighfar xhighclose], ymax*ones(1,4), [zlowclose zlowfar zhighfar zhighclose], color);
            
        end
    end
    
    view([0 -1 0]);
%     pause(.001);
    
    if exist('sgtitle', 'builtin') || exist('sgtitle', 'file')
        % Figure title
        sgtitle(['Frame ' num2str(frame_index) ' of ' num2str(num_frames) ...
            ' (' num2str(frame_index/fps, '%.1f') '/' num2str(num_frames/fps, '%.1f') ' s)'],...
            'Color', 'w');
    end    

    frame = getframe(vfig);
    writeVideo(v, frame);
    
    
    %% Histogram of counts
    
%     figure(2);
%     bar3(slicecount);
%     title('Histogram of raw point counts');
%     
%     figure(3);
%     bar3(slicecountquad);
%     title('Histogram of quadratic adjusted point counts');
%     
%     figure(4);
%     bar3(adjustedcount);
%     title('Histogram of coarse adjusted point counts');
%     
%     figure(5);
%     bar3(sclicecountpolar);
%     title('Histogram of polar-counted point counts');
  
    

end % for frame_index

% Close image stream file
fclose(fd);

% Close animation video file
v.FrameRate;
close(v);
fprintf('Finished writing ''%s''\n', moviename);
