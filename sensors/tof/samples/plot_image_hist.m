function plot_image_hist(image)

max_intensity = max(max(image));
cnt = zeros(max_intensity+1, 1);
for ii = 0:max_intensity
%     fprintf('%d: %d\n', ii, sum(sum(image == ii)));
    cnt(ii+1) = sum(sum(image == ii));
end
plot(0:max_intensity, cnt)
title('Pixel frequency');
xlabel('Intensity');
ylabel('Count');

end