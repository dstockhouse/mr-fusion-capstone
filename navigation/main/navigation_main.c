
#include "config.h"
#include "navigation_run.h"

int main(int argc, char** argv) {

	int run_rc;

	// Set up network and serial interfaces

	// Dispatch necessary threads and processes

	run_rc = navigation_run(0);

	// Safely shutdown the application

	return run_rc;
}

