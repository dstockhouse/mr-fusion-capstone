# ICARUS DroneNet

This repository is for a subset of research related to the DroneNet project by
the ICARUS group at Embry-Riddle Aeronautical University in Prescott, AZ. 

## Instructions

To use this software in the ICARUS configuration, we needed to make some
additional modifications for the host pi.
* Add `cd /home/pi/icarus-dronenet && ./test` to `/etc/rc.local`
* Add cron job to update the system clock every minute so that it properly
timestamps log files

## Objectives

### ADS-B

The first goal of this project is to interface to a uAvionix ADS-B receiver
which outputs ADS-B data in MAVLink packets through a USB serial port. The code
will be running on a Raspberry Pi 3 connected through USB to the ADS-B receiver. 

This goal has two steps to completion: interfacing to the ADS-B receiver and
parsing MAVLink packets to decode and make available received ADS-B traffic
data.

### VN200

The second goal is to interface the VN200 GPS/IMU with the pi. Once both sensor
data are being collected by the pi, the next step is to read them
simultaneously.

### Realtime

Once both above goals are met, the code will be integrated together and
transitioned to a realtime multithreaded system.

## Status

The ADS-B is working but not thoroughly tested. The VN200 has working code to
read from both GPS and IMU sensors, but it has a few more features to add.  
Simultaneous readout and automatic logging of both GPS and IMU is now
functional, but the parser is not fully tested.

## Contact

David Stockhouse  
[stockhod@my.erau.edu](mailto:stockhod@my.erau.edu)

Joseph Kroeker  
[kroekerj@my.erau.edu](mailto:kroekerj@my.erau.edu)

Last updated 10/11/2019
