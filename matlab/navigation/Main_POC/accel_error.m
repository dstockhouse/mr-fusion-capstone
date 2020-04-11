function accel = accel_error(FB, BS, BI, PSD, sfs, m_a)
% Function Name: accel_error.m
% Description: Takes accel error source terms as inputs and outputs them 
% into an accel error source stuct 
    accel.FB = FB;      % Bias - Fixed Bias term (mg)
    accel.BS = BS;      % Bias - Bias Stability term 1-sigma (mg)
    accel.BI = BI;      % Bias - Bias Instability term 1-sigma (mg)
    accel.PSD = PSD;    % Accel PSD (mg/rt-Hz) 
    accel.sfs = sfs;    % Scale factor stability (ppm)
    accel.m_a = m_a;    % Misalignment term (deg)
end