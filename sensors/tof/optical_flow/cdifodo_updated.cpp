
#include <CDifodo.h>
#include <mrpt/utils/utils_defs.h>
#include <mrpt/utils/CTicTac.h>
#include <mrpt/utils/round.h>

using namespace mrpt;
using namespace mrpt::math;
using namespace std;
using namespace Eigen;
using mrpt::utils::round;
using mrpt::utils::square;

CDifodo::CDifodo()
{
	rows = 60;
	cols = 80;
	fovh = M_PI*58.6/180.0;
	fovv = M_PI*45.6/180.0;
	cam_mode = 1;			// (1 - 640 x 480, 2 - 320 x 240, 4 - 160 x 120)
	downsample = 1;
	ctf_levels = 1;
	width = 640/(cam_mode*downsample);
	height = 480/(cam_mode*downsample);
	fast_pyramid = true;
	indices.resize(2, rows*cols);

	//Resize pyramid
    const unsigned int pyr_levels = round(log(float(width/cols))/log(2.f)) + ctf_levels;
	transformations.resize(pyr_levels);

	for (unsigned int i = 0; i<pyr_levels; i++)
    {
        unsigned int s = pow(2.f,int(i));
        cols_i = width/s; rows_i = height/s;
        depth[i].resize(rows_i, cols_i);
        depth_inter[i].resize(rows_i, cols_i);
        depth_old[i].resize(rows_i, cols_i);
        depth[i].assign(0.0f);
        depth_old[i].assign(0.0f);
        xx[i].resize(rows_i, cols_i);
        xx_inter[i].resize(rows_i, cols_i);
        xx_old[i].resize(rows_i, cols_i);
        xx[i].assign(0.0f);
        xx_old[i].assign(0.0f);
        yy[i].resize(rows_i, cols_i);
        yy_inter[i].resize(rows_i, cols_i);
        yy_old[i].resize(rows_i, cols_i);
        yy[i].assign(0.0f);
        yy_old[i].assign(0.0f);
		transformations[i].resize(4,4);

		if (cols_i <= cols)
		{
			depth_warped[i].resize(rows_i,cols_i);
			xx_warped[i].resize(rows_i,cols_i);
			yy_warped[i].resize(rows_i,cols_i);
		}
    }

	depth_wf.setSize(height,width);
	dt.resize(rows,cols);
	du.resize(rows,cols);
	dv.resize(rows,cols);

	previous_speed_const_weight = 0.05f;
	previous_speed_eig_weight = 0.5f;
	kai_loc_old.assign(0.f);
	num_valid_points = 0;

	//Compute gaussian mask
	VectorXf v_mask(4);
	v_mask(0) = 1.f; v_mask(1) = 2.f; v_mask(2) = 2.f; v_mask(3) = 1.f;
	for (unsigned int i = 0; i<4; i++)
		for (unsigned int j = 0; j<4; j++)
			f_mask(i,j) = v_mask(i)*v_mask(j)/36.f;

	//Compute gaussian mask
	float v_mask2[5] = {1,4,6,4,1};
	for (unsigned int i = 0; i<5; i++)
		for (unsigned int j = 0; j<5; j++)
			g_mask[i][j] = v_mask2[i]*v_mask2[j]/256.f;
}

void CDifodo::buildCoordinatesPyramid()
{
	const float max_depth_dif = 0.1f;

	//Push coordinates back
	for (unsigned int k=0; k<6; k++)
	{
		depth_old[k].swap(depth[k]);
		xx_old[k].swap(xx[k]);
		yy_old[k].swap(yy[k]);
	}

	//The number of levels of the pyramid does not match the number of levels used
	//in the odometry computation (because we might want to finish with lower resolutions)

	unsigned int pyr_levels = round(log(float(width/cols))/log(2.f)) + ctf_levels;

	//Generate levels
	for (unsigned int i = 0; i<pyr_levels; i++)
	{
		unsigned int s = pow(2.f,int(i));
		cols_i = width/s;
		rows_i = height/s;
		const int rows_i2 = 2*rows_i;
		const int cols_i2 = 2*cols_i;
		const int i_1 = i-1;

		if (i == 0)
			depth[i].swap(depth_wf);

		//                              Downsampling
		//-----------------------------------------------------------------------------
		else
		{
			for (unsigned int u = 0; u != cols_i; u++)
			for (unsigned int v = 0; v != rows_i; v++)
			{
				const int u2 = 2*u;
				const int v2 = 2*v;
				const float dcenter = depth[i_1](v2,u2);

				//Inner pixels
				if ((v>0)&&(v<rows_i-1)&&(u>0)&&(u<cols_i-1))
				{
					if (dcenter != 0.f)
					{
						float sum = 0.f;
						float weight = 0.f;

						for (int l = -2; l<3; l++)
						for (int k = -2; k<3; k++)
						{
							const float abs_dif = abs(depth[i_1](v2+k,u2+l)-dcenter);
							if (abs_dif < max_depth_dif)
							{
								const float aux_w = g_mask[2+k][2+l]*(max_depth_dif - abs_dif);
								weight += aux_w;
								sum += aux_w*depth[i_1](v2+k,u2+l);
							}
						}
						depth[i](v,u) = sum/weight;
					}
					else
					{
						float min_depth = 10.f;
						for (int l = -2; l<3; l++)
						for (int k = -2; k<3; k++)
						{
							const float d = depth[i_1](v2+k,u2+l);
							if ((d > 0.f)&&(d < min_depth))
								min_depth = d;
						}

						if (min_depth < 10.f)
							depth[i](v,u) = min_depth;
						else
							depth[i](v,u) = 0.f;
					}
				}

				//Boundary
				else
				{
					if (dcenter != 0.f)
					{
						float sum = 0.f;
						float weight = 0.f;

						for (int l = -2; l<3; l++)
						for (int k = -2; k<3; k++)
						{
							const int indv = v2+k,indu = u2+l;
							if ((indv>=0)&&(indv<rows_i2)&&(indu>=0)&&(indu<cols_i2))
							{
								const float abs_dif = abs(depth[i_1](indv,indu)-dcenter);
								if (abs_dif < max_depth_dif)
								{
									const float aux_w = g_mask[2+k][2+l]*(max_depth_dif - abs_dif);
									weight += aux_w;
									sum += aux_w*depth[i_1](indv,indu);
								}
							}
						}
						depth[i](v,u) = sum/weight;
					}
					else
					{
						float min_depth = 10.f;
						for (int l = -2; l<3; l++)
						for (int k = -2; k<3; k++)
						{
							const int indv = v2+k,indu = u2+l;
							if ((indv>=0)&&(indv<rows_i2)&&(indu>=0)&&(indu<cols_i2))
							{
								const float d = depth[i_1](indv,indu);
								if ((d > 0.f)&&(d < min_depth))
									min_depth = d;
							}
						}

						if (min_depth < 10.f)
							depth[i](v,u) = min_depth;
						else
							depth[i](v,u) = 0.f;
					}
				}
			}
		}

		//Calculate coordinates "xy" of the points
		const float inv_f_i = 2.f*tan(0.5f*fovh)/float(cols_i);
		const float disp_u_i = 0.5f*(cols_i-1);
		const float disp_v_i = 0.5f*(rows_i-1);

		for (unsigned int u = 0; u != cols_i; u++)
			for (unsigned int v = 0; v != rows_i; v++)
			if (depth[i](v,u) != 0.f)
			{
				xx[i](v,u) = (u - disp_u_i)*depth[i](v,u)*inv_f_i;
				yy[i](v,u) = (v - disp_v_i)*depth[i](v,u)*inv_f_i;
			}
			else
			{
				xx[i](v,u) = 0.f;
				yy[i](v,u) = 0.f;
			}
	}
}

