#include "LiDAR.h"				//	"global" stuff for the project as a whole
#include "mem.c"				//	mem functions for project
#include <wiringSerial.h>			//	digitalWite functions, etc (inc UART access)
//#include <time.h>				//	timing functions
//#include <unistd.h>				//	contains sleep()
#include <stdlib.h>				//	contains exit()
#include "TTFMini.cpp"

//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <unistd.h>

/*some pseudocode
 * 1) initiate sensor
 * 2) onnect to (or init) memory space
 * 3) constantly ingest sensor data
 * 4) continuosly:
 * 	a) 	When MOVE_COMP flag is set, update data with latest range
 * 	b)	monitor memory for kill flag
 * 	c) 	set OK_2_MOVE flag
 * 
 * 	I think that's it(?)
 */
 
int main(int argc, char *argv[] )
{
//	int 			Angle = 0;
	Rmem_ptr		mem_ptr = 0;
	int				shmid = 0;
	int				serStream = 0;
//	int				old_sent = 0;
	char 			c = 0;
	int				Range = 0;
	
	//	open the serial port (wiringPi functions to make things a little cleaner (?)
	serStream = serialOpen( TF_UART, PC_BAUD_RATE );
	
	//	initialize the tfmini structures
	TFMini();
	
	if( serStream == -1 )
	{
		perror( "Serial" );
		exit(1);
	}

	//	get shared memory
	shmid = getShmid( configFile, PROJ_ID, sizeof(Rmem_typ) );
	
	/* Attach (map) the shared memory segment into the current process */
	if((mem_ptr = (Rmem_ptr)shmat(shmid, 0, 0)) == (Rmem_ptr)-1)
	{
		perror("shmat");
		exit(1);
	}

//	setMoveOkFlag( mem_ptr );				//	let the motor know it's okay to move
	
	// run until we get a kill message
	while( notDone(mem_ptr) )
	{
		//	continuously read data from sensor
		if( serialDataAvail(serStream) > 0 )
		{
			c = serialGetchar( serStream );		//	get data
			encode( c );						//	encode the data
		}

//		if( rSentences() > old_sent )
//		{
//			old_sent = rSentences();
//			printf("rDist = %d, aDist = %d Byt = %ld Sent = %ld Fail = %ld\n ", rDistance(), aDistance(), rbytes(), rSentences(), fChkSums());
//		}

		if( checkMoveComp( mem_ptr ) )		//	check if the move is complete
		{
			//	Move is complete...
			clearMoveOKflag( mem_ptr );					//	clear the OK_2_MOVE flag
			clearMoveCompflag( mem_ptr );				//	Clear the MOVE_COMP flag (avoids some scrwiness)
			Range = rDistance();						//	get the raw distance
			setCurrentRnage( mem_ptr, Range );			//	store it in memory
			setDataAvailFlag( mem_ptr );				//	Let processing module know data is available
//		printf("Range = %d |", Range);
			//	store current range to memory array
//			Angle = (int)mem_ptr->currentAngle;
//		printf("Angle = %d\n", Angle);
//			mem_ptr->lastPass[Angle].X = mem_ptr->currentPass[Angle];
//			mem_ptr->currentPass[Angle] = Range;
//			setMoveOkFlag( mem_ptr );				//	let the motor know it's okay to move - moved to computation process, just in case it takes longer than the move time (I really doubt it, but who knows?)
		}
	}
	
	//	okay - we got the DONE flag, flag memory for deletion
	if( shmctl(shmid, IPC_RMID, 0) == -1 )
	{
		printf("big fucking error on shmctl!\n");
		return -1;
	}
	
	printf("Shared memory segment marked for deletion\n");

	//	and the detach from it
//	shmdt();

	//	close the serial port
	serialClose( serStream );
	
	return 0;
}
