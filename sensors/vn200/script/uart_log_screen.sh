#!/bin/bash

baud=115200
if [ $1 ]; then
	baud=$1
fi

logfilename="screen_$(date +%Y.%m.%d_%H-%M-%S).bin"
echo $logfilename
sleep 2
sudo screen /dev/ttyUSB0 $baud -L $logfilename

