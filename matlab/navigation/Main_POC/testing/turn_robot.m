function [P, V, A] = turn_robot(P, V, A, angle, in_place, constants)
% Function Name: turn_robot.m
% Description: 
% Inputs: P = Current Position Matrix
%         V = Current Velocity Matrix
%         A = Current Attitude Matrix
%         angle = angle in radians to turn right from (for left turn use 
%         negative number
%         in_place = boolean to determine fi turning in place or turning at
%         a curve
% Outputs: P = Position Matrix after Turn
%          V = Velocity Matrix after Turn
%          A = Attitude Matrix after Turn
if in_place
    % If turning in place
    omega = 1.8103;     % Angular rotation of robot based on 1 m/s from each wheel
    t = angle/omega;    % Time required for turn
    len = t/dt;         % Added length in indices for each matrix
    for i=1:len
        P(:,end+1) = P(:,end);  % For in place turn position does not change
        V(:,end+1) = [0 0 0];   % Linear velocity is 0 when turning in place
        A(:,end+1) = 
    end
    
else
    % If not turning in place
    
end
end