void CDifodo::buildCoordinatesPyramidFast()
{
	const float max_depth_dif = 0.1f;
	
	//Push coordinates back
	for (unsigned int k=0; k<6; k++)
	{
		depth_old[k].swap(depth[k]);
		xx_old[k].swap(xx[k]);
		yy_old[k].swap(yy[k]);
	}
	//depth_old.swap(depth);
	//xx_old.swap(xx);
	//yy_old.swap(yy);

	//The number of levels of the pyramid does not match the number of levels used
	//in the odometry computation (because we might want to finish with lower resolutions)

	unsigned int pyr_levels = round(log(float(width/cols))/log(2.f)) + ctf_levels;

	//Generate levels
	for (unsigned int i = 0; i<pyr_levels; i++)
	{
		unsigned int s = pow(2.f,int(i));
		cols_i = width/s;
		rows_i = height/s;
		const int i_1 = i-1;

		//Refs
		const MatrixXf &depth_prev_ref = depth[i_1];
		MatrixXf &depth_ref = depth[i];
		MatrixXf &xx_ref = xx[i];
		MatrixXf &yy_ref = yy[i];


		if (i == 0)
			depth_ref.swap(depth_wf);

		//                              Downsampling
		//-----------------------------------------------------------------------------
		else
		{
			for (unsigned int u = 0; u != cols_i; u++)
				for (unsigned int v = 0; v != rows_i; v++)
				{
					const int u2 = 2*u;
					const int v2 = 2*v;
					
					//Inner pixels
					if ((v!=0)&&(v!=rows_i-1)&&(u!=0)&&(u!=cols_i-1))
					{
						const Matrix4f d_block = depth_prev_ref.block<4,4>(v2-1,u2-1);
						float depths[4] = {d_block(5),d_block(6),d_block(9),d_block(10)};

						//Find the "second maximum" value of the central block
						if (depths[1] < depths[0]) {std::swap(depths[1], depths[0]);}
						if (depths[3] < depths[2]) {std::swap(depths[3], depths[2]);}
						const float dcenter = (depths[3] < depths[1]) ? max(depths[3], depths[0]) : max(depths[1], depths[2]);
						
						if (dcenter != 0.f)
						{	
							//Faster for 32 bits
							float sum = 0.f, weight = 0.f;
							for (unsigned char k = 0; k!=16; k++)
							{
								const float abs_dif = abs(d_block(k) - dcenter);
								if (abs_dif < max_depth_dif)
								{
									const float aux_w = f_mask(k)*(max_depth_dif - abs_dif);
									weight += aux_w;
									sum += aux_w*d_block(k);
								}
							}
							depth_ref(v,u) = sum/weight;

							//Faster for 64 bits (I guess the compilers vectorizes more)
							//const Array44f abs_mat = (d_block.array() - dcenter).abs();
							//const Array44f aux_w = f_mask.array()*(max_depth_dif - abs_mat).max(0.f);
							//const Array44f depth_w = aux_w*d_block.array();
							//depth_ref(v,u) = depth_w.sum()/aux_w.sum();
						}
						else
							depth_ref(v,u) = 0.f;
                    }

                    //Boundary
					else
					{
						const Matrix2f d_block = depth_prev_ref.block<2,2>(v2,u2);

						float new_d = 0.f;
						unsigned int cont = 0;
                        for (unsigned int k=0; k!=4;k++)
							if (d_block(k) != 0.f)
							{
								new_d += d_block(k);
								cont++;
							}

                        if (cont != 0)	depth_ref(v,u) = new_d/float(cont);
                        else		    depth_ref(v,u) = 0.f;
					}
				}
        }

        //Calculate coordinates "xy" of the points
		const float inv_f_i = 2.f*tan(0.5f*fovh)/float(cols_i);
        const float disp_u_i = 0.5f*float(cols_i-1);
        const float disp_v_i = 0.5f*float(rows_i-1);

   //     for (unsigned int u = 0; u != cols_i; u++) 
			//for (unsigned int v = 0; v != rows_i; v++)
   //             if (depth_ref(v,u) != 0.f)
			//	{
			//		xx_ref(v,u) = (u - disp_u_i)*depth_ref(v,u)*inv_f_i;
			//		yy_ref(v,u) = (v - disp_v_i)*depth_ref(v,u)*inv_f_i;
			//	}
			//	else
			//	{
			//		xx_ref(v,u) = 0.f;
			//		yy_ref(v,u) = 0.f;
			//	}		


		const ArrayXf v_col = ArrayXf::LinSpaced(rows_i, 0.f, float(rows_i-1));
		for (unsigned int u = 0; u != cols_i; u++)
		{
			yy_ref.col(u) = (inv_f_i*(v_col - disp_v_i)*depth_ref.col(u).array()).matrix();
			xx_ref.col(u) = inv_f_i*(float(u) - disp_u_i)*depth_ref.col(u);
		}


    }
}

