/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2019, Analog Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <aditof/camera.h>
#include <aditof/camera_96tof1_specifics.h>
#include <aditof/device_interface.h>
#include <aditof/frame.h>
#include <aditof/system.h>
#include <glog/logging.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#ifdef OPENCV2
#include <opencv2/contrib/contrib.hpp>
#endif

#include "../aditof_opencv.h"

using namespace aditof;

#define BUFFER_SIZE 4096
#define BUFFER_MOD(N) ((((N)%BUFFER_SIZE)>=0)?((N)%BUFFER_SIZE):(BUFFER_SIZE+((N)%BUFFER_SIZE)))

std::mutex buffer_mutex;
std::condition_variable buffer_condition;
int buffer_start, buffer_end;
std::vector<cv::Mat> depth_buffer;
std::vector<cv::Mat> ir_buffer;
std::vector<std::string> name_buffer;

bool threadAbort = false;

// void *imageSaveThread(void *) {
void imageSaveThread() {

	char filename[64];

	cv::Mat depth_temp;
	cv::Mat ir_temp;
	std::string name_temp;

	while (!threadAbort || BUFFER_MOD(buffer_end - buffer_start) > 0) {

		printf("store size %d\n", BUFFER_MOD(buffer_end - buffer_start));

		{
			std::unique_lock<std::mutex> guard(buffer_mutex);

			// Wait for data to be available in buffer
			buffer_condition.wait_for(guard, std::chrono::seconds(1), [] { return BUFFER_MOD(buffer_end - buffer_start) > 0; });

			if (BUFFER_MOD(buffer_end - buffer_start) > 0) {

				depth_temp = std::move(depth_buffer[buffer_start]);
				ir_temp = std::move(ir_buffer[buffer_start]);
				name_temp = std::move(name_buffer[buffer_start]);

				buffer_start = (buffer_start + 1) % BUFFER_SIZE;

			} else {
				threadAbort = true;
			}
		}

		// Save images
		snprintf(filename, 64, "%s_depth.png", name_temp.c_str());
		cv::imwrite(filename, depth_temp);
		snprintf(filename, 64, "%s_ir.png", name_temp.c_str());
		cv::imwrite(filename, ir_temp);

		printf("Saved %s\n", name_temp.c_str());

	}

	printf("Stopping save thread\n");

}

double ts2double(struct timespec &time) {
	return (double) time.tv_sec + ((double) time.tv_nsec)/1000000000;
}

double gettime(struct timespec *outtime) {
	struct timespec time;

	if (outtime == NULL) {
		outtime = &time;
	}
	clock_gettime(CLOCK_REALTIME, outtime);

	return ts2double(*outtime);
}

