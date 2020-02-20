function points = depth2points(depth, focal_length)

% Get dimensions of image
depth_dim = size(depth);
rows = depth_dim(1);
cols = depth_dim(2);
u_center = 0.5 * cols;
v_center = 0.5 * rows;

% Pre-allocate
points = zeros(rows, cols, 3);

% Iterate through all pixels, computing xyz coords of each
for u = 1:cols
    for v = 1:rows
        
        z = depth(v, u);
        
        % X, Y, and Z points
        points(v, u, 1) = (u - u_center) * z / focal_length;
        points(v, u, 2) = (v - v_center) * z / focal_length;
        
        % Can just be done as a vector above
        points(v, u, 3) = z;
        
    end % for jj
end % for ii

end