void CDifodo::buildCoordinatesPyramidInteger()
{
	const int max_depth_dif = 100;

	//Integer mask
	VectorXi v_mask(4);
	v_mask(0) = 1; v_mask(1) = 2; v_mask(2) = 2; v_mask(3) = 1;
	Matrix4i int_mask;
	for (unsigned int i = 0; i<4; i++)
		for (unsigned int j = 0; j<4; j++)
			int_mask(i,j) = v_mask(i)*v_mask(j);
	
	//Push coordinates back
	for (unsigned int k=0; k<6; k++)
	{
		depth_old[k].swap(depth[k]);
		xx_old[k].swap(xx[k]);
		yy_old[k].swap(yy[k]);
	}
	//depth_old.swap(depth);
	//xx_old.swap(xx);
	//yy_old.swap(yy);

	//The number of levels of the pyramid does not match the number of levels used
	//in the odometry computation (because we might want to finish with lower resolutions)

	unsigned int pyr_levels = round(log(float(width/cols))/log(2.f)) + ctf_levels;

	//Generate levels
	for (unsigned int i = 0; i<pyr_levels; i++)
	{
		unsigned int s = pow(2.f,int(i));
		cols_i = width/s;
		rows_i = height/s;
		const int i_1 = i-1;

		//Refs
		const MatrixXf &depth_prev_ref = depth[i_1];
		MatrixXf &depth_ref = depth[i];
		MatrixXf &xx_ref = xx[i];
		MatrixXf &yy_ref = yy[i];


		if (i == 0)
			depth_ref.swap(depth_wf);

		//                              Downsampling
		//-----------------------------------------------------------------------------
		else
		{
			for (unsigned int u = 0; u != cols_i; u++)
				for (unsigned int v = 0; v != rows_i; v++)
				{
					const int u2 = 2*u;
					const int v2 = 2*v;
					
					//Inner pixels
					if ((v!=0)&&(v!=rows_i-1)&&(u!=0)&&(u!=cols_i-1))
					{
						const Matrix4i d_block = (1000.f*depth_prev_ref.block<4,4>(v2-1,u2-1)).cast<int>();
						int depths[4] = {d_block(5),d_block(6),d_block(9),d_block(10)};

						//Find the "second maximum" value of the central block
						if (depths[1] < depths[0]) {std::swap(depths[1], depths[0]);}
						if (depths[3] < depths[2]) {std::swap(depths[3], depths[2]);}
						const int dcenter = (depths[3] < depths[1]) ? max(depths[3], depths[0]) : max(depths[1], depths[2]);
						
						if (dcenter != 0)
						{	
							int sum = 0, weight = 0;

							for (unsigned char k = 0; k!=16; k++)
								{
									const int abs_dif = abs(d_block(k) - dcenter);
									if (abs_dif < max_depth_dif)
									{
										const int aux_w = int_mask(k)*(max_depth_dif - abs_dif);
										weight += aux_w;
										sum += aux_w*d_block(k);
									}
								}
							depth_ref(v,u) = 0.001f*float(sum/weight);
						}
						else
							depth_ref(v,u) = 0.f;
                    }

                    //Boundary
					else
					{
						const Matrix2i d_block = (1000.f*depth_prev_ref.block<2,2>(v2,u2)).cast<int>();

						int new_d = 0.f;
						unsigned int cont = 0;
                        for (unsigned int k=0; k!=4;k++)
							if (d_block(k) != 0)
							{
								new_d += d_block(k);
								cont++;
							}

                        if (cont != 0)	depth_ref(v,u) = 0.001f*float(new_d/cont);
                        else		    depth_ref(v,u) = 0.f;

						//const float new_d = 0.25f*d_block.sumAll();
						//if (new_d < 0.4f)
						//	depth_ref(v,u) = 0.f;
						//else
						//	depth_ref(v,u) = new_d;
					}
				}
        }

        //Calculate coordinates "xy" of the points
		const float inv_f_i = 2.f*tan(0.5f*fovh)/float(cols_i);
        const float disp_u_i = 0.5f*(cols_i-1);
        const float disp_v_i = 0.5f*(rows_i-1);

        for (unsigned int u = 0; u != cols_i; u++) 
			for (unsigned int v = 0; v != rows_i; v++)
                if (depth_ref(v,u) != 0.f)
				{
					xx_ref(v,u) = (u - disp_u_i)*depth_ref(v,u)*inv_f_i;
					yy_ref(v,u) = (v - disp_v_i)*depth_ref(v,u)*inv_f_i;
				}
				else
				{
					xx_ref(v,u) = 0.f;
					yy_ref(v,u) = 0.f;
				}
    }
}

