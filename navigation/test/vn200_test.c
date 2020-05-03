
#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>

#include "vn200_struct.h"
#include "vn200_imu.h"
#include "vn200_gps.h"
#include "vn200_crc.h"

Describe(VN200);
BeforeEach(VN200) {}
AfterEach(VN200) {}

Ensure(VN200, parse_imu_packet) {

    int rc;
    IMU_DATA data;

    unsigned char *packet = (unsigned char *)
        "VNIMU,"
        "+01.0854,-02.0143,+02.1980,"
        "-01.157,+00.271,-09.847,"
        "+00.001114,+00.000727,+00.002568,"
        "+21.4,+084.334";
    unsigned char chksum = 0x6D;

    int start = 6;
    int len = strlen((char *)&packet[start]);

    rc = VN200IMUPacketParse(&packet[start], len, &data);
    assert_that(rc, is_equal_to(len));

    // Verify parsed data
    significant_figures_for_assert_double_are(5);
    assert_that_double(data.compass[0], is_equal_to_double(1.0854));
    assert_that_double(data.compass[1], is_equal_to_double(-2.0143));
    assert_that_double(data.compass[2], is_equal_to_double(2.1980));

    significant_figures_for_assert_double_are(4);
    assert_that_double(data.accel[0], is_equal_to_double(-1.157));
    assert_that_double(data.accel[1], is_equal_to_double(0.271));
    assert_that_double(data.accel[2], is_equal_to_double(-9.847));

    significant_figures_for_assert_double_are(4);
    assert_that_double(data.gyro[0], is_equal_to_double(0.001114));
    assert_that_double(data.gyro[1], is_equal_to_double(0.000727));
    assert_that_double(data.gyro[2], is_equal_to_double(0.002568));

    // Compute checksum
    len = strlen((char *)packet);
    unsigned char newChk = VN200CalculateChecksum(packet, len);
    assert_that(chksum, is_equal_to(newChk));

}

Ensure(VN200, parse_gps_packet) {

    int rc;
    GPS_DATA data;

    unsigned char *packet = (unsigned char *)
        "VNGPE,"
        "570937.199558,2075,3,07,"
        "-2006902.850,-4857470.210,+3604176.410,"
        "+000.110,-000.680,+000.170,"
        "+019.320,+016.935,+016.758,"
        "+001.312,9.00E-09";
    unsigned char chksum = 0x07;

    int start = 6;
    int len = strlen((char *)&packet[start]);

    rc = VN200GPSPacketParse(&packet[start], len, &data);
    assert_that(rc, is_equal_to(len));

    significant_figures_for_assert_double_are(12);
    assert_that_double(data.time, is_equal_to_double(570937.199558));
    assert_that(data.week, is_equal_to(2075));
    assert_that(data.GpsFix, is_equal_to(3));
    assert_that(data.NumSats, is_equal_to(7));

    significant_figures_for_assert_double_are(10);
    assert_that_double(data.PosX, is_equal_to_double(-2006902.850));
    assert_that_double(data.PosY, is_equal_to_double(-4857470.210));
    assert_that_double(data.PosZ, is_equal_to_double(+3604176.410));

    significant_figures_for_assert_double_are(3);
    assert_that_double(data.VelX, is_equal_to_double(0.110));
    assert_that_double(data.VelY, is_equal_to_double(-0.680));
    assert_that_double(data.VelZ, is_equal_to_double(0.170));

    significant_figures_for_assert_double_are(5);
    assert_that_double(data.PosAccX, is_equal_to_double(19.320));
    assert_that_double(data.PosAccY, is_equal_to_double(16.935));
    assert_that_double(data.PosAccZ, is_equal_to_double(16.758));

    significant_figures_for_assert_double_are(4);
    assert_that_double(data.SpeedAcc, is_equal_to_double(1.312));
    significant_figures_for_assert_double_are(3);
    assert_that_double(data.TimeAcc, is_equal_to_double(9.00e-9));

    // Compute checksum
    len = strlen((char *)packet);
    unsigned char newChk = VN200CalculateChecksum(packet, len);
    assert_that(chksum, is_equal_to(newChk));

}

Ensure(VN200, parse_failures) {

    int rc;
    unsigned char nodata[16] = "No packet";
    GPS_DATA gps;
    IMU_DATA imu;

    rc = VN200GPSPacketParse(NULL, 0, NULL);
    assert_that(rc, is_less_than(0));
    rc = VN200IMUPacketParse(NULL, 0, NULL);
    assert_that(rc, is_less_than(0));

    rc = VN200GPSPacketParse(nodata, 4, &gps);
    assert_that(rc, is_less_than(0));
    rc = VN200IMUPacketParse(nodata, 4, &imu);
    assert_that(rc, is_less_than(0));

    rc = VN200GPSPacketParse(nodata, -4, &gps);
    assert_that(rc, is_less_than(0));
    rc = VN200IMUPacketParse(nodata, -4, &imu);
    assert_that(rc, is_less_than(0));

}

