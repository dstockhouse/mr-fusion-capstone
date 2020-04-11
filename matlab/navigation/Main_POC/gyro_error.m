function gyro = gyro_error(FB, BS, BI, PSD, sfs, m_g)
% Function Name: gyro_errors.m
% Description: Takes gyro error source terms as inputs and outputs them 
% into a gyro error source stuct 
    gyro.FB = FB;       % Bias - Fixed Bias (deg/hr)
    gyro.BS = BS;       % Bias - Bias Stability term 1-sigma (deg/hr)
    gyro.BI = BI;       % Bias - Bias Instability term 1-sigma (deg/hr)
    gyro.PSD = PSD;     % Gyro PSD (deg/hr)/rt_Hz
    gyro.sfs = sfs;     % Scale factor stability (ppm)
    gyro.m_g = m_g;     % Misalignment term (deg)
end