//void CDifodo::performWarping()
//{
//	//Camera parameters (which also depend on the level resolution)
//	const float f = float(cols_i)/(2.f*tan(0.5f*fovh));
//	const float disp_u_i = 0.5f*float(cols_i-1);
//    const float disp_v_i = 0.5f*float(rows_i-1);
//
//	//Refs
//	MatrixXf &depth_warped_ref = depth_warped[image_level];
//	MatrixXf &xx_warped_ref = xx_warped[image_level];
//	MatrixXf &yy_warped_ref = yy_warped[image_level];
//
//	//Rigid transformation estimated up to the present level
//	Matrix4f acu_trans; 
//	acu_trans.setIdentity();
//	for (unsigned int i=1; i<=level; i++)
//		acu_trans = transformations[i-1]*acu_trans;
//
//	MatrixXf wacu(rows_i,cols_i);
//	wacu.assign(0.f);
//	depth_warped[image_level].assign(0.f);
//
//	const float cols_lim = float(cols_i-1);
//	const float rows_lim = float(rows_i-1);
//
//	//						Warping loop
//	//---------------------------------------------------------
//	for (unsigned int j = 0; j!=cols_i; j++)
//		for (unsigned int i = 0; i!=rows_i; i++)
//		{		
//			const float z = depth[image_level](i,j);
//			
//			if (z != 0.f)
//			{
//				//Transform point to the warped reference frame
//				const float depth_w = acu_trans(0,0)*z + acu_trans(0,1)*xx[image_level](i,j) + acu_trans(0,2)*yy[image_level](i,j) + acu_trans(0,3);
//				const float x_w = acu_trans(1,0)*z + acu_trans(1,1)*xx[image_level](i,j) + acu_trans(1,2)*yy[image_level](i,j) + acu_trans(1,3);
//				const float y_w = acu_trans(2,0)*z + acu_trans(2,1)*xx[image_level](i,j) + acu_trans(2,2)*yy[image_level](i,j) + acu_trans(2,3);
//
//				//Calculate warping
//				const float uwarp = f*x_w/depth_w + disp_u_i;
//				const float vwarp = f*y_w/depth_w + disp_v_i;
//
//				//The warped pixel (which is not integer in general) contributes to all the surrounding ones
//				if (( uwarp >= 0.f)&&( uwarp < cols_lim)&&( vwarp >= 0.f)&&( vwarp < rows_lim))
//				{
//					const int uwarp_l = uwarp;
//					const int uwarp_r = uwarp_l + 1;
//					const int vwarp_d = vwarp;
//					const int vwarp_u = vwarp_d + 1;
//					const float delta_r = float(uwarp_r) - uwarp;
//					const float delta_l = uwarp - float(uwarp_l);
//					const float delta_u = float(vwarp_u) - vwarp;
//					const float delta_d = vwarp - float(vwarp_d);
//
//					//Warped pixel very close to an integer value
//					if (abs(round(uwarp) - uwarp) + abs(round(vwarp) - vwarp) < 0.05f)
//					{
//						depth_warped_ref(round(vwarp), round(uwarp)) += depth_w;
//						wacu(round(vwarp), round(uwarp)) += 1.f;
//					}
//					else
//					{
//						const float w_ur = square(delta_l) + square(delta_d);
//						depth_warped_ref(vwarp_u,uwarp_r) += w_ur*depth_w;
//						wacu(vwarp_u,uwarp_r) += w_ur;
//
//						const float w_ul = square(delta_r) + square(delta_d);
//						depth_warped_ref(vwarp_u,uwarp_l) += w_ul*depth_w;
//						wacu(vwarp_u,uwarp_l) += w_ul;
//
//						const float w_dr = square(delta_l) + square(delta_u);
//						depth_warped_ref(vwarp_d,uwarp_r) += w_dr*depth_w;
//						wacu(vwarp_d,uwarp_r) += w_dr;
//
//						const float w_dl = square(delta_r) + square(delta_u);
//						depth_warped_ref(vwarp_d,uwarp_l) += w_dl*depth_w;
//						wacu(vwarp_d,uwarp_l) += w_dl;
//					}
//				}
//			}
//		}
//
//	//Scale the averaged depth and compute spatial coordinates
//    const float inv_f_i = 1.f/f;
//	for (unsigned int u = 0; u!=cols_i; u++)
//		for (unsigned int v = 0; v!=rows_i; v++)
//		{	
//			if (wacu(v,u) > 0.f)
//			{
//				depth_warped_ref(v,u) /= wacu(v,u);
//				xx_warped_ref(v,u) = (u - disp_u_i)*depth_warped_ref(v,u)*inv_f_i;
//				yy_warped_ref(v,u) = (v - disp_v_i)*depth_warped_ref(v,u)*inv_f_i;
//			}
//			else
//			{
//				depth_warped_ref(v,u) = 0.f;
//				xx_warped_ref(v,u) = 0.f;
//				yy_warped_ref(v,u) = 0.f;
//			}
//		}
//}

void CDifodo::performWarping()
{
	//Camera parameters (which also depend on the level resolution)
	const float f = float(cols_i)/(2.f*tan(0.5f*fovh));
	const float disp_u_i = 0.5f*float(cols_i-1);
	const float disp_v_i = 0.5f*float(rows_i-1);

	//Refs
	MatrixXf &depth_warped_ref = depth_warped[image_level];
	MatrixXf &xx_warped_ref = xx_warped[image_level];
	MatrixXf &yy_warped_ref = yy_warped[image_level];
	const MatrixXf &depth_ref = depth[image_level];
	const MatrixXf &xx_ref = xx[image_level];
	const MatrixXf &yy_ref = yy[image_level];
	depth_warped_ref.assign(0.f);

	//Rigid transformation estimated up to the present level
	Matrix4f acu_trans; acu_trans.setIdentity();
	for (unsigned int i=0; i<=level; i++)
		acu_trans = transformations[i]*acu_trans;

	//Aux variables
	MatrixXf wacu(rows_i,cols_i); wacu.assign(0.f);
	const int cols_lim = 100*(cols_i-1);
	const int rows_lim = 100*(rows_i-1);

	// Warping loop
	//---------------------------------------------------------
	for (unsigned int j = 0; j<cols_i; j++)
		for (unsigned int i = 0; i<rows_i; i++)
		{ 
			const float z = depth_ref(i,j);
			if (z != 0.f)
			{
				//Transform point to the warped reference frame
				const float depth_w = acu_trans(0)*z + acu_trans(4)*xx_ref(i,j) + acu_trans(8)*yy_ref(i,j) + acu_trans(12);
				const float x_w = acu_trans(1)*z + acu_trans(5)*xx_ref(i,j) + acu_trans(9)*yy_ref(i,j) + acu_trans(13);
				const float y_w = acu_trans(2)*z + acu_trans(6)*xx_ref(i,j) + acu_trans(10)*yy_ref(i,j) + acu_trans(14);

				//Calculate warping
				const int uwarp = int(100.f*(f*x_w/depth_w + disp_u_i));
				const int vwarp = int(100.f*(f*y_w/depth_w + disp_v_i));

				//The projection after transforming is not integer in general and, hence, the pixel contributes to all the surrounding ones
				if (( uwarp >= 0)&&( uwarp < cols_lim)&&( vwarp >= 0)&&( vwarp < rows_lim))
				{
					const int uwarp_l = uwarp - uwarp%100;
					const int uwarp_r = uwarp_l + 100;
					const int vwarp_d = vwarp - vwarp%100;
					const int vwarp_u = vwarp_d + 100;
					const int delta_r = uwarp_r - uwarp;
					const int delta_l = uwarp - uwarp_l;
					const int delta_u = vwarp_u - vwarp;
					const int delta_d = vwarp - vwarp_d;

					//Warped pixel very close to an integer value
					if (min(delta_r, delta_l) + min(delta_u, delta_d) < 5)
					{
						const int ind_u = delta_r > delta_l ? uwarp_l/100 : uwarp_r/100;
						const int ind_v = delta_u > delta_d ? vwarp_d/100 : vwarp_u/100;

						depth_warped_ref(ind_v,ind_u) += depth_w;
						wacu(ind_v,ind_u) += 1.f;
					}
					else
					{
						const int v_u = vwarp_u/100, v_d = vwarp_d/100;
						const int u_r = uwarp_r/100, u_l = uwarp_l/100;
						const float w_ur = square(delta_l) + square(delta_d);
						depth_warped_ref(v_u,u_r) += w_ur*depth_w;
						wacu(v_u,u_r) += w_ur;

						const float w_ul = square(delta_r) + square(delta_d);
						depth_warped_ref(v_u,u_l) += w_ul*depth_w;
						wacu(v_u,u_l) += w_ul;

						const float w_dr = square(delta_l) + square(delta_u);
						depth_warped_ref(v_d,u_r) += w_dr*depth_w;
						wacu(v_d,u_r) += w_dr;

						const float w_dl = square(delta_r) + square(delta_u);
						depth_warped_ref(v_d,u_l) += w_dl*depth_w;
						wacu(v_d,u_l) += w_dl;
					}
				}
			}
		}

	//Scale the averaged depth and compute spatial coordinates
	const float inv_f_i = 1.f/f;
	for (unsigned int u = 0; u!=cols_i; u++)
		for (unsigned int v = 0; v!=rows_i; v++)
		{ 
			if (wacu(v,u) != 0.f)
			{
				depth_warped_ref(v,u) /= wacu(v,u);
				xx_warped_ref(v,u) = (u - disp_u_i)*depth_warped_ref(v,u)*inv_f_i;
				yy_warped_ref(v,u) = (v - disp_v_i)*depth_warped_ref(v,u)*inv_f_i;
			}
			else
			{
				xx_warped_ref(v,u) = 0.f;
				yy_warped_ref(v,u) = 0.f;
			}
		}

	//This is not faster with 32 bits (requires setting wacu to something bigger than 0 as well)
	//const float inv_f_i = 1.f/f;
	//const ArrayXf v_col = ArrayXf::LinSpaced(rows_i, 0.f, float(rows_i-1));
	//depth_warped_ref = (depth_warped_ref.array()/wacu.array()).matrix();
	//for (unsigned int u = 0; u != cols_i; u++)
	//{
	//	//depth_warped_ref.col(u) = (depth_warped_ref.col(u).array()/wacu.col(u).array()).matrix();
	//	yy_warped_ref.col(u) = (inv_f_i*(v_col - disp_v_i)*depth_warped_ref.col(u).array()).matrix();
	//	xx_warped_ref.col(u) = inv_f_i*(float(u) - disp_u_i)*depth_warped_ref.col(u);
	//}
}

