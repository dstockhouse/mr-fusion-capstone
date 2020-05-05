function plot_pixel_points_slow(points)

depth_dim = size(points);
rows = depth_dim(1);
cols = depth_dim(2);

plot_points(1,1:3) = 0;
valid_points = 0;
for u = 1:cols
    for v = 1:rows
        
        if points(v, u, 3) > 10
            valid_points = valid_points + 1;
            plot_points(valid_points, 1:3) = points(v, u, 1:3);
        end
        
    end
end
pcshow(plot_points);
xlabel('x');
ylabel('y');
zlabel('z');

end