int main(int argc, char *argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = 1;

    Status status = Status::OK;

    // Initialize picture buffers
    depth_buffer.resize(BUFFER_SIZE);
    ir_buffer.resize(BUFFER_SIZE);
    name_buffer.resize(BUFFER_SIZE);

    System system;
    status = system.initialize();
    if (status != Status::OK) {
        LOG(ERROR) << "Could not initialize system!";
        return 0;
    }

    std::vector<std::shared_ptr<Camera>> cameras;
    system.getCameraList(cameras);
    if (cameras.empty()) {
        LOG(WARNING) << "No cameras found!";
        return 0;
    }

    auto camera = cameras.front();
    status = camera->initialize();
    if (status != Status::OK) {
        LOG(ERROR) << "Could not initialize camera!";
        return 0;
    }

    std::vector<std::string> frameTypes;
    camera->getAvailableFrameTypes(frameTypes);
    if (frameTypes.empty()) {
        LOG(ERROR) << "No frame type available!";
        return 0;
    }
    std::cout << "Camera types: ";
    for (int i = 0; i < frameTypes.size(); i++) {
	    std::cout << frameTypes[i] << " ";
    }
    std::cout << std::endl;


    status = camera->setFrameType(frameTypes.front());
    if (status != Status::OK) {
        LOG(ERROR) << "Could not set camera frame type!";
        return 0;
    }

    std::vector<std::string> modes;
    camera->getAvailableModes(modes);
    if (modes.empty()) {
        LOG(ERROR) << "No camera modes available!";
        return 0;
    }

    status = camera->setMode(modes[1]);
    if (status != Status::OK) {
        LOG(ERROR) << "Could not set camera mode!";
        return 0;
    }

    aditof::CameraDetails cameraDetails;
    camera->getDetails(cameraDetails);
    int cameraRange = cameraDetails.maxDepth;
    int irRange = 4096;
    double desired_fps = 8.0;

    aditof::Frame frame;

    const int smallSignalThreshold = 80;
    auto specifics = camera->getSpecifics();
    auto cam96tof1Specifics =
        std::dynamic_pointer_cast<Camera96Tof1Specifics>(specifics);
    cam96tof1Specifics->setNoiseReductionThreshold(smallSignalThreshold);
    cam96tof1Specifics->enableNoiseReduction(true);


    // Create runtime directory
    char dirname[128];
    snprintf(dirname, 128, "TOF_PNG_md%d_mi%d_st%d_%09d",
		    cameraRange, irRange, smallSignalThreshold,
		    (unsigned int) gettime(NULL));
    mkdir(dirname, 0777);
    chdir(dirname);

    printf("Saving images to '%s'\n", dirname);

    // Start imwrite thread
    std::thread saveThread(imageSaveThread);

    // cv::namedWindow("Depth Image", cv::WINDOW_AUTOSIZE);
    // cv::namedWindow("IR Image", cv::WINDOW_AUTOSIZE);

    // Set stdin nonblocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // Set noncanonical
    struct termios tty, tty_old;
    tcgetattr(STDIN_FILENO, &tty_old);
    tty = tty_old; // Save terminal settings to be restored
    tty.c_lflag &= ~(ECHO | ICANON);
    tty.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);

    double startTime = gettime(NULL);
    double trueStartTime = startTime;
    double newTime;

    int i = 0;
    int num_seconds = 5;
    int num_frames = 30 * num_seconds;
    char input = 0, rc = 0;
    // while (i < num_frames) {
    // while (startTime < (trueStartTime + num_seconds)) {
    printf("Press space to exit\n");
    while (input != ' ') {
    // while (cv::waitKey(1) != 27 &&
    //        getWindowProperty("Depth Image", cv::WND_PROP_AUTOSIZE) >= 0 &&
    //        getWindowProperty("IR Image", cv::WND_PROP_AUTOSIZE) >= 0) {

	i++;
	rc = read(STDIN_FILENO, &input, 1);

        /* Request frame from camera */
        status = camera->requestFrame(&frame);
        if (status != Status::OK) {
            LOG(ERROR) << "Could not request frame!";
            return 0;
        }

	struct timespec newTime_ts;
	newTime = gettime(&newTime_ts);

	double timestep = newTime - startTime;
	double elapsed = newTime - trueStartTime;

	startTime = newTime;

        /* Convert from frame to depth mat */
        cv::Mat depthmat;
        status = fromFrameToDepthMat(frame, depthmat);
        if (status != Status::OK) {
            LOG(ERROR) << "Could not convert from frame to depth mat!";
            return 0;
        }

        /* Convert from frame to ir mat */
        cv::Mat irmat;
        status = fromFrameToIrMat(frame, irmat);
        if (status != Status::OK) {
            LOG(ERROR) << "Could not convert from frame to IR mat!";
            return 0;
        }

        /* Distance factor */
        // double distance_scale = 255.0 / cameraRange;
        // double ir_scale = 255.0 / irRange;
        double distance_scale = 65535.0 / cameraRange;
        double ir_scale = 65535.0 / irRange;

        /* Convert from raw values to values that opencv can understand */
        depthmat.convertTo(depthmat, CV_16U, distance_scale);
        irmat.convertTo(irmat, CV_16U, ir_scale);


        /* Display the images */
        // imshow("Depth Image", depthmat);
        // imshow("IR Image", irmat);

	// char* ext;
	// ext = "png";

	// std::vector<int> saveParams{CV_IMWRITE_PNG_COMPRESSION, 3};

	// Save image to disk
	// char filename[64];
	// snprintf(filename, 64, "depth_%d.%s", i, ext);
	// cv::imwrite(filename, depthmat);
	// cv::imwrite(filename, depthmat, saveParams);
	// snprintf(filename, 64, "ir_%d.%s", i, ext);
	// cv::imwrite(filename, irmat);
	// cv::imwrite(filename, irmat, saveParams);

	// Put images into ring buffers to be saved by other thread
	char filename[64];
	if (BUFFER_MOD(buffer_end - buffer_start) < BUFFER_SIZE - 1) {

		printf("capture size %d\n", BUFFER_MOD(buffer_end - buffer_start));

		snprintf(filename, 64, "im%05d_%08.3f", i, elapsed);

		{
			std::lock_guard<std::mutex> guard(buffer_mutex);

			depth_buffer[buffer_end] = depthmat;
			ir_buffer[buffer_end] = irmat;
			name_buffer[buffer_end] = filename;
			buffer_end = (buffer_end + 1) % BUFFER_SIZE;
		}

		buffer_condition.notify_one();

	} else {
		printf("    ERROR dropped frame %d\n", i);
	}

	printf("Captured frame %d @ %.2f fps (avg %.2f) [%s]\n",
			i, 1/(timestep), i/(newTime - trueStartTime), filename);

	newTime = gettime(&newTime_ts);
	elapsed = newTime - trueStartTime;

	// Determine time to wait
	double delay_time = i/desired_fps - elapsed;
	if (delay_time > 0.0) {
		struct timespec delay_ts;
		delay_ts.tv_sec = 0;
		delay_ts.tv_nsec = (int)(delay_time * 1000000000);
		nanosleep(&delay_ts, NULL);
	}
    }

    threadAbort = true;

    fcntl(STDIN_FILENO, F_SETFL, flags);
    tcsetattr(STDIN_FILENO, TCSANOW, &tty_old);

    printf("Saved to directory %s\n\n", dirname);

    saveThread.join();

    return 0;
}
