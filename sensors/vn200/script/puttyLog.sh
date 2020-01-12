mkdir -p $HOME/icarus-dronenet/log/putty
logfilename="VN200_$(date +%Y.%m.%d_%H-%M-%S).bin"
plink -serial /dev/ttyUSB0 -sercfg $1 | tee $HOME/icarus-dronenet/log/putty/$logfilename

