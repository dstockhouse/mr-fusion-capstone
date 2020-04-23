

#include <CDifodo.h>
#include <mrpt/utils/CConfigFileBase.h>
#include <mrpt/utils/CImage.h>
#include <mrpt/utils/CTicTac.h>
#include <mrpt/utils/round.h>
#include <mrpt/opengl/COpenGLScene.h>
#include <mrpt/gui/CDisplayWindow3D.h>
#include <iostream>
#include <OpenNI.h>
#include "legend.xpm"


class CDifodoCamera : public CDifodo {
public:

	mrpt::gui::CDisplayWindow3D	window;
	std::ofstream		f_res;

	bool save_results;

	/** Constructor. */
	CDifodoCamera() : CDifodo()
	{
		save_results = 0;
	}

	/** Initialize the visual odometry method */
	void loadConfiguration( const mrpt::utils::CConfigFileBase &ini );

	/** Open camera */
	bool openCamera();

	/** Close camera */
	void closeCamera();

	/** Capture a new depth frame */
	void loadFrame();

	/** Create a file to save the estimated trajectory */
	void CreateResultsFile();

	/** Initialize opengl scene */
	void initializeScene();

	/** Refresh the opengl scene */
	void updateScene();

	/** A pre-step that should be performed before starting to estimate the camera velocity.
	  * It can also be called to reset the estimated trajectory and pose */
	void reset();

	/** Save the pose estimation following the format of the TUM datasets:
	  * 
	  * 'timestamp tx ty tz qx qy qz qw'
	  *
	  * Please visit http://vision.in.tum.de/data/datasets/rgbd-dataset/file_formats for further details.*/  
	void writeTrajectoryFile();

private:
	
	unsigned int repr_level;

	mrpt::opengl::COpenGLScenePtr	scene;	//!< Opengl scene

	// OpenNI variables to manage the camera
	openni::Status		rc;
	openni::Device		device;
	openni::VideoMode	video_options;
	openni::VideoStream depth_ch;

	/** Clock used to save the timestamp */
	mrpt::utils::CTicTac clock;
};
