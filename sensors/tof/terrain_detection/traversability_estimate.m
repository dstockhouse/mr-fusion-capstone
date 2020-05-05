% Environmental Traversability Estimation using 3D ToF
% Joy Fucella and David Stockhouse

clear; clc; close all;


%% Read image file
addpath('../samples');
addpath('../optical_flow');
% From video captured from dragonboard
samples_path = '../samples';
addpath(samples_path);
filename = [samples_path '/far_noise_reduce_60_rotate.tof'];
% filename = [samples_path '/far_noise_reduce_60_linear_move.tof'];
% filename = [samples_path '/medium_noise_reduce_60_rotate.tof'];
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


%% Traversability Estimate Calculation

% Desample to remove noise and speed up computation
gaussian_levels = 2;

% Start video recording
[path, fname, ext] = fileparts(filename);
moviename = [fname '_traversability.mp4'];
v = VideoWriter(moviename, 'MPEG-4');
v.FrameRate = constants.fps;
open(v);

for frame_index = 1:constants.num_frames
    
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
    new_depth = fuse_ir_depth(new_depth, ir_frame, ir_max_thresh, ir_min_thresh, false);
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
    
    div = div + 1;
    
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

            end
        end
    end
    
    %% Determine which slices contain obstacles
    
    numstd = 1.2;
    logcount = log(slicecountpolar+1);
    upperthreshold = exp(mean(logcount, 'all') + numstd*std(logcount, 1, 'all'));
    lowerthreshold = exp(mean(logcount, 'all') - numstd*std(logcount, 1, 'all'));
    adjustedcount = zeros(div);
    patch_xdimensions = zeros(4, div*div);
    patch_zdimensions = zeros(4, div*div);
    patch_color = zeros(div*div, 1);
    color_index_red = 1;
    color_index_green = 2;
    color_lookup = ["red", "green"];
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
                patch_color((ii-1)*div + jj) = color_index_red;
            else
                patch_color((ii-1)*div + jj) = color_index_green;
            end
            patch_xdimensions(:, (ii-1)*div + jj) = [xlowclose; xlowfar; xhighfar; xhighclose];
            patch_zdimensions(:, (ii-1)*div + jj) = [zlowclose; zlowfar; zhighfar; zhighclose];
             
        end
    end
    
    
    %% Determine best traversable direction
    
    % Keep track of how traversable each direction is
    traversability_score = ones(div, 1);
    
    % Loop through all distance divisions
    for ii = 1:div
        % Loop through all angle divisions
        for jj = 1:div

            distavg = (ii-.5) * distdiff / div + distmin;
            angavg = (jj-.5) * angdiff / div + angmin;

            if patch_color((ii-1)*div + jj) == color_index_red
                traversability_score(jj) = traversability_score(jj) - 1/ii;
            end

        end
    end
    
    % Select most traversable direction
%     best_direction = find(traversability_score == max(traversability_score), 1);
    qx = distmin .* sin(angrange);
    qy = ymin * ones(1, div);
    qz = distmin .* cos(angrange);
    qu = traversability_score' .* sin(angrange);
    qv = zeros(1, div);
    qw = traversability_score' .* cos(angrange);
    
    % Scale and exclude the arrows that are negative
    quiver_scale = distdiff;
    arrow_exclude = traversability_score > 0;
    qx = qx(arrow_exclude);
    qy = qy(arrow_exclude);
    qz = qz(arrow_exclude);
    qu = qu(arrow_exclude);
    qv = qv(arrow_exclude);
    qw = qw(arrow_exclude);
    
    %% Make a Nice Plot
    
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
    % Point cloud
    pcshow(vecpoints);
    view([0 -.3 -1]);
    hold on;
    grid on;
    % Traversable/nontraversable patches
    for ii = 1:div*div
        patch(patch_xdimensions(:,ii), ymax*ones(4, 1), patch_zdimensions(:,ii), color_lookup(patch_color(ii)));
    end
    % Directions with score
    quiver3(qx, qy, qz, qu, qv, qw, 'AutoScale', 'off', 'LineWidth', 1, 'Color', 'w');
    title('Point Cloud (front)');
    xlabel('x (m)');
    ylabel('y (m)');
    zlabel('z (m)');
    axis([ -4 4 -4 3 0 6]);
    
    
    % Bottom right plot
    subplot(2, 2, 4);
    % Point cloud
    pcshow(vecpoints);
    view([0 -1 0]);
    grid on;
    hold on;
    % Traversable/nontraversable patches
    for ii = 1:div*div
        patch(patch_xdimensions(:,ii), ymax*ones(4, 1), patch_zdimensions(:,ii), color_lookup(patch_color(ii)));
    end
    % Directions with score
    quiver3(qx, qy, qz, qu, qv, qw, 'AutoScale', 'off', 'LineWidth', 1, 'Color', 'w');
    title(['Traversable Regions'], 'color', 'w');
    xlabel('x (m)');
    ylabel('y (m)');
    zlabel('z (m)');
    axis([ -4 4 -4 3 0 6]);
    
    if exist('sgtitle', 'builtin') || exist('sgtitle', 'file')
        % Figure title
        sgtitle(['Frame ' num2str(frame_index) ' of ' num2str(num_frames) ...
            ' (' num2str(frame_index/fps, '%.1f') '/' num2str(num_frames/fps, '%.1f') ' s)'],...
            'Color', 'w');
    end    

    frame = getframe(vfig);
    writeVideo(v, frame);
    
    
    % Log traversability scores to console
    fprintf('Trav: ');
    for ii = 1:div
        fprintf('%4.2f ', traversability_score(ii));
    end
    fprintf('\n');
    
    
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