void CDifodo::calculateCoord()
{	
	num_valid_points = 0;

	//Refs
	MatrixXf &depth_inter_ref = depth_inter[image_level];
	MatrixXf &xx_inter_ref = xx_inter[image_level];
	MatrixXf &yy_inter_ref = yy_inter[image_level];
	const MatrixXf &depth_old_ref = depth_old[image_level];
	const MatrixXf &depth_warped_ref = depth_warped[image_level];

	const int multiplier = max(1, int(cols_i)/int(max_res_sparsity));
	
	for (unsigned int u = 0; u != cols_i; u++)
		for (unsigned int v = 0; v != rows_i; v++)
		{
			if ((depth_old_ref(v,u) != 0.f) && (depth_warped_ref(v,u) != 0.f))
			{
				depth_inter_ref(v,u) = 0.5f*(depth_old_ref(v,u) + depth_warped_ref(v,u));
				xx_inter_ref(v,u) = 0.5f*(xx_old[image_level](v,u) + xx_warped[image_level](v,u));
				yy_inter_ref(v,u) = 0.5f*(yy_old[image_level](v,u) + yy_warped[image_level](v,u));

				if ((u!=0)&&(v!=0)&&(u!=cols_i-1)&&(v!=rows_i-1)&(u%multiplier == 0)&&(v%multiplier == 0))
				{
					indices(0, num_valid_points) = v;
					indices(1, num_valid_points) = u;
					num_valid_points++;
				}
			}
			else
			{
				depth_inter_ref(v,u) = 0.f;
				xx_inter_ref(v,u) = 0.f;
				yy_inter_ref(v,u) = 0.f;
			}
		}

	//printf("\n number of valid points = %d", num_valid_points);
}

//void CDifodo::calculateDepthDerivatives()
//{
//	dt.resize(rows_i,cols_i); dt.assign(0.f);
//	du.resize(rows_i,cols_i); du.assign(0.f);
//	dv.resize(rows_i,cols_i); dv.assign(0.f);
//
//    //Compute connectivity
//	MatrixXf rx_ninv(rows_i,cols_i);
//	MatrixXf ry_ninv(rows_i,cols_i);
//    rx_ninv.assign(1.f); ry_ninv.assign(1.f);
//
//	//Refs
//	const MatrixXf &depth_inter_ref = depth_inter[image_level];
//	const MatrixXf &xx_inter_ref = xx_inter[image_level];
//	const MatrixXf &yy_inter_ref = yy_inter[image_level];
//
//	for (unsigned int u = 0; u < cols_i-1; u++)
//        for (unsigned int v = 0; v < rows_i; v++)
//			if (null(v,u) == false)
//			{
//				rx_ninv(v,u) = sqrtf(square(xx_inter_ref(v,u+1) - xx_inter_ref(v,u))
//									+ square(depth_inter_ref(v,u+1) - depth_inter_ref(v,u)));
//			}
//
//	for (unsigned int u = 0; u < cols_i; u++)
//        for (unsigned int v = 0; v < rows_i-1; v++)
//			if (null(v,u) == false)
//			{
//				ry_ninv(v,u) = sqrtf(square(yy_inter_ref(v+1,u) - yy_inter_ref(v,u))
//									+ square(depth_inter_ref(v+1,u) - depth_inter_ref(v,u)));
//			}
//
//
//    //Spatial derivatives
//    for (unsigned int v = 0; v < rows_i; v++)
//    {
//        for (unsigned int u = 1; u < cols_i-1; u++)
//			if (null(v,u) == false)
//				du(v,u) = (rx_ninv(v,u-1)*(depth_inter_ref(v,u+1)-depth_inter_ref(v,u)) + rx_ninv(v,u)*(depth_inter_ref(v,u) - depth_inter_ref(v,u-1)))/(rx_ninv(v,u)+rx_ninv(v,u-1));
//
//		du(v,0) = du(v,1);
//		du(v,cols_i-1) = du(v,cols_i-2);
//    }
//
//    for (unsigned int u = 0; u < cols_i; u++)
//    {
//        for (unsigned int v = 1; v < rows_i-1; v++)
//			if (null(v,u) == false)
//				dv(v,u) = (ry_ninv(v-1,u)*(depth_inter_ref(v+1,u)-depth_inter_ref(v,u)) + ry_ninv(v,u)*(depth_inter_ref(v,u) - depth_inter_ref(v-1,u)))/(ry_ninv(v,u)+ry_ninv(v-1,u));
//
//		dv(0,u) = dv(1,u);
//		dv(rows_i-1,u) = dv(rows_i-2,u);
//    }
//
//	//Temporal derivative
//	for (unsigned int u = 0; u < cols_i; u++)
//		for (unsigned int v = 0; v < rows_i; v++)
//			if (null(v,u) == false)
//				dt(v,u) = depth_warped[image_level](v,u) - depth_old[image_level](v,u);
//}

