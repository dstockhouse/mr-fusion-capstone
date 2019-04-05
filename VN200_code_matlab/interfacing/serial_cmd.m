function [] = serial_cmd(s, cmd)

%   Compute the 8-bit checksum (i.e., XOR) of the command bytes
checksum = uint8(cmd(1));           % Convert to type unsigned 8-bit integer

for i = 2:length(cmd)
    checksum = bitxor(checksum, uint8(cmd(i)), 'uint8');
end
checksum = dec2hex(checksum, 2);    % Convert to type ASCII - Must have 2-bytes

% Issue the cmd with checksum
str = sprintf('$%s*%s\n', cmd, checksum);
fprintf(s, str);

end     % End function serial_cmd