#ifndef	__LIDAR_H
#define	__LIDAR_H

//#define Serial   Serial
#define PC_BAUD_RATE 115200

//  parameters for comms with TFMini (which pin is the TFMini tx connectedto?)
#define TF_UART			"/dev/serial0"
#define TF_BAUD_RATE	115200

//  control parameters
#define PAUSE           55      //  minimum ms allowed between sensor moves
#define	STEPS_PER_REV	2048
#define	STEP_PER_DEG	STEPS_PER_REV/360.0	//	~5.7 steps per degree
#define	DEG_PER_MOVE	0.5
#define STEPS_PER_MOVE  DEG_PER_MOVE*STEP_PER_DEG*2    //  2.84 steps/half degree...  ~0.175 deg per step (2048 full steps per revolution)
#define MAX_ANGLE       150     //  maximum angular travel (home is full CCW)
#define MIN_ANGLE       30

//  mapping constants
#define PLT_WID         1400                      //  (X x Y)
#define PLT_HT          750 
#define XOFFSET         PLT_WID/2                 //  x-center of plot
#define YOFFSET         150                       //  how far "north" is the center of the sweep
#define YCORRECT        PLT_HT-YOFFSET            //  used to compute y coordinates
#define SWP_ANG         (MAX_ANGLE-MIN_ANGLE)     //  full angle of the sweep
#define OFF_ANG         90-SWP_ANG/2              //  Offset angle for sensor array (sensor angle-half angle)
#define DEG2RAD         3.14159/180.0             //  conversion factor to go from degrees to radians
#define lRANGE          500                       //  length of the beam indicator
#define	NUM_DATAPTS		SWP_ANG+MIN_ANGLE+5			//	array size of data
#define	MIN_DIA			1

//#define	HOME_SW	0		//	pi pin
#define	HOME_SW	17		//	gpio pin	- this matches the numbering on the T-shield
//#define	HOME_SW	11		//	physical pin

//const char htmlFileName[] = "/media/ramdisk/index.html";
const char htmlFileName[] = "/var/www/html/index.html";
const char tempFileName[] = "/var/www/html/index";
	

#endif
