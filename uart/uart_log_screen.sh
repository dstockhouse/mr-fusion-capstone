#!/bin/bash

logfilename="screen_$(date +%Y.%m.%d_%H-%M-%S).bin"
sudo screen /dev/ttyUSB0 57600 -L -LogFile $logfilename

