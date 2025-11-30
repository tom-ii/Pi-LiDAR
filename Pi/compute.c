#include "LiDAR.h"				//	"global" stuff for the project as a whole
#include "mem.c"				//	mem functions for project
//#include <time.h>				//	timing functions
//#include <unistd.h>				//	contains sleep()
#include <stdlib.h>				//	contains exit()
#include "compute.h"
//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <unistd.h>
#include <math.h>				//	contains sin() & cos()
#include <sys/param.h>			//	contains MAX() "function"

/*some pseudocode
 * 1) Connect to (or init) memory space
 * 2) continuosly:
 * 	a) 	When MOVE_COMP flag is set, update data with latest range
 * 	b)	monitor memory for kill flag
 * 	c) 	set OK_2_MOVE flag
 * 
 * 	I think that's it(?)
 */
 
int main(int argc, char *argv[] )
{
	float 			Angle = 0;
	Rmem_ptr		mem_ptr = 0;
	int				shmid = 0;
	int				Range = 0;
//	int				X;
//	int				Y;
	
	//	get shared memory
	shmid = getShmid( configFile, PROJ_ID, sizeof(Rmem_typ) );
	
	/* Attach (map) the shared memory segment into the current process */
	if((mem_ptr = (Rmem_ptr)shmat(shmid, 0, 0)) == (Rmem_ptr)-1)
	{
		perror("shmat");
		exit(1);
	}

	setMoveOkFlag( mem_ptr );				//	let the motor know it's okay to move
	
	// run until we get a kill message
	while( notDone(mem_ptr) )
	{
		//	update the angle pointer
		if( readMDataAvailFlag( mem_ptr ) )		//	check if data is available
		{
			Angle = getCurrentAngle( mem_ptr );
			Range = getCurrentRange( mem_ptr );
			clearDataAvailflag( mem_ptr );	//	We got the data so clear the flag

//			printf("Angle: %f, Range: %d\t", Angle, Range );
			//	first things first - compute the position of the az line
			mem_ptr->currentPass[SENSOR_POS].X = computeX( Angle, 500 );
			mem_ptr->currentPass[SENSOR_POS].Y = computeY( Angle, 500 );
			mem_ptr->currentPass[SENSOR_POS].size = 1;

			//	next, we need to transfer the new data to the old slot
			mem_ptr->lastPass[(int)Angle].X = mem_ptr->currentPass[(int)Angle].X;
			mem_ptr->lastPass[(int)Angle].Y = mem_ptr->currentPass[(int)Angle].Y;
			mem_ptr->lastPass[(int)Angle].size = mem_ptr->currentPass[(int)Angle].size;
			
			//	Lastly, we need to compute the new data
			mem_ptr->currentPass[(int)Angle].X = computeX( Angle, Range );
			mem_ptr->currentPass[(int)Angle].Y = computeY( Angle, Range );
			mem_ptr->currentPass[(int)Angle].size = MAX(MIN_DIA,(int)(Range/100));


			//	...  and finally, tell the motor it's okay to move
			setMoveOkFlag( mem_ptr );				//	let the motor know it's okay to move
			
			//	...	and finally, finally, let PageMaker process know to update page (don't want tohave it rely on DATA_AVAIL flag - might create weirdness)
			setMkPgFlag( mem_ptr );

//			printf("X: %d\tY: %d\tRange: %d\tsize: %d\n", mem_ptr->currentPass[(int)Angle].X, \
//															mem_ptr->currentPass[(int)Angle].Y, \
//															Range, mem_ptr->currentPass[(int)Angle].size);
			
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

	return 0;
}

int computeX( float Angle, int Range )
{
	return (int)((double)XOFFSET + ((double)Range * cos( Angle*DEG2RAD )));
}

int computeY( float Angle, int Range )
{	
	return (int)((double)YCORRECT - (Range * cos( (90-Angle)*DEG2RAD )));  
	
}
