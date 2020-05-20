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
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

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

#define GUIDANCE_IP	"192.168.1.3"
#define GUIDANCE_PORT	31403

int generateFilename(char *buf, int bufSize, time_t *filetime, 
		const char *dir, const char *pre, unsigned suf, const char *ext) {

	// Length of filename generated
	int charsWritten;

	struct timespec randseed;
	// Different time source
	// clock_gettime(CLOCK_MONOTONIC_RAW, &randseed);
	clock_gettime(CLOCK_MONOTONIC, &randseed);
	srand(randseed.tv_sec + randseed.tv_nsec);

	// Time variables
	struct tm currentTime;
	time_t ltime;
	if(filetime == NULL) {
		ltime = time(NULL);
		filetime = &ltime;
	}

	// Get current time in UTC
	localtime_r(filetime, &currentTime);

	// Create filename using date/time and input string
	charsWritten = snprintf(buf, bufSize, 
			"%s/%s-%02d.%02d.%04d_%02d-%02d-%02d_%08x.%s",
			dir, pre,
			currentTime.tm_mon + 1,
			currentTime.tm_mday,
			currentTime.tm_year + 1900,
			currentTime.tm_hour,
			currentTime.tm_min,
			currentTime.tm_sec,
			suf,
			ext);

	// Return length of the new string
	return charsWritten;

} // generateFilename(char *, int, time_t, char *, char *, char *)

/**** Function TCPClientInit ****
*
* Opens and initializes a TCP socket as a client. Creates a socket but does
* not attempt to connect to server.
*
* Arguments: 
*      None
*
* Return value:
*      On success, returns file descriptor corresponding to opened socket
*      On failure, prints error message and returns a negative number 
*/
int TCPClientInit() {

	int sock_fd, rc;

	// Create bare socket
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1) {
		printf( "%s: Failed to create client socket\n", strerror(errno));
		return sock_fd;
	}

	// Configure socket options to allow reusing addresses
	int socketOption = 1;
	rc = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &socketOption, sizeof(int));
	if (rc != 0) {
		printf( "Unable to set socket option SO_REUSEADDR: %s\n", strerror(errno));
	}
	rc = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &socketOption, sizeof(int));
	if (rc != 0) {
		printf( "Unable to set socket option SO_REUSEPORT: %s\n", strerror(errno));
	}

	// Return file descriptor for socket
	return sock_fd;

} // TCPClientInit()


/**** Function TCPClientTryConnect ****
 *
 * Attempts to establish a connection to the server at ipAddr and port. If the 
 * connection cannot be made (i.e. the server is not ready), it returns 
 * immediately instead of blocking.
 *
 * Arguments: 
 *      sock_fd - File descriptor of open client socket
 *      ipAddr  - String containing the IPv4 address "XXX.XXX.XXX.XXX"
 *      port    - Integer port number to establish connection
 *
 * Return value:
 *      Returns value returned by connect, which set errno appropriately
 */
