/****************************************************************************\
 *
 * File:
 * 	ADS_B.c
 *
 * Description:
 * 	Interfaces with the ADS-B reciever
 *
 * Author:
 * 	Joseph Kroeker & David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/20/19
 *
 * Revision 0.2
 * 	Split into source and header files
 * 	Last edited 2/25/19
 *
\***************************************************************************/

#include "ADS_B.h"

#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


/*
 * Function -> main()
 * retval -> 1 on success
 */
int main(void) {
	uint32_t rv = 0;
	// uint8_t* testHeader = "\xfe\x26\x28\x00\x00\xf6"; // just the header for now
	// uint8_t* testData = "\xFF\xEA\x00\x00\xC0\xE2\xA1\x14\x4B\x6B\xF9\xBC\x10\xCA\x00\x00\x00\x00\x00\x00\x00\x1F\x80\x00\x00\x00\x49\x43\x41\x52\x55\x53\x31\x00\x00\x0E\x01\x65\xDF";
	// uint8_t* testFooter = "\x4a\x39";
	MsgHeader header;
	MsgData246 data;

	// Enough room for all data in the file
#define FILE_SIZE 221184
	unsigned char fileData[FILE_SIZE];
	int rc, i = 0;

	// Binary file descriptor
	int fd;

	// 'b' not strictly necessary, but won't hurt
	fd = open("SampleData/ADS_B-02.20.2019_18-15-14.bin", O_RDONLY);

	// Read binary data into local variable
	rc = read(fd, fileData, FILE_SIZE);
	printf("Read %d bytes\n\n", rc);

	while(i < rc) {

		// Seek to next start of packet
		// for( ; i < rc - 38 && (fileData[i] != 0xfe || fileData[i + 1] != 0x26 || fileData[i + 5] != 246); i++) ;
		for( ; i < rc - 38 && (fileData[i] != 0xfe && fileData[i + 1] != 0x26 || fileData[i + 5] != 246); i++) {
			// printf("%d: 0x%x\n", i, fileData[i]);
		}

		if(i >= rc - 38) {
			printf("Test complete\n");
			return 0;
		}

		printf("Parsing index %d (0x%x)\n", i, i);

		// rv = parseHeader(testHeader, &header, 1);
		rv = parseHeader(&(fileData[i]), &header, 1);
		// printf("Header\n");
		// printf("start flg: %d\n", header.startFlg);
		// printf("message ID: %d\n", header.messageID);
		// printf("length: %d\n", header.length);
		printf("Header:\n");
		printHeader(&header);

		printf("\n");

		// rv = parseData(testData, &data);
		rv = parseData(&(fileData[i + 6]), &data, 1);
		// printf("Callsign: %s\n\n", data.callsign);
		// printf("Lat:      %.3f deg N\n", data.lat * 1e-7);
		// printf("Lon:      %.3f deg E\n", data.lon * 1e-7);
		// printf("Altitude: %.3f m\n", data.altitude * 1e-3);
		// printf("Heading:  %.3f deg\n", data.heading * 1e-2);
		// printf("V_horiz:  %.3f m/s\n", data.hor_velocity * 1e-2);
		// printf("V_vert:   %.3f m/s\n", data.ver_velocity * 1e-2);
		// printf("\nTime since last communication: %d s\n\n", data.tslc);

		printf("Data:\n");
		printData(&data);

		i++;

	}

}

/*
 * Function -> parseData
 * Purpose -> parse adsb data for a single message
 * Input -> single data binary
 * Output -> Pointer to struct of data field values
 */
