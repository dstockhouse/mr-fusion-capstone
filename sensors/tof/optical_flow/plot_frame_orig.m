function plot_frame_orig(C, O, label, color)

hp = hggroup;           % Create a group object and the handle to that parent

% Compute new axis vectors
nx = C * [1; 0; 0];
ny = C * [0; 1; 0];
nz = C * [0; 0; 1];

% X vector
line([O(1), O(1) + nx(1)], [O(2), O(2) + nx(2)], [O(3), O(3) + nx(3)], 'Color', color, 'LineWidth',1,'Parent',hp);
text(nx(1) + O(1), nx(2) + O(2), nx(3) + O(3), ['X_{' label '}'] , 'Color', color,'Parent',hp);

line([O(1), O(1) + ny(1)], [O(2), O(2) + ny(2)], [O(3), O(3) + ny(3)], 'Color', color, 'LineWidth',1,'Parent',hp);
text(ny(1) + O(1), ny(2) + O(2), ny(3) + O(3), ['Y_{' label '}'] , 'Color', color,'Parent',hp);

line([O(1), O(1) + nz(1)], [O(2), O(2) + nz(2)], [O(3), O(3) + nz(3)], 'Color', color, 'LineWidth',1,'Parent',hp);
text(nz(1) + O(1), nz(2) + O(2), nz(3) + O(3), ['Z_{' label '}'] , 'Color', color,'Parent',hp);

text(O(1), O(2), O(3), label, 'Color', color,'Parent',hp);

end