void CDifodo::calculateDepthDerivatives()
{
	dt.assign(0.f);
	du.assign(0.f);
	dv.assign(0.f);
	
	//Compute weights for the gradients
	MatrixXf rx(rows_i,cols_i), ry(rows_i,cols_i);
    rx.fill(1.f); ry.fill(1.f);
	const float epsilon_depth = 0.005f;

	//Refs
	const MatrixXf &depth_ref = depth_inter[image_level];
	const MatrixXf &xx_ref = xx_inter[image_level];
	const MatrixXf &yy_ref = yy_inter[image_level];

	//Weights for forward and backward derivatives
	//----------------------------------------------------------------------------------------------------
	rx.block(0,0,rows_i,cols_i-1) = ((depth_ref.block(0,1,rows_i,cols_i-1) - depth_ref.block(0,0,rows_i,cols_i-1)).array() + epsilon_depth).matrix();
	ry.block(0,0,rows_i-1,cols_i) = ((depth_ref.block(1,0,rows_i-1,cols_i) - depth_ref.block(0,0,rows_i-1,cols_i)).array() + epsilon_depth).matrix();	

	//for (unsigned int k=0; k<num_valid_points; k++)
	//{
	//	const int v = indices(0,k);
	//	const int u = indices(1,k);
	//	rx(v,u) = abs(depth_ref(v,u+1) - depth_ref(v,u)) + epsilon_depth;
	//	ry(v,u) = abs(depth_ref(v+1,u) - depth_ref(v,u)) + epsilon_depth;
	//}

   // for (unsigned int u = 0; u != cols_i-1; u++)
   //     for (unsigned int v = 0; v != rows_i-1; v++)
   //         if (depth_ref(v,u) != 0.f)
			//{
			//	rx(v,u) = abs(depth_ref(v,u+1) - depth_ref(v,u)) + epsilon_depth;
			//	ry(v,u) = abs(depth_ref(v+1,u) - depth_ref(v,u)) + epsilon_depth;
			//}		


    //Spatial derivatives
	//--------------------------------------------------------------------------------------------------------
	//for (unsigned int k=0; k<num_valid_points; k++)
	//{
	//	const int v = indices(0,k);
	//	const int u = indices(1,k);
	//	du(v,u) = (rx(v,u-1)*(depth_ref(v,u+1)-depth_ref(v,u)) + rx(v,u)*(depth_ref(v,u) - depth_ref(v,u-1)))/(rx(v,u)+rx(v,u-1));
	//	dv(v,u) = (ry(v-1,u)*(depth_ref(v+1,u)-depth_ref(v,u)) + ry(v,u)*(depth_ref(v,u) - depth_ref(v-1,u)))/(ry(v,u)+ry(v-1,u));
	//}


 //   for (unsigned int v = 0; v != rows_i; v++)
 //       for (unsigned int u = 1; u != cols_i-1; u++)
 //           if (depth_ref(v,u) != 0.f)
 //               du(v,u) = (rx(v,u-1)*(depth_ref(v,u+1)-depth_ref(v,u)) + rx(v,u)*(depth_ref(v,u) - depth_ref(v,u-1)))/(rx(v,u)+rx(v,u-1));

	//du.col(0) = du.col(1);
	//du.col(cols_i-1) = du.col(cols_i-2);

 //   for (unsigned int u = 0; u != cols_i; u++)
 //       for (unsigned int v = 1; v != rows_i-1; v++)
 //           if (depth_ref(v,u) != 0.f)
 //               dv(v,u) = (ry(v-1,u)*(depth_ref(v+1,u)-depth_ref(v,u)) + ry(v,u)*(depth_ref(v,u) - depth_ref(v-1,u)))/(ry(v,u)+ry(v-1,u));

 //   dv.row(0) = dv.row(1);
 //   dv.row(rows_i-1) = dv.row(rows_i-2);

	for (unsigned int u = 1; u != cols_i-1; u++)
        for (unsigned int v = 1; v != rows_i-1; v++)
            if (depth_ref(v,u) != 0.f)
			{
                dv(v,u) = (ry(v-1,u)*(depth_ref(v+1,u)-depth_ref(v,u)) + ry(v,u)*(depth_ref(v,u) - depth_ref(v-1,u)))/(ry(v,u)+ry(v-1,u));
				du(v,u) = (rx(v,u-1)*(depth_ref(v,u+1)-depth_ref(v,u)) + rx(v,u)*(depth_ref(v,u) - depth_ref(v,u-1)))/(rx(v,u)+rx(v,u-1));
			}

	du.col(0) = du.col(1); du.col(cols_i-1) = du.col(cols_i-2); du.row(0) = du.row(1); du.row(rows_i-1) = du.row(rows_i-2);
	dv.row(0) = dv.row(1); dv.row(rows_i-1) = dv.row(rows_i-2); dv.col(0) = dv.col(1); dv.col(cols_i-1) = dv.col(cols_i-2);

	//Temporal derivative
	//-------------------------------------------------------------------------------------------------
    dt = depth_warped[image_level] - depth_old[image_level];
}