int TCPClientTryConnect(int sock_fd, char *ipAddr, int port) {

	int rc;
	struct sockaddr_in socketAddress;

	// Exit on error if invalid pointer
	if (ipAddr == NULL) {
		return -1;
	}

	// Configure socket address
	// AF_INET = Address Family InterNet
	socketAddress.sin_family = AF_INET;

	// Convert port number to network byte order
	// htons = host to network byte order (short)
	socketAddress.sin_port = htons(port);

	// Convert IP address string into address variable
	// pton = string to network address
	rc = inet_pton(AF_INET, ipAddr, &(socketAddress.sin_addr));
	if (rc == -1) {
		printf( "Failed to convert IP address for TCP client (%s): %s\n", 
				ipAddr, strerror(errno));
		return rc;
	}

	// Attempt to connect to the server at ipAddr and port
	rc = connect(sock_fd, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
	if (rc == 0) {

		// Make socket nonblocking after connecting
		// Do this here instead of at the init function so that if it doesn't try to
		// exit early during connect
		rc = fcntl(sock_fd, F_SETFL, O_NONBLOCK);
		if (rc == -1) {
			printf( "%s: Failed to set client socket to nonblocking (not fatal)\n",
					strerror(errno));
		}

	}

	// Return file descriptor for socket
	return rc;

} // TCPClientTryConnect(int, char *, int)


/**** Function TCPServerInit ****
 *
 * Opens and initializes a TCP socket for server operation. Creates a socket,
 * binds it to the port and address, and sets it to listen.
 *
 * Arguments: 
 *      ipAddr - String containing the IPv4 address "XXX.XXX.XXX.XXX"
 *      port   - Integer port number to listen for connection
 *
 * Return value:
 *      On success, returns file descriptor corresponding to opened listening socket
 *      On failure, prints error message and returns a negative number 
 */
int TCPServerInit(char *ipAddr, int port) {

	int sock_fd, rc;
	struct sockaddr_in socketAddress;

	// Exit on error if invalid pointer
	if (ipAddr == NULL) {
		return -1;
	}

	// Create bare socket
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1) {
		printf( "%s: Failed to create server socket\n", strerror(errno));
		return sock_fd;
	}

	// Configure socket options to allow reusing addresses
	int socketOption = 1;
	rc = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &socketOption, sizeof(int));
	if (rc != 0) {
		printf( "Unable to set socket option SO_REUSEADDR: %s\n", strerror(errno));
	}
	rc = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &socketOption, sizeof(int));
	if (rc != 0) {
		printf( "Unable to set socket option SO_REUSEPORT: %s\n", strerror(errno));
	}

	// Configure socket address
	// AF_INET = Address Family InterNet
	socketAddress.sin_family = AF_INET;

	// Convert port number to network byte order
	// htons = host to network byte order (short)
	socketAddress.sin_port = htons(port);

	// Convert IP address string into address variable
	// pton = string to network address
	rc = inet_pton(AF_INET, ipAddr, &(socketAddress.sin_addr));
	if (rc == -1) {
		printf( "%s: Failed to convert IP address for TCP server (%s)\n", 
				ipAddr, strerror(errno));
		return rc;
	}

	// Bind to an incoming port to listen for client connections
	rc = bind(sock_fd, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
	if (rc == -1) {
		printf( "%s: Failed to bind server socket to address\n", strerror(errno));
		return rc;
	}

	// Listen for incoming connections (only allow one queued connection)
	rc = listen(sock_fd, 1);
	if (rc == -1) {
		printf( "%s: Failed to listen on server socket\n", strerror(errno));
		return rc;
	}

	// Return file descriptor for socket
	return sock_fd;

} // TCPServerInit(char *, int)


/**** Function TCPServerTryAccept ****
 *
 * Attempts to accept a waiting connection on a listening socket. Once a
 * connection has been accepted, it closes the original socket fd.
 *
 * For server sockets, this should be called after *Init() and before *TryAccept().
 * For client sockets, this should be called after *Init() and after *TryConnect().
 *
 * Arguments: 
 *      sock_fd - File descriptor for open and listening TCP server socket
 *
 * Return value:
 *      Returns the value returned by accept(3), with errno set appropriately.
 */
int TCPClose(int sock_fd);
int TCPServerTryAccept(int sock_fd) {

	struct sockaddr_in socketAddress;
	int newsock_fd;

	// Accept an incoming connection
	socklen_t sockAddressLength = sizeof(socketAddress);
	newsock_fd = accept(sock_fd, (struct sockaddr*)&socketAddress, &sockAddressLength);

	// If successful, close server (only one connection per server)
	if (newsock_fd != -1) {
		TCPClose(sock_fd);
	}

	// Return file descriptor for socket
	return newsock_fd;

} // TCPServerTryAccept(int)


/**** Function TCPSetNonBlocking ****
 *
 * Sets an open socket to non-blocking
 *
 * Arguments: 
 *      sock_fd - File descriptor for open TCP socket
 *
 * Return value:
 *      Returns value returned by fcntl, which set errno appropriately
 */
int TCPSetNonBlocking(int sock_fd) {

	int rc;

	// Set nonblocking and directly return fcntl return status
	rc = fcntl(sock_fd, F_SETFL, O_NONBLOCK);

	return rc;

} // TCPSetNonBlocking(int)


/**** Function TCPRead ****
 *
 * Reads from an open and initialized socket file descriptor.
 *
 * Arguments: 
 *      sock_fd - File descriptor for open and initialized TCP socket
 *      buf     - Buffer to store data that is read
 *      length  - Length of room left in the buffer
 *
 * Return value:
 *      Returns number of characters read (may be 0)
 *      On failure, prints error message and returns a negative number 
 */
