function [clean] = depth_denoise(depth, focal_length, window, threshold)

% Radius of neighboring points that need to exist for point to be valid
if ~exist('window', 'var')
    window = 1;
end

% Percentage of neighboring points that must be present
if ~exist('threshold', 'var')
    threshold = .5;
end

dims = size(depth);
rows = dims(1);
cols = dims(2);
u_center = 0.5 * cols;
v_center = 0.5 * rows;

clean = zeros(rows, cols);
x = zeros(rows, cols);
y = zeros(rows, cols);

for u = 1:cols
    for v = 1:rows
        
        num_valid_points = 0;
        num_total_points = 0;
        neighbor_points = zeros((window*2 + 1)^2, 3);
        % Ensure the point has enough valid neighbors
        for ii = max([u-window, 1]):min([cols, u+window])
            for jj = max([v-window, 1]):min([rows, v+window])
                
                num_total_points = num_total_points + 1;
                if depth(jj, ii) > .001
                    
                    if x(jj, ii) == 0 && y(jj, ii) == 0
                        x(jj, ii) = (ii - u_center) * depth(jj, ii) / focal_length;
                        y(jj, ii) = (jj - v_center) * depth(jj, ii) / focal_length;
                    end
                    
                    num_valid_points = num_valid_points + 1;
                    neighbor_points(num_valid_points, 1) = x(jj, ii);
                    neighbor_points(num_valid_points, 2) = y(jj, ii);
                    neighbor_points(num_valid_points, 3) = depth(jj, ii);

                end
                
            end
        end
        
        % If enough neighbors, we can save the point
        if (num_valid_points / num_total_points) > threshold
            
            if depth(v, u) > .001
                clean(v, u) = depth(v, u);
                
                % Don't attempt to interpolate point for now
%             else
%                 % Determine if neighboring points are coplanar
% 
%                 % 3D linear regression, depth as a function of x and y
%                 % Adapted from https://www.mathworks.com/help/matlab/data_analysis/linear-regression.html
%                 n_x = neighbor_points(1:num_valid_points, 1);
%                 n_y = neighbor_points(1:num_valid_points, 2);
%                 n_z = neighbor_points(1:num_valid_points, 3);
%                 B = [ones(num_valid_points, 1) n_x n_y] \ n_z;
% 
%                 predict_z = [ones(num_valid_points, 1) n_x n_y] * B;
%                 R_squared = 1 - sum((n_z - predict_z).^2) / sum((n_z - mean(n_z)).^2);
% 
%                 % If the regression is a good enough fit (meaning the points are close to planar),
%                 % then use the regression to interpolate the depth of the missing point
%                 if R_squared > .99
%                     cleanz = focal_length * B(1) /...
%                         (focal_length - B(2)*(u - u_center) - B(3)*(v - v_center));
%                     cleanx = (u - u_center) * cleanz / focal_length;
%                     cleany = (v - v_center) * cleanz / focal_length;
%                     clean(v, u) = cleanz;
%                 end
            end

        end

    end
end

end

