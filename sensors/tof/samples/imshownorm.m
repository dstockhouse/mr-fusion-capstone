function imshownorm(depth, lims)

if exist('lims', 'var')
    imshow(mat2gray(depth, lims));
else
    imshow(mat2gray(depth));
end

end