int TCPRead(int sock_fd, unsigned char *buf, int length) {

	int numRead;

	// Exit on error if invalid pointer
	if (buf == NULL) {
		return -1;
	}

	// Ensure non-blocking read
	// Done at initialization, but placed here for assurance when misbehaving
	// if (!(fcntl(sock_fd, F_GETFL) & O_NONBLOCK)) {
	//     fcntl(sock_fd, F_SETFL, O_NONBLOCK);
	// }

	// Attempt to receive a message from TCP socket at most length bytes (nonblocking)
	numRead = recv(sock_fd, buf, length, MSG_DONTWAIT);
	printf( "TCPRead: received %d chars\n", numRead);
	if (numRead < 0) {
		if (errno == EAGAIN) {
			// Tell the application that no data was received
			numRead = 0;
		} else {
			printf( "%s: TCPRead recv() failed for TCP socket\n", strerror(errno));
		}
	}

	// Return number of bytes successfully read into buffer
	return numRead;

} // TCPRead(int, unsigned char *, int)


/**** Function TCPWrite ****
 *
 * Sends to an open and initialized socket file descriptor.
 *
 * Arguments: 
 *      sock_fd - File descriptor for open and initialized TCP socket
 *      buf     - Buffer containing data to write
 *      length  - Number of characters to read
 *
 * Return value:
 *      Returns number of characters written
 *      On failure, returns a negative number 
 */
int TCPWrite(int sock_fd, unsigned char *buf, int length) {

	int numWritten;

	// Exit on error if invalid pointer
	if (buf == NULL) {
		return -1;
	}

	// Attempt to write to TCP socket length bytes
	// Flags:
	//      Nonblocking
	//      Don't generate a SIGPIPE signal if the connection is broken
	numWritten = send(sock_fd, buf, length, MSG_DONTWAIT | MSG_NOSIGNAL);
	if (numWritten < 0) {
		printf( "%s: TCPWrite send() failed for TCP socket\n", strerror(errno));
	}

	// Return number of bytes successfully read into buffer
	return numWritten;

} // TCPWrite(int, unsigned char *, int)


/**** Function TCPClose ****
 *
 * Closes the file descriptor for a TCP socket
 *
 * Arguments: 
 *      sock_fd - File descriptor for the socket to close
 *
 * Return value:
 *      On success, returns 0
 *      On failure, returns a negative number 
 */
