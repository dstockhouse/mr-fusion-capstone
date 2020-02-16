/***************************************************************************\
 *
 * File:
 * 	VN200Packet.c
 *
 * Description:
 *	Packet ring buffer management for VN200
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/30/2019
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "control.h"
#include "utils.h"
#include "buffer.h"
#include "logger.h"
#include "debuglog.h"
#include "uart.h"
#include "VN200_CRC.h"
#include "VN200Packet.h"


int VN200PacketParse(VN200_PACKET_RING_BUFFER *ringbuf, int packetIndex) {

    int i, rc;
    uint8_t chkOld, chkNew;
    int unrolledEnd, unrolledIndex, packetIDLength, chkStartIndex;

    VN200_PACKET *packet;
    char *packetBuf;
    int packetDataStart, packetLen;

    // Check null pointer
    if(ringbuf == NULL) {
        return -1;
    }
    // Ensure packet in valid range
    if(packetIndex < 0 || packetIndex >= VN200_PACKET_RING_BUFFER_SIZE) {
        return -2;
    }

    // Compute unrolled end and current packet index
    // Unrolling is undoing mod to buffer size
    unrolledEnd = ringbuf->end;
    unrolledIndex = packetIndex;
    if(unrolledEnd < ringbuf->start) {
        unrolledEnd += VN200_PACKET_RING_BUFFER_SIZE;
    }
    if(unrolledIndex < ringbuf->start) {
        unrolledIndex += VN200_PACKET_RING_BUFFER_SIZE;
    }

    // If unrolled index is past the unrolled end must be invalid, return
    if(unrolledIndex >= unrolledEnd) {
        return -3;
    }

    // Make local pointer to current packet and buffer
    packet = &(ringbuf->packets[packetIndex]);
    packetBuf = ringbuf->buf->buffer;

    // If first character is not $, not true start of packet
    if(packetBuf[packet->startIndex] != '$')
    {
        return -4;
    }

    // Find end of packet (*)
    for(chkStartIndex = packet->startIndex; chkStartIndex < packet->endIndex - 3 && packetBuf[chkStartIndex] != '*'; chkStartIndex++) {
        // Loop until asterisk is reached
    }

    // Verify checksum (from start to [end-1] - 3 to avoid reading checksum)
    int chkLength = sscanf(&(packetBuf[chkStartIndex + 1]), "%hhX", &chkOld);
    chkNew = VN200CalculateChecksum(&(packetBuf[packet->startIndex + 1]), packet->endIndex - packet->startIndex - 5);

    logDebug(L_VDEBUG, "Read checksum from index %d (%c) to %d (%c)\n",
            chkStartIndex + 1, packetBuf[chkStartIndex + 1],
            chkStartIndex + chkLength + 1, packetBuf[chkStartIndex + chkLength + 1]);
    logDebug(L_VDEBUG, "Computed checksum from index %d (%c) to %d (%c)\n",
            packet->startIndex, packetBuf[packet->startIndex],
            packet->endIndex - packet->startIndex - 4 + packet->startIndex,
            packetBuf[packet->endIndex - packet->startIndex - 4 + packet->startIndex]);


    if(chkNew != chkOld) {
        // Checksum failed, don't parse but skip to next packet
        logDebug("Checksum failed: Read %02X but computed %02X\n", chkOld, chkNew);
        return 1;
    }

    // Search for end of packet ID (ex. VNIMU)
    for(i = packet->startIndex; i < packet->endIndex && packetBuf[i] != ','; i++) {
        // Loop until comma is reached
    }

    if(i < packet->endIndex) {
        // Terminate string temporarily
        char temp = packetBuf[i];
        packetBuf[i] = '\0';
        logDebug(L_DEBUG, "%s: Packet ID at index %d is %s\n", __func__, packetIndex, &(packetBuf[packet->startIndex]));
        packetBuf[i] = temp;
    } else {
        char temp = packetBuf[i-1];
        packetBuf[i-1] = '\0';
        logDebug("%s: End of packet reached before packet ID for %d: %s\n", __func__, packetIndex, &(packetBuf[packet->startIndex]));
        packetBuf[i-1] = temp;
    }

    // Length of packet ID string is the number travelled
    packetIDLength = i - packet->startIndex;

    // Ignore intro $VN***
    packetDataStart = i + 1;
    packetLen = packet->endIndex - packetDataStart;

    /**** Determine type of packet and parse accordingly ****/

    // Packet is a GPS packet
    if(packetIDLength == 6 && 
            !strncmp(&(packetBuf[packet->startIndex]), "$VNGPE", packetIDLength)) {

        logDebug("Found a GPS Packet at index %d: %c%c%c%c%c%c\n",
                packet->startIndex,
                packetBuf[packet->startIndex],
                packetBuf[packet->startIndex+1],
                packetBuf[packet->startIndex+2],
                packetBuf[packet->startIndex+3],
                packetBuf[packet->startIndex+4],
                packetBuf[packet->startIndex+5]);

        // Parse as GPS packet
        rc = VN200GPSPacketParse(&(packetBuf[packetDataStart]), packetLen, &(packet->GPSData));
        packet->GPSData.timestamp = packet->timestamp;


        // Set packet stats
        packet->contentsType = VN200_PACKET_CONTENTS_TYPE_GPS;
        packet->isParsed = 1;

        // Packet is an IMU packet
    } else if(packetIDLength == 6 && 
            !strncmp(&(packetBuf[packet->startIndex]), "$VNIMU", packetIDLength)) {

        logDebug("Found an IMU Packet at index %d: %c%c%c%c%c%c\n",
                packet->startIndex,
                packetBuf[packet->startIndex],
                packetBuf[packet->startIndex+1],
                packetBuf[packet->startIndex+2],
                packetBuf[packet->startIndex+3],
                packetBuf[packet->startIndex+4],
                packetBuf[packet->startIndex+5]);

        // Parse as IMU packet
        rc = VN200IMUPacketParse(&(packetBuf[packetDataStart]), packetLen, &(packet->IMUData));

        // Set packet stats
        packet->contentsType = VN200_PACKET_CONTENTS_TYPE_IMU;
        packet->isParsed = 1;

        // Packet is unknown type or improperly formatted
    } else {

        char tempbuf[512];

        // Printout confusion message
        logDebug("%s: Unknown or improper message packet: ", __func__);

        snprintf(tempbuf, MIN(512, packet->endIndex - packet->startIndex), "%s", &(packetBuf[packet->startIndex]));
        logDebug(tempbuf);
        logDebug("\n");

        // Set packet stats
        packet->contentsType = VN200_PACKET_CONTENTS_TYPE_OTHER;
        packet->isParsed = 1;

    }

    return 0;

} // VN200PacketParse(VN200_PACKET_RING_BUFFER *, int)