int parseData(uint8_t* data, MsgData246* msgData, int verbose) {
	int idx = 0;

	if(verbose) {
		printf("Data is:\n");
		for(idx = 0; idx < 38; idx++) {
			printf("\t%02x\n", data[idx]);
		}
		printf("\n");
	}

	msgData->ICAO_adress = ((data[0] << 0x18) | (data[1] << 0x10) |
			(data[2] << 0x08) | (data[3]));
	if(verbose) {
		printf("\tICAO:  ");
		for(idx = 0; idx < 4; idx++) {
			printf("%02x ", data[idx]);
		}
		printf("\n");
		// printf("\t       %08x\n", msgData->ICAO_adress);
	}

// 	msgData->lat = ((data[4] << 0x18) | (data[5] << 0x10) |
// 			(data[6] << 0x08) | (data[7]));
	msgData->lat = ((data[4]) | (data[5] << 0x08) |
			(data[6] << 0x10) | (data[7] << 0x18));
	if(verbose) {
		// printf("\tlat:   ");
		for(idx = 4; idx < 8; idx++) {
			// printf("%02x ", data[idx]);
		}
		// printf("\n");
	}

//	msgData->lon = ((data[8] << 0x18) | (data[9] << 0x10) |
//			(data[10] << 0x08) | (data[11]));
	msgData->lon = ((data[8]) | (data[9] << 0x08) |
			(data[10] << 0x08) | (data[11] << 0x18));
	if(verbose) {
		// printf("\tlon:   ");
		for(idx = 8; idx < 12; idx++) {
			// printf("%02x ", data[idx]);
		}
		// printf("\n");
	}

//	msgData->altitude = ((data[12] << 0x18) | (data[13] << 0x10) |
//			(data[14] << 0x08) | (data[15]));
//	msgData->altitude_inv = ((data[12]) | (data[13] << 0x08) |
//			(data[14] << 0x10) | (data[15] << 0x18));
	msgData->altitude = ((data[12]) | (data[13] << 0x08) |
			(data[14] << 0x10) | (data[15] << 0x18));
	if(verbose) {
		// printf("\talt:   ");
		for(idx = 12; idx < 16; idx++) {
			// printf("%02x ", data[idx]);
		}
		// printf("\n");
	}

	msgData->heading = ((data[16]) | (data[17] << 0x08));
	if(verbose) {
		// printf("\thead:  ");
		for(idx = 16; idx < 18; idx++) {
			// printf("%02x ", data[idx]);
		}
		// printf("\n");
	}
	msgData->hor_velocity = ((data[19] << 0x08) | (data[18]));
	if(verbose) {
		// printf("\thorv:  ");
		for(idx = 18; idx < 20; idx++) {
			// printf("%02x ", data[idx]);
		}
		// printf("\n");
	}
	msgData->ver_velocity = ((data[21] << 0x08) | (data[20]));
	if(verbose) {
		// printf("\tverv:  ");
		for(idx = 20; idx < 22; idx++) {
			// printf("%02x ", data[idx]);
		}
		// printf("\n");
	}
	msgData->validFlags = ((data[23] << 0x08) | (data[22]));
	if(verbose) {
		// printf("\tvalid: ");
		for(idx = 22; idx < 24; idx++) {
			// printf("%02x ", data[idx]);
		}
		// printf("\n");
	}
	msgData->squawk= ((data[25] << 0x08) | (data[24]));
	if(verbose) {
		// printf("\tsqk:   ");
		for(idx = 24; idx < 26; idx++) {
			// printf("%02x ", data[idx]);
		}
		// printf("\n");
	}
	msgData->altitude_type = data[26];
	if(verbose) {
		// printf("\talt_t: %02x\n", data[26]);
	}

	// printf("%c\n%c\n", data[26], data[27]);

	if(verbose) {
		printf("\tclsgn: ");
	}
	for (idx = 0;idx < 9;idx++) {
		// printf("idx: %d, d: (0x%x): ", idx, data[27+idx]);
		if(data[27+idx] >= 0x20 && data[27+idx] < 0xff) {
			// printf("%c", data[27+idx]);
		}
		// printf("\n");
		msgData->callsign[idx] = data[27+idx];
		if(verbose) {
			printf("%02x ", data[27+idx]);
		}
	}
	printf("\n");

	msgData->emitter_type = data[36];
	if(verbose) {
		// printf("\temt_t: %02x\n", data[36]);
	}
	msgData->tslc = data[37];
	if(verbose) {
		// printf("\ttslc:  %02x\n", data[37]);
	}
}


int printData(MsgData246 *data) {

	int i;

	printf("Lat:      %.3f deg N\n", data->lat * 1e-7);
	printf("Lon:      %.3f deg E\n", data->lon * 1e-7);
	printf("Altitude: %.3f m\n", data->altitude * 1e-3);
	// printf("  (inv) : %.3f m\n", data->altitude_inv * 1e-3);
	printf("Heading:  %.3f deg\n", data->heading * 1e-2);
	printf("V_horiz:  %.3f m/s\n", data->hor_velocity * 1e-2);
	printf("V_vert:   %.3f m/s\n", data->ver_velocity * 1e-2);
	printf("Callsign: %s (", data->callsign);
	for(i = 0; i < 9; i++) {
		printf("%02x ", data->callsign[i]);
	}
	printf(")\n");
	printf("Time since last communication: %d s\n\n", data->tslc);

	return 0;

}


int parseHeader(uint8_t* message, MsgHeader* header, int verbose) {
	int idx;
	// printf("Data is:\n");
	for(idx = 0; idx < 6; idx++) {
		// printf("%3d: (0x%x)\n", idx, message[idx]);
	}
	// printf("\n");

	// Find Message id
	header->startFlg = message[0]; // Find start flag
	if(verbose) {
		printf("\tstart: %02x\n", message[0]);
	}

	// verify that start flag is expected value
	if (header->startFlg == 0xfe) {
		// parse the header
		header->length = message[1];
		if(verbose) {
			printf("\tlen:   %02x\n", message[1]);
		}
		header->sequence = message[2];
		if(verbose) {
			printf("\tseq:   %02x\n", message[2]);
		}
		header->sysID = message[3];
		if(verbose) {
			printf("\tSID:   %02x\n", message[3]);
		}
		header->component = message[4];
		if(verbose) {
			printf("\tcomp:  %02x\n", message[4]);
		}
		header->messageID = message[5];
		if(verbose) {
			printf("\tMID:   %02x\n", message[5]);
		}
		return 1;
	}
	return -1;
}

int printHeader(MsgHeader *header) {

	printf("start flg: %d\n", header->startFlg);
	printf("message ID: %d\n", header->messageID);
	printf("length: %d\n", header->length);

	return 0;

}

int parseBuffer(BYTE_BUFFER *buf, MsgHeader *header, MsgData246 *data) {

	int i, rc, numParsed = 0;

	while(i < buf->length) {

		/********************************************************************** Finish *************/

		for( ; i < buf->length - 38 && buf->buffer[i] != 0xfe; i++) {
			// printf("%d: 0x%x\n", i, fileData[i]);
		}

		if(i >= buf->length - 38) {
			return 0;
		}

		rc = parseHeader(&(buf->buffer[i]), header, 0);

		rc = parseData(&(buf->buffer[i + 6]), data, 0);

		numParsed = i + 37;
		i++;

	}

	return 0;

}