int TCPClose(int sock_fd) {

	int rc;

	rc = close(sock_fd);
	if (rc == -1) {
		printf( "%s: TCPClose Couldn't close socket with fd = %d\n", strerror(errno), sock_fd);
	}

	return rc;

} // TCPClose(int)


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

	int i, rc;

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


	// Remote run configuration parameters
	double programStartTime;
	unsigned programKey;

	int interactiveMode = 0;
	int gSock;
	if (!interactiveMode) {

		// Now set up TCP Connection with guidance
		printf("\n\n  Attempting to connect to guidance at %s:%d\n", GUIDANCE_IP, GUIDANCE_PORT);
		gSock = TCPClientInit();
		if (gSock == -1) {
			printf("  Failed to initialize socket: %s\n", strerror(errno));
		}

		// Loop until all sockets are connected
		const int MAX_CONNECT_SECONDS = 120;
		printf("Waiting for at most two minutes...\n");
		int gConnected = 0, numTries = 0; 
		double connectStartTime = gettime(NULL);
		double connectEndTime = gettime(NULL);
		while (!gConnected && connectEndTime < (connectStartTime + MAX_CONNECT_SECONDS)) {

			// Count attempt number
			numTries++;

			// Attempt to connect to guidance (if not already connected)
			if (!gConnected) {

				rc = TCPClientTryConnect(gSock, GUIDANCE_IP, GUIDANCE_PORT);
				if (rc != -1) {
					printf( "Successful TCP connection to guidance\n");
					gConnected = 1;
				} else if (errno == ECONNREFUSED) {
					// printf("Unsuccessful connection to guidance, will try again\n");
					// Delay to give other end a chance to start
					usleep(100000);
				} else {
					printf( "Could not connect to guidance: %s\n", strerror(errno));
				}
			}

		} // while (!connected && tries remaining)    // Loop until all sockets are connected

		if (!gConnected) {
			printf("Timeout for connection to guidance. Exiting\n");
			return -1;
		}

		// Wait to receive start time
		unsigned char tcpBuf[512] = {0};
		int numReceived = 0, messageReceived = 0;

		const double MAX_TCP_WAIT = 30.0;
		double waitStartTime, waitEndTime;

		waitStartTime = gettime(NULL);
		do {

			/*
			// Determine if data available
			struct pollfd pollSock;
			pollSock.fd = gSock;
			pollSock.events = POLLIN;
			rc = poll(&pollSock, 1, 10);
			if (rc > 0) {
				if (pollSock.revents | POLLIN) {
					// printf( "Data available on socket\n");
				}
			} else if (rc < 0) {
				printf( "Poll failed: %s\n",
						strerror(errno));
			}
			*/

			rc = TCPRead(gSock, &(tcpBuf[numReceived]), 512 - numReceived);
			usleep(100000);
			if (rc < 0) {
				if (errno == EAGAIN) {
					printf( "No TCP data available: %s\n", strerror(errno));
					usleep(100000);
				} else {
					printf( "Failed to read from TCP: %s\n", strerror(errno));
				}
			} else {
				numReceived += rc;

				if (rc > 0) {
					for (i = 0; i < numReceived; i++) {
						if (i < 4) {
							printf( "%c", tcpBuf[i]);
						} else {
							printf( " %02x", tcpBuf[i]);
						}
					}
					printf( "\n");
				}

				if (numReceived >= 16) {

					// See if the message is present in the bytes
					for (i = 0; i <= numReceived - 16; i++) {

						if (strncmp((char *) &(tcpBuf[i]), "init", 4) == 0) {
							// Since it starts with "init", this is the message we want
							memcpy(&programStartTime, &(tcpBuf[i+4]), 8);
							memcpy(&programKey, &(tcpBuf[i+12]), 4);

							// Set system clock based on received start time
							time_t startTimeSeconds = (time_t) programStartTime;
							stime(&startTimeSeconds);

							messageReceived = 1;
						}
					}
				}
			}
			waitEndTime = gettime(NULL);
			printf("%0.1lf\r", waitEndTime - waitStartTime);
		} while (!messageReceived && numReceived < 512 && ((waitEndTime - waitStartTime) < MAX_TCP_WAIT));

		if (!messageReceived) {
			printf( "\nNever received starting conditions from guidance, using defaults\n");

		} else {
			printf( "\nInitial conditions received from guidance in %0.3lf seconds\n",
					waitEndTime - waitStartTime);
		}
	} else {

		programStartTime = gettime(NULL);
		printf("Seeding RNG with %d\n", (unsigned) programStartTime);
		srand((unsigned) round(programStartTime));
		programKey = rand();
	}

	printf("\n  time = %.61lf; key = %08x\n", programStartTime, programKey);

    // Create runtime directory
    char dirname[1024], localdirname[512];
    snprintf(localdirname, 512, "TOF_PNG_md%d_mi%d_st%d_%09d_%08x",
		    cameraRange, irRange, smallSignalThreshold,
		    (unsigned int) programStartTime, programKey);
    time_t startTimeSeconds = (time_t) programStartTime;
    generateFilename(dirname, 1024, &startTimeSeconds, "log", "MRFUSION_RUN",
		    programKey, "d");
    strcat(dirname, localdirname);
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
    double trueStartTime = programStartTime;
    double newTime;

    int numReceived = 0;
    i = 0;
    rc = 0;
    int num_seconds = 5;
    int num_frames = 30 * num_seconds;
    char input = 0;
    // while (i < num_frames) {
    // while (startTime < (trueStartTime + num_seconds)) {
    printf("Press space to exit\n");
    int loopContinue = 1;
    while (loopContinue) {
    // while (cv::waitKey(1) != 27 &&
    //        getWindowProperty("Depth Image", cv::WND_PROP_AUTOSIZE) >= 0 &&
    //        getWindowProperty("IR Image", cv::WND_PROP_AUTOSIZE) >= 0) {

	i++;

	if (interactiveMode) {
		rc = read(STDIN_FILENO, &input, 1);
		if (input != ' ') {
			loopContinue = 0;
		}
	}

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

	if (!interactiveMode) {
		// Check for TCP commands
		char tcpBuf[512];
		rc = TCPRead(gSock, (unsigned char *) &(tcpBuf[numReceived]), 512 - numReceived);

		if (rc < 0) {
			printf( "Navigation: Failed to read from TCP: %s\n", strerror(errno));
		} else if (rc > 0) {
			numReceived += rc;

			if (numReceived >= 4) {
				// See if the message is present in the bytes
				int i;
				for (i = 0; i <= numReceived - 4; i++) {

					if (strncmp((char *) &(tcpBuf[i]), "stop", 4) == 0) {
						// Stop operation
						printf( "Navigation: Received stop command from guidance\n");
						loopContinue = 0;
					}
				}
			}
		}

		if (numReceived >= 512) {
			printf( "Navigation: Received maximum inputs without stop command; exiting\n");
			loopContinue = 0;
		}
	}

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
