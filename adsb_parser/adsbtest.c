/****************************************************************************\
 *
 * File:
 * 	adsbtest.c
 *
 * Description:
 * 	Tests the interface with the ADS-B reciever
 *
 * Author:
 * 	Joseph Kroeker & David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/26/19
 *
\***************************************************************************/

#include "adsb_parser.h"

#include "../buffer/buffer.h"
#include "crc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// #define DEFAULT_LOG_FILE "SampleData/ADS_B/putty_2019.02.26_103032.log"

/*
 * Function -> main()
 * retval -> 1 on success
 */
int main(int argc, char **argv) {
	uint32_t rv = 0;
	// uint8_t* testHeader = "\xfe\x26\x28\x00\x00\xf6"; // just the header for now
	// uint8_t* testData = "\xFF\xEA\x00\x00\xC0\xE2\xA1\x14\x4B\x6B\xF9\xBC\x10\xCA\x00\x00\x00\x00\x00\x00\x00\x1F\x80\x00\x00\x00\x49\x43\x41\x52\x55\x53\x31\x00\x00\x0E\x01\x65\xDF";
	// uint8_t* testFooter = "\x4a\x39";
	MsgHeader header;
	MsgData246 data;

	// Enough room for all data in the file
#define FILE_SIZE 221184
	unsigned char fileData[FILE_SIZE];
	char filename[512], parselog[512], parselogextra[512];
	int rc, i = 0, chkRd, chkNew, packfd, improper=0, proper=0;

	// Binary file descriptor
	int fd;


	if(argc > 1) {
		strcpy(filename, argv[1]);
	} else {
		strcpy(filename, "SampleData/ADS_B/ADS_B-02.20.2019_18-15-14.bin");
	}
	fd = open(filename, O_RDONLY);

	// Read binary data into local variable
	rc = read(fd, fileData, FILE_SIZE);
	printf("Read %d bytes\n\n", rc);

	while(i < rc) {

		// Seek to next start of packet
		// for( ; i < rc - 38 && (fileData[i] != 0xfe || fileData[i + 1] != 0x26 || fileData[i + 5] != 246); i++) ;
		for( ; i < rc - 46 && (fileData[i] != 0xfe || fileData[i + 1] != 0x26 || fileData[i + 5] != 246); i++) {
			// printf("%d: 0x%x\n", i, fileData[i]);
		}

		if(i >= rc - 38) {
			break;
		}

// 		printf("Parsing index %d (0x%x)\n", i, i);

		chkRd = fileData[i + 44] | (fileData[i + 45] << 8);
		chkNew = crc_calculate(&(fileData[i+1]), 43);
// 		printf("Read checksum: %04x\n", chkRd);
// 		printf("Computed chks: %04x\n", chkNew);

		if(chkRd != chkNew) {
// 			printf("Failed checksum, skipping...\n\n");
			i++;
			improper++;
			continue;
		}
// 		printf("Passed checksum, parsing...\n\n");
		proper++;

		// Log the packet at this index to its own file
		sprintf(parselogextra, ".id%08x.pkt", i);
		strcpy(parselog, filename);
		strcat(parselog, parselogextra);
		printf("Logging packet to file %s...\n", parselog);
		packfd = open(parselog, O_WRONLY | O_CREAT);
		write(packfd, &(fileData[i]), 46);
		close(packfd);


		// rv = parseHeader(testHeader, &header, 1);
		rv = parseHeader(&(fileData[i]), &header, 0);
		// printf("Header\n");
		// printf("start flg: %d\n", header.startFlg);
		// printf("message ID: %d\n", header.messageID);
		// printf("length: %d\n", header.length);
		printf("Header:\n");
		printHeader(&header);

		// rv = parseData(testData, &data);
		rv = parseData(&(fileData[i + 6]), &data, 0);
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
	printf("\n\nTest complete!\n\n");
	printf("Found %d valid packets, %d invalid\n\n", proper, improper);

}

