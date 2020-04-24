function [p_depth, p_points] = gaussian_pyramid(depth, levels, constants)
% Computes a Gaussian pyramid from a high-resolution depth image
% Joy Fucella and David Stockhouse

% Input:
%    depth
%      Depth image captured from camera at "native" starting resolution
%
%    levels
%      Number of levels to compute for the pyramid
%
% Output:
%    p_depth
%      3D array (list of 2D images) containing pyramid levels. Lower resolution
%      levels of the pyramid have more space than they use, but each resolution
%      decreases by half of the previous resolution.
%    p_points
%      Pixel-indexed Point cloud of the points in the depth map
%    constants
%      Struct containing constant parameters

% Thresholding information
edge_depth_thresh = 10;

% Get dimensions of original image
depth_dim = size(depth);
rows = depth_dim(1);
cols = depth_dim(2);

% Setup empty pyramids
p_depth = zeros(levels, rows, cols);
p_points = zeros(levels, rows, cols, 3); % xyz point at each pixel

% Copy first image to pyramid
p_depth(1, :, :) = depth(:, :);

% Gaussian mask
kernel_v = [1 4 6 4 1];
gauss_kernel = zeros(5);
for ii = 1:5
    for jj = 1:5
        gauss_kernel(ii, jj) = kernel_v(ii) * kernel_v(jj) / 256;
    end
end

% Manual convolution and de-resolution
for level = 1:levels

    scale_factor = 2^(level-1);

    cols_l = cols / scale_factor;
    rows_l = rows / scale_factor;

    % Filter previous image into current image for all but the first one
    if level > 1

        for ii = 1:cols_l
            for jj = 1:rows_l

                % Center of this pixel on previous image
                center_prev_depth = p_depth(level - 1, jj*2, ii*2);

                % For inner pixels
                if ii > 2 && jj > 2 && ii < (cols_l-1) && jj < (rows_l-1)

                    % True filter only if valid depth at this space
                    if center_prev_depth > 0

                        sum = 0;
                        weight = 0;

                        % Manual kernel convolution
                        for kernel_i = -2:2
                            for kernel_j = -2:2

                                % Depth for this kernel position
                                kernel_prev_depth = p_depth(level - 1, jj*2, ii*2);

                                % Ensure depth is close enough to be the same object
                                depth_diff = abs(center_prev_depth - kernel_prev_depth);
                                if depth_diff < edge_depth_thresh

                                    % Here's a step away from gaussian;
                                    %   Points that are closer in depth are weighted more
                                    proximity_weight = gauss_kernel(kernel_i + 3, kernel_j + 3) ...
                                        * (edge_depth_thresh - depth_diff);

                                    % Update cumulative sum and weight
                                    weight = weight + proximity_weight;
                                    sum = sum + proximity_weight * kernel_prev_depth;

                                end % if depth_diff < thresh

                            end % for kernel_j
                        end % for kernel_i

                        % Save depth to downsampled pixel
                        p_depth(level, jj, ii) = sum / weight;

                    end % if prev_depth > 0

                else % for boundary pixels

                    % Ignore complicated stuff and duplicate close to edges for now

                end % inner/boundary pixels

            end % for jj
        end % for ii
        
        % Duplicate edge pixels
        % TODO implement more complicated blurring
        p_depth(level, :, 1) = 0;
        p_depth(level, :, 2) = 0;
        p_depth(level, 1, :) = 0;
        p_depth(level, 2, :) = 0;
        p_depth(level, :, cols_l  ) = 0;
        p_depth(level, :, cols_l-1) = 0;
        p_depth(level, rows_l, :) = 0;
        p_depth(level, rows_l-1, :) = 0;

    end % if level > 1

    % Calculate point cloud (xyz coords) for each pixel
    % Could use depth2points function instead
    level_f_length = cols_l / (2 * tan(0.5 * constants.fov_horizontal));
    u_center_l = 0.5 * cols_l;
    v_center_l = 0.5 * rows_l;

    for ii = 1:cols_l
        for jj = 1:rows_l

            z = p_depth(level, jj, ii);

            % X, Y, and Z points
            p_points(level, jj, ii, 1) = (ii - u_center_l) * z / level_f_length;
            p_points(level, jj, ii, 2) = (jj - v_center_l) * z / level_f_length;

            % Can just be done as a vector above
            p_points(level, jj, ii, 3) = z;

        end % for jj
    end % for ii

end % for level = 1:levels

end

