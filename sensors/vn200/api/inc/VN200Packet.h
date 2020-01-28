/***************************************************************************
 *
 * File:
 * 	VN200Packet.h
 *
 * Description:
 *	Function and type declarations and constants for VN200Packet.c
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/30/2019
 *
 ***************************************************************************/

#ifndef __VN200_PACKET_H
#define __VN200_PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "buffer.h"
#include "logger.h"
#include "uart.h"
#include "VN200Struct.h"
#include "VN200_GPS.h"
#include "VN200_IMU.h"

int VN200PacketParse(VN200_PACKET_RING_BUFFER *ringbuf, int packetOffset);

int VN200PacketRingBufferEmpty(VN200_PACKET_RING_BUFFER *ringbuf);

int VN200PacketRingBufferIsEmpty(VN200_PACKET_RING_BUFFER *ringbuf);

int VN200PacketRingBufferAddPacket(VN200_PACKET_RING_BUFFER *ringbuf, int startIndex);

int VN200PacketRingBufferUpdateEndpoints(VN200_PACKET_RING_BUFFER *ringbuf);

int VN200PacketIncomplete(VN200_PACKET *packet);

#endif