void CDifodo::computeWeights()
{
    weights.resize(rows_i, cols_i);
    weights.assign(0.f);

	//Refs
	const MatrixXf &depth_inter_ref = depth_inter[image_level];
	const MatrixXf &depth_old_ref = depth_old[image_level];
	const MatrixXf &depth_warped_ref = depth_warped[image_level];


    //Parameters for error_measurement and error_linearization 
    const float kz2 = 0.01f;
    const float kduvt = 200.f;
    const float k2dt = 5.f;
    const float k2duv = 5.f;

	for (unsigned int k=0; k<num_valid_points; k++)
	{
		const int v = indices(0,k);
		const int u = indices(1,k);

        //					Compute error_measurement
        //-----------------------------------------------------------------------
        const float error_m = kz2*square(square(depth_inter_ref(v,u)));


        //					Compute error_linearization
        //-----------------------------------------------------------------------
        const float ini_du = depth_old_ref(v,u+1) - depth_old_ref(v,u-1);
        const float ini_dv = depth_old_ref(v+1,u) - depth_old_ref(v-1,u);
        const float final_du = depth_warped_ref(v,u+1) - depth_warped_ref(v,u-1);
        const float final_dv = depth_warped_ref(v+1,u) - depth_warped_ref(v-1,u);

        const float dut = ini_du - final_du;
        const float dvt = ini_dv - final_dv;
        const float duu = du(v,u+1) - du(v,u-1);
        const float dvv = dv(v+1,u) - dv(v-1,u);
        const float dvu = dv(v,u+1) - dv(v,u-1);

        const float error_l = kduvt*(square(du(v,u)) + square(dv(v,u)) + square(dt(v,u))) + k2dt*(square(dut) + square(dvt))
                                + k2duv*(square(duu) + square(dvv) + square(dvu));

        weights(v,u) = sqrt(1.f/(error_m + error_l));
    }

    const float inv_max = 1.f/weights.maximum();
    weights = inv_max*weights;
}

void CDifodo::solveOneLevel()
{
	Matrix<float, Dynamic, 6> A(num_valid_points,6);
	VectorXf B(num_valid_points);
	unsigned int cont = 0;

	//Refs
	const MatrixXf &depth_inter_ref = depth_inter[image_level];
	const MatrixXf &xx_inter_ref = xx_inter[image_level];
	const MatrixXf &yy_inter_ref = yy_inter[image_level];

	//Fill the matrix A and the vector B
	//The order of the unknowns is (vz, vx, vy, wz, wx, wy)
	//The points order will be (1,1), (1,2)...(1,cols-1), (2,1), (2,2)...(row-1,cols-1).

	const float f_inv = float(cols_i)/(2.f*tan(0.5f*fovh));

	for (unsigned int k=0; k<num_valid_points; k++)
	{
		const int v = indices(0,k);
		const int u = indices(1,k);

		// Precomputed expressions
		const float d = depth_inter_ref(v,u);
		const float inv_d = 1.f/d;
		const float x = xx_inter_ref(v,u);
		const float y = yy_inter_ref(v,u);
		const float dycomp = du(v,u)*f_inv*inv_d;
		const float dzcomp = dv(v,u)*f_inv*inv_d;
		const float tw = weights(v,u);

		//Fill the matrix A
		A(cont, 0) = tw*(1.f + dycomp*x*inv_d + dzcomp*y*inv_d);
		A(cont, 1) = tw*(-dycomp);
		A(cont, 2) = tw*(-dzcomp);
		A(cont, 3) = tw*(dycomp*y - dzcomp*x);
		A(cont, 4) = tw*(y + dycomp*inv_d*y*x + dzcomp*(y*y*inv_d + d));
		A(cont, 5) = tw*(-x - dycomp*(x*x*inv_d + d) - dzcomp*inv_d*y*x);
		B(cont) = tw*(-dt(v,u));

		cont++;
			}
	
	//Solve the linear system of equations using weighted least squares
	MatrixXf AtA, AtB;
	AtA.multiply_AtA(A);
	AtB.multiply_AtB(A,B);
	const Matrix<float, 6, 1> Var = AtA.ldlt().solve(AtB);

	//Covariance matrix calculation 
	VectorXf res = -B;
	for (unsigned int k = 0; k<6; k++)
		res += Var(k)*A.col(k);
	//const VectorXf res = A*Var - B;

	est_cov = (1.f/float(num_valid_points-6))*AtA.inverse()*res.squaredNorm();

	//Update last velocity in local coordinates
	kai_loc_level = Var;
}

void CDifodo::odometryCalculation()
{
	//Clock to measure the runtime
	utils::CTicTac clock;
	clock.Tic();

	//Build the gaussian pyramid
	if (fast_pyramid)	buildCoordinatesPyramidFast();
	else				buildCoordinatesPyramid();

	//const float tpyr = clock.Tac();
	//printf("\n Pyramid time = %f", 1000.f*tpyr);

	//Coarse-to-fines scheme
    for (unsigned int i=0; i<ctf_levels; i++)
	{
		//Initialize transformations
		transformations[i].setIdentity();

		for (unsigned int k=0; k<4; k++)
		{
			level = i;
			unsigned int s = pow(2.f,int(ctf_levels-(i+1)));
			cols_i = cols/s; rows_i = rows/s;
			image_level = ctf_levels - i + round(log(float(width/cols))/log(2.f)) - 1;

			//1. Perform warping
			if ((i == 0)&&(k == 0))
			{
				depth_warped[image_level] = depth[image_level];
				xx_warped[image_level] = xx[image_level];
				yy_warped[image_level] = yy[image_level];
			}
			else
				performWarping();

			//2. Calculate inter coords and find null measurements
			calculateCoord();

			//3. Compute derivatives
			calculateDepthDerivatives();

			//4. Compute weights
			computeWeights();

			//5. Solve odometry
			if (num_valid_points > 6)
				solveOneLevel();

			//6. Filter solution
			filterLevelSolution();

			//Check convergence of nonlinear iterations
			if (kai_loc_level.norm() < 0.004f) //0.01
			{
				//printf("\n level = %d, Iterations = %d", i, k+1);
				break;
			}
			//else if (k == 2)
			//	printf("\n level = %d, Iterations = %d", i, k+1);
		}
	}

	//Update poses
	poseUpdate();

	//Save runtime
	execution_time = 1000.f*clock.Tac();   
}

