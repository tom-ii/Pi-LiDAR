#include "TStepper.cpp"			//	Stepper driver stuff - it includes its own .h
#include "LiDAR.h"				//	"global" stuff for the project as a whole
#include "mem.c"				//	mem functions for project
#include "motor.c"				//	motor control functions for project
#include <wiringPi.h>			//	digitalWite functions, etc
#include <time.h>				//	timing functions
#include <unistd.h>				//	contains sleep()
#include <stdlib.h>				//	contains exit()
#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>

int _kbhit( void ); 

//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <unistd.h>

/*some pseudocode
 * 1) initiate motor
 * 2) home the motor (give another process a chance to init mem space)
 * 3) connect to (or init) memory space
 * 4) continuosly:
 * 	a)	monitor memory for kill flag
 * 	b)	wait for move_done flag to clear
 * 	c) 	step ~1/2 degree (direction based on angle limits)
 * 	d)	report current angle
 * 	e)	set move_done flag
 * 
 * 	I think that's it(?)
 */
 
int main(int argc, char *argv[] )
{
	float 			Angle = 0;
	unsigned char 	direction = CCW;
	Rmem_ptr		mem_ptr = 0;
	int				shmid = 0;
//	char			kbd[20];
	
	
	//	Set up wiringPi 
	//	wiringPiSetup();		//	set up using wiPi numbering
	wiringPiSetupGpio();	//	set up using gpio numbering
	//	wiringPiSetupPhys();	//	set up using physical pin numbering

	setHome( HOME_SW );			//	turn on the home switch

//	Stepper motor;
	TStepper( STEPS_PER_REV );	//  Instantiate Stepper Motor
	SetConnects( BLUE, PINK, YELLOW, ORANGE );	//	initialize connections to stepper
	//	get shared memory
	shmid = getShmid( configFile, PROJ_ID, sizeof(Rmem_typ) );
	
	/* Attach (map) the shared memory segment into the current process */
	if((mem_ptr = (Rmem_ptr)shmat(shmid, 0, 0)) == (Rmem_ptr)-1)
	{
		perror("shmat");
		exit(1);
	}

	direction = motorHome( &Angle );			//	home the motor nad update direction accordingly
	setCurrentAngle( mem_ptr, Angle );			//	update system angle
	
	// run until we get a kill message
	while( notDone(mem_ptr) )
	{
		//  two things need to happen before we can move the motor
			//	1) deleted - I was wrong...  WRONG, I say!
			//	2) we need indication that the data process is finished reading data
		if( readMoveOkFlag(mem_ptr) )
		{
			direction = SetDirection( direction, Angle );	//  update the movement direction
			Angle = MoveMotor( Angle, direction );				//	had to break up into two lines
																//	as optimizing was doing weird
			setCurrentAngle( mem_ptr, Angle );				//	update system angle
			setMovedFlag( mem_ptr );						//	let other processes know move is complete
			clearMoveOKflag( mem_ptr );
//			sleep(1);	//	for troubleshooting
		}
		
		
		if( _kbhit() )		//	Got a keyboard input - kill everything
			setDoneFlag( mem_ptr );
	}
	
	//	okay - we got the DONE flag, flag memory for deletion
	if( shmctl(shmid, IPC_RMID, 0) == -1)
	{
		printf("big fucking error on shmctl!\n");
		return -1;
	}
	
	printf("Shared memory segment marked for deletion\n");

	//	and the detach from it
//	shmdt();

	return 0;
}

/* stolen from:
 * http://www.flipcode.com/archives/_kbhit_for_Linux.shtml
 */
int _kbhit( void ) 
{
    static const int STDIN = 0;
    static char initialized = FALSE;

    if (! initialized) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = TRUE;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}
