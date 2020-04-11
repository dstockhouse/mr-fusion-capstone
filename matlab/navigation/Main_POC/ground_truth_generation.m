function [w_b__i_b, f_b__i_b, r_t__t_b, v_t__t_b, C_t__b, C_t__bm_out] = ground_truth_generation(t,first_run_flag,C_t__bm_in, constants)
% Function Name: ground_truth_generation.m
% Description: Generates Ground truth parameters for simulation 

    if first_run_flag == 1    % First run of function has special parameters
        [r_t__t_b, v_t__t_b, a_t__t_b, C_t__b, w_t__tb, C_t__bm_out] = body_in_tan(t, 1,eye(3), constants);
    else
        [r_t__t_b, v_t__t_b, a_t__t_b, C_t__b, w_t__tb, C_t__bm_out] = body_in_tan(t, 0,C_t__bm_in, constants);
    end

        [r_e__e_b, v_e__e_b, a_e__e_b, C_e__b, w_e__eb] = tan_to_ecef(r_t__t_b, v_t__t_b, a_t__t_b, C_t__b, w_t__tb, constants);

        [w_b__i_b, f_b__i_b] = ecef_to_eci(t, r_e__e_b, v_e__e_b, a_e__e_b, C_e__b, w_e__eb, constants); 
    
end %End Function 