void CDifodo::filterLevelSolution()
{
	//		Calculate Eigenvalues and Eigenvectors
	//----------------------------------------------------------
	SelfAdjointEigenSolver<MatrixXf> eigensolver(est_cov);
	if (eigensolver.info() != Success) 
	{ 
		printf("\n Eigensolver couldn't find a solution. Pose is not updated");
		return;
	}
	
	//First, we have to describe both the new linear and angular velocities in the "eigenvector" basis
	//-------------------------------------------------------------------------------------------------
	Matrix<float,6,6> Bii;
	Matrix<float,6,1> kai_b;
	Bii = eigensolver.eigenvectors();
	kai_b = Bii.colPivHouseholderQr().solve(kai_loc_level);

	//Second, we have to describe both the old linear and angular velocities in the "eigenvector" basis
	//-------------------------------------------------------------------------------------------------
	Matrix<float,6,1> kai_loc_sub = kai_loc_old;

	//Important: we have to substract the previous levels' solutions from the old velocity.
	Matrix4f acu_trans;
	acu_trans.setIdentity();
	for (unsigned int i=0; i<=level; i++)
		acu_trans = transformations[i]*acu_trans;

	Matrix4f log_trans = acu_trans.log();
	kai_loc_sub(0) -= log_trans(0,3); kai_loc_sub(1) -= log_trans(1,3); kai_loc_sub(2) -= log_trans(2,3);
	kai_loc_sub(3) += log_trans(1,2); kai_loc_sub(4) -= log_trans(0,2); kai_loc_sub(5) += log_trans(0,1);

	//Transform that local representation to the "eigenvector" basis
	Matrix<float,6,1> kai_b_old;
	kai_b_old = Bii.colPivHouseholderQr().solve(kai_loc_sub);

	//									Filter velocity
	//--------------------------------------------------------------------------------
	const float cf = previous_speed_eig_weight*expf(-int(level)), df = previous_speed_const_weight*expf(-int(level));
	Matrix<float,6,1> kai_b_fil;
	for (unsigned int i=0; i<6; i++)
		kai_b_fil(i) = (kai_b(i) + (cf*eigensolver.eigenvalues()(i,0) + df)*kai_b_old(i))/(1.f + cf*eigensolver.eigenvalues()(i) + df);

	//Transform filtered velocity to the local reference frame 
	Matrix<float, 6, 1> kai_loc_fil = Bii.inverse().colPivHouseholderQr().solve(kai_b_fil);

	//Compute the rigid transformation
	Matrix4f local_mat; local_mat.assign(0.f); 
	local_mat(0,1) = -kai_loc_fil(5); local_mat(1,0) = kai_loc_fil(5);
	local_mat(0,2) = kai_loc_fil(4); local_mat(2,0) = -kai_loc_fil(4);
	local_mat(1,2) = -kai_loc_fil(3); local_mat(2,1) = kai_loc_fil(3);
	local_mat(0,3) = kai_loc_fil(0);
	local_mat(1,3) = kai_loc_fil(1);
	local_mat(2,3) = kai_loc_fil(2);
	transformations[level] = local_mat.exp()*transformations[level];
}

void CDifodo::poseUpdate()
{
	//First, compute the overall transformation
	//---------------------------------------------------
	Matrix4f acu_trans;
	acu_trans.setIdentity();
	for (unsigned int i=1; i<=ctf_levels; i++)
		acu_trans = transformations[i-1]*acu_trans;


	//Compute the new estimates in the local and absolutes reference frames
	//---------------------------------------------------------------------
	Matrix<float, 4, 4> log_trans = acu_trans.log();
	kai_loc(0) = log_trans(0,3); kai_loc(1) = log_trans(1,3); kai_loc(2) = log_trans(2,3);
	kai_loc(3) = -log_trans(1,2); kai_loc(4) = log_trans(0,2); kai_loc(5) = -log_trans(0,1);

	CMatrixDouble33 inv_trans;
	CMatrixFloat31 v_abs, w_abs;

	cam_pose.getRotationMatrix(inv_trans);
	v_abs = inv_trans.cast<float>()*kai_loc.topRows(3);
	w_abs = inv_trans.cast<float>()*kai_loc.bottomRows(3);
	kai_abs.topRows<3>() = v_abs;
	kai_abs.bottomRows<3>() = w_abs;	


	//						Update poses
	//-------------------------------------------------------	
	cam_oldpose = cam_pose;
	CMatrixDouble44 aux_acu = acu_trans;
	poses::CPose3D pose_aux(aux_acu);
	cam_pose = cam_pose + pose_aux;


	//Compute the velocity estimate in the new ref frame (to be used by the filter in the next iteration)
	//---------------------------------------------------------------------------------------------------
	cam_pose.getRotationMatrix(inv_trans);
	kai_loc_old.topRows<3>() = inv_trans.inverse().cast<float>()*kai_abs.topRows(3);
	kai_loc_old.bottomRows<3>() = inv_trans.inverse().cast<float>()*kai_abs.bottomRows(3);
}

void CDifodo::setFOV(float new_fovh, float new_fovv)
{
	fovh = M_PI*new_fovh/180.0;
	fovv = M_PI*new_fovv/180.0;
}

void CDifodo::getPointsCoord(MatrixXf &x, MatrixXf &y, MatrixXf &z)
{
	x.resize(rows,cols);
	y.resize(rows,cols);
	z.resize(rows,cols);

	z = depth_inter[0];
	x = xx_inter[0];
	y = yy_inter[0];
}

void CDifodo::getDepthDerivatives(MatrixXf &cur_du, MatrixXf &cur_dv, MatrixXf &cur_dt)
{
	cur_du.resize(rows,cols);
	cur_dv.resize(rows,cols);
	cur_dt.resize(rows,cols);

	cur_du = du;
	cur_dv = dv;
	cur_dt = dt;
}

void CDifodo::getWeights(MatrixXf &w)
{
	w.resize(rows,cols);
	w = weights;
}


