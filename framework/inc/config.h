// Configuration parameters for the entire system

#ifndef MR_FUSION_SYSTEM_CONFIG
#define MR_FUSION_SYSTEM_CONFIG

// Placeholders. Fill in with real data during test and integration
#define GUIDANCE_IP_ADDR 	"XXX.XXX.XXX.XXX"
#define NAVIGATION_IP_ADDR 	"XXX.XXX.XXX.XXX"
#define CONTROL_IP_ADDR 	"XXX.XXX.XXX.XXX"
#define DRAGONBOARD_IP_ADDR 	"XXX.XXX.XXX.XXX"

// Need individual port for each pair of devices
#define GUIDANCE_TCP_PORT 	38000
#define NAVIGATION_TCP_PORT 	38010
#define CONTROL_TCP_PORT 	38020
#define DRAGONBOARD_TCP_PORT 	38030

#endif // MR_FUSION_SYSTEM_CONFIG

