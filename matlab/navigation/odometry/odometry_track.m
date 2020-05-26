
ROBOT_RADIUS = .5524 * 1000;

data = readmatrix('ODOMETRY_K-05.21.2020_09-00-02_2fa21b9a.csv');
diff_data = data(2:end,:) - data(1:(end-1),:);

l_dist = data(:,1); % mm
r_dist = data(:,2); % mm
time = data(:,3);

l_diff = diff_data(:,1);
r_diff = diff_data(:,2);
timestep = diff_data(:,3);

% Linear velocity of the center of robot at each timestep
lin_diff = (l_diff + r_diff) / 2;
lin_vel = lin_diff ./ timestep;

nonlin_diff = r_diff - l_diff;
ang_diff = nonlin_diff / ROBOT_RADIUS;
ang_vel = ang_diff ./ timestep;


% Accumulate differences
heading = cumsum(ang_diff);

x_contrib = lin_diff .* -sin(heading);
y_contrib = lin_diff .* cos(heading);
x = cumsum(x_contrib);
y = cumsum(y_contrib);

num_frames = length(timestep);
% As loop rather than as vector operations
% heading = zeros(num_frames, 1);
% x = zeros(num_frames, 1);
% y = zeros(num_frames, 1);
% for ii = 2:length(timestep)
% 
%     % Update position from previous heading
%     x(ii) = x(ii-1) - lin_diff(ii-1) * sin(heading(ii-1))
%     y(ii) = y(ii-1) + lin_diff(ii-1) * cos(heading(ii-1))
% 
%     % Accumulate heading from angular distances
%     heading(ii) = heading(ii-1) + ang_diff(ii);
% 
% end

% Plot robot position
figure(1);
for ii = 1:num_frames

    plot(x(1:ii), y(1:ii), 'k');
    line([x(ii) x(ii)-ROBOT_RADIUS*sin(heading(ii))], [y(ii) y(ii)+ROBOT_RADIUS*cos(heading(ii))],...
        'Color', 'b');
    line([x(ii)-ROBOT_RADIUS*cos(heading(ii)) x(ii)+ROBOT_RADIUS*cos(heading(ii))],...
        [y(ii)-ROBOT_RADIUS*sin(heading(ii)) y(ii)+ROBOT_RADIUS*sin(heading(ii))],...
        'Color', 'b');

    title(['Time: ' num2str(time(ii), '%.2f') '/' num2str(time(end), '%.2f') ' s']);
    xlabel('x (mm)');
    ylabel('y (mm)');

    grid on;
    
    axis([min(x) max(x) min(y) max(y)]);
    axis equal;

    pause(1e-2);

end

