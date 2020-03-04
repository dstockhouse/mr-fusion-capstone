function fused = fuse_ir_depth(depth, ir, ir_threshold)

if ~exist('ir_threshold', 'var')
    ir_threshold = 120;
end

fused = depth .* ((ir > ir_threshold) & (depth < max(max(depth))));
    
end