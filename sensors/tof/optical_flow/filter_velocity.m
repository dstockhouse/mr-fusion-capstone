function [kai_loc_fil,transformations] = filter_velocity(constants,kai_est_old,est_cov,kai_est,image_level,accumulatedTransformation)
% Inputs

% Outputs


% Defaults
previous_speed_eig_weight = 0.5;
previous_speed_const_weight = 0.05;

Bii = eig(est_cov,'matrix');
eigenVals = eig(est_cov);

kai_loc = kai_est_old - trans2kai(accumulatedTransformation);
kai_b = Bii\kai_est;
kai_b_old = Bii\kai_loc;

cf = previous_speed_eig_weight * exp(-image_level);
df = previous_speed_const_weight * exp(-image_level);

kai_b_fil = zeros(6,1);
for i = 1:6
    kai_b_fil(i) = (kai_b(i)+(cf*eigenVals(i)+df)*kai_b_old(i))/(1+cf*eigenVals(i)+df);
end

kai_loc_fil = inv(Bii)\kai_b_fil;

transformations = kai2trans(kai_loc_fil/constants.fps);

end