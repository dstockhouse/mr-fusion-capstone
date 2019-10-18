mkdir -p /home/pi/icarus-dronenet/log/putty
logfilename="VN200_$(date +%Y.%m.%d_%H-%M-%S).bin"
plink -serial /dev/ttyUSB0 -sercfg $1 | tee /home/pi/icarus-dronenet/log/putty/$logfilename

