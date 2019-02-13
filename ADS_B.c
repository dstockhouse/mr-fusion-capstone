
/****************************************************************************\
 *
 * File:
 * 	ADS_B.c
 *
 * Description:
 *  Interfaces with the ADS-B reciever
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 1.1
 * 	Last edited 6/7/18
 *
\***************************************************************************/
#define COM_PORT "/dev/TTYUSB0"

/*
* Function -> main()
* retval -> 1 on success
*/
int main(void) {
    int conAttempts = 0; // connection Attemps (limit = 3)
    int rv = 0; // Ret val from other functions
    for (conAttempts = 0; (conAttemps < 3) && (rv != 1), conAttempts++) {
        rv = connectPort(); // Attempt Connection to port
    }
}

/*
* Function -> connectPort
* retval -> 1 on success, 0 on failure
*/
int connectPort(void){

}