int VN200PacketRingBufferEmpty(VN200_PACKET_RING_BUFFER *ringbuf) {

    ringbuf->start = 0;
    ringbuf->end = 0;

    return 0;

} // VN200PacketRingBufferEmpty(VN200_PACKET_RING_BUFFER *)


int VN200PacketRingBufferIsEmpty(VN200_PACKET_RING_BUFFER *ringbuf) {

    if(ringbuf == NULL) {
        return -1;
    }

    // Empty if start and end are same position
    return ringbuf->start == ringbuf->end;

} // VN200PacketRingBufferIsEmpty(VN200_PACKET_RING_BUFFER *)


int VN200PacketRingBufferIsFull(VN200_PACKET_RING_BUFFER *ringbuf) {

    if(ringbuf == NULL) {
        return -1;
    }

    // Full if end is one behind start
    return ringbuf->end == MOD((ringbuf->end - 1), VN200_PACKET_RING_BUFFER_SIZE);

} // VN200PacketRingBufferIsFull(VN200_PACKET_RING_BUFFER *)


int VN200PacketRingBufferAddPacket(VN200_PACKET_RING_BUFFER *ringbuf, int startIndex) {

    int packetIndex;

    if(ringbuf == NULL) {
        return -1;
    }

    // If ring buffer is full, do nothing
    if(VN200PacketRingBufferIsFull(ringbuf)) {
        return 0;
    }

    // Set new packet index to end of ring buffer
    packetIndex = ringbuf->end;

    // Move end forward by one
    ringbuf->end = (ringbuf->end + 1) % VN200_PACKET_RING_BUFFER_SIZE;

    // Get system timestamp for packet
    getTimestamp(&(ringbuf->packets[packetIndex].timestamp_ts), &(ringbuf->packets[packetIndex].timestamp));

    // Set contents type to neither GPS nor IMU
    ringbuf->packets[packetIndex].contentsType = VN200_PACKET_CONTENTS_TYPE_OTHER;

    // Set not parsed
    ringbuf->packets[packetIndex].isParsed = 0;

    // Set packet start and end points to input argument (currently incomplete packet)
    ringbuf->packets[packetIndex].startIndex = startIndex;
    ringbuf->packets[packetIndex].endIndex = startIndex;

    return 1;

} // VN200PacketRingBufferAddPacket(VN200_PACKET_RING_BUFFER *, int)


int VN200PacketRingBufferRemovePacket(VN200_PACKET_RING_BUFFER *ringbuf) {

    // logDebug("\tAttempting to remove packet\n");

    if(ringbuf == NULL) {
        return -1;
    }

    // If ring buffer is empty, do nothing
    if(VN200PacketRingBufferIsEmpty(ringbuf)) {
        return 0;
    }

    ringbuf->start = (ringbuf->start + 1) % VN200_PACKET_RING_BUFFER_SIZE;

    return 1;

} // VN200PacketRingBufferRemovePacket(VN200_PACKET_RING_BUFFER *)


