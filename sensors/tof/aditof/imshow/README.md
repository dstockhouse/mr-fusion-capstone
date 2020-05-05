Download the ADI ToF SDK and demo applications from https://github.com/analogdevicesinc/aditof onto the dragonboard

Replace the OpenCV imshow example file (aditof/bindings/opencv/imshow) with this main.cpp

Compile on the dragonboard from aditof/build with `cmake -DDRAGONBOARD=1 -DWITH_OPENCV=1 ..` followed by `make`
