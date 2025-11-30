#include "LiDAR.h"				//	"global" stuff for the project as a whole
#include "mem.c"				//	mem functions for project
#include <unistd.h>				//	contains sleep()
#include <stdio.h>				//	printf, etc.
#include <stdlib.h>				//	contains exit()
#include <time.h>				//	timing functions


/*some pseudocode
 * 3) connect to (or init) memory space
 * wait 10 s
 * set done flag
 * exit
 * 
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
	Rmem_ptr		mem_ptr = 0;
	int				shmid = 0;
	
	//	get shared memory
	shmid = getShmid( configFile, PROJ_ID, sizeof(Rmem_typ) );
	
	/* Attach (map) the shared memory segment into the current process */
	if((mem_ptr = (Rmem_ptr)shmat(shmid, 0, 0)) == (Rmem_ptr)-1)
	{
		perror("shmat");
		exit(1);
	}
	
	//	repeatedly set sensor read_bit
	for( int i=0 ; i< 180 ; i++ )
	{
		printf("Angle = %f\n", getCurrentAngle(mem_ptr) );
		sleep(1);
		if( !readMoveOkFlag(mem_ptr) )
			setMoveOkFlag( mem_ptr );
	}

	//	sleepfor 10 seconds
	sleep(10);

	//	send the DONE message
	setDoneFlag( mem_ptr );
	
	
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