int VN200PacketRingBufferUpdateEndpoints(VN200_PACKET_RING_BUFFER *ringbuf) {

    int i;
    int lastPacketIndex, packetsAdded = 0, addSuccess;

    // Local copy of current packet
    VN200_PACKET *packet;

    if(ringbuf == NULL) {
        return -1;
    }

    // Ensure at least one packet in ring buffer
    if(VN200PacketRingBufferIsEmpty(ringbuf)) {

        // Add blank packet
        packetsAdded += VN200PacketRingBufferAddPacket(ringbuf, 0);

    }

    // Find last packet in ring buffer
    lastPacketIndex = MOD((ringbuf->end - 1), VN200_PACKET_RING_BUFFER_SIZE);
    logDebug("\tLast packet index is %d\n", lastPacketIndex);
    packet = &(ringbuf->packets[lastPacketIndex]);

    // If start == end, then the packet was just created.
    // We don't want to trigger immediately and detect the same packet
    if(packet->endIndex == packet->startIndex) {
        packet->endIndex++;
    }

    // Loop from current ring-buffer-known end to true end of buffer
    logDebug("\tlooping. start=%d, max=%d\n", packet->endIndex, ringbuf->buf->length);
    for(i = packet->endIndex; i < ringbuf->buf->length; i++) {


        // Determine if this end index is the start of new packet
        if(ringbuf->buf->buffer[i] == '$') {

            logDebug("\t\tFound start of packet. i=%d\n", i);

            // End previous packet
            packet->endIndex = i;

            // Create new packet
            addSuccess = VN200PacketRingBufferAddPacket(ringbuf, i);
            packetsAdded += addSuccess;

            // Update last packet index
            lastPacketIndex = MOD((ringbuf->end - 1), VN200_PACKET_RING_BUFFER_SIZE);
            logDebug("\t\tLast packet index is %d\n", lastPacketIndex);
            packet = &(ringbuf->packets[lastPacketIndex]);

            getTimestamp(&(packet->timestamp_ts), &(packet->timestamp));
            /********* *SET TIMESTAMP HERE* **********/

        }

    }

    // Return number of packets added
    return packetsAdded;

} // VN200PacketRingBufferUpdateEndpoints(VN200_PACKET_RING_BUFFER *) {


// Disable to consider deleting
#if 0
int VN200PacketRingBufferSetLastEndpoint(VN200_PACKET_RING_BUFFER *ringbuf, int endpoint) {

    int i;
    int lastPacketIndex, addStatus;

    // Local copy of current packet
    VN200_PACKET *packet;

    if(ringbuf == NULL) {
        return -1;
    }

    // Ensure at least one packet in ring buffer
    if(VN200PacketRingBufferIsEmpty(ringbuf)) {

        // Add blank packet
        addStatus = VN200PacketRingBufferAddPacket(ringbuf, 0);

        // Return failure if couldn't add packet
        if(!addStatus) {
            return -2;
        }

    }

    // Find last packet in ring buffer
    lastPacketIndex = MOD((ringbuf->end - 1), VN200_PACKET_RING_BUFFER_SIZE);
    packet = &(ringbuf->packets[lastPacketIndex]);

    // Invalid if end is before start for this packet
    if(endpoint < packet->startIndex) {
        packet->endIndex = endpoint;
        return -3;
    }

    return 0;

} // VN200PacketRingBufferSetLastEndpoint(VN200_PACKET_RING_BUFFER *, int) {


int VN200PacketRingBufferConsume(VN200_PACKET_RING_BUFFER *ringbuf, int numToConsume) {

    int packetsConsumed = 0, i;

    logDebug("\tAttempting to consume %d packets\n", numToConsume);

    VN200_PACKET *packet;

    if(ringbuf == NULL) {
        return -1;
    }

    // If ring buffer is already empty, don't remove anything
    if(VN200PacketRingBufferIsEmpty(ringbuf)) {
        return 0;
    }

    logDebug("\t\tLooping...\n");

    // Loop through ring buffer to adjust every packet
    for(i = ringbuf->start;
            i != ringbuf->end;
            i = (i + 1) % VN200_PACKET_RING_BUFFER_SIZE) {

        logDebug("\t\t\tIn loop. s=%d, e=%d, i=%d...\n",
                ringbuf->start, ringbuf->end, i);

        // Pointer to current packet
        packet = &(ringbuf->packets[i]);

        // Move start and end indices backwards by length of that packet
        packet->startIndex -= numToConsume;
        packet->endIndex -= numToConsume;

        logDebug("\t\t\tEnd of loop. si=%d, ei=%d...\n",
                packet->startIndex, packet->endIndex);
        // If index is below 0 the packet has been (at least partially) consumed
        if(packet->startIndex < 0 || packet->endIndex < 0) {
            packetsConsumed += VN200PacketRingBufferRemovePacket(ringbuf);
        }

    }

    logDebug("\t\tConsumed %d packets.\n", packetsConsumed);

    // Return number of packets consumed
    return packetsConsumed;

} // VN200PacketRingBufferConsume(VN200_PACKET_RING_BUFFER *, int)
#endif


int VN200PacketIsIncomplete(VN200_PACKET *packet) {

    if(packet == NULL) {
        return -1;
    }

    // Packet is incomplete if the start and end indices are the same
    return packet->startIndex == packet->endIndex;

} // VN200PacketIsIncomplete(VN200_PACKET *)

