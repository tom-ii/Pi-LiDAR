#include "mem.h"
#include <sys/shm.h>			//	shared memory stuff
#include <stdlib.h>				//	contains exit()
#include <stdio.h>
#include <wiringPi.h>			//	TRUE and FALSE

unsigned char notDone( Rmem_ptr mem_ptr )
{
	unsigned char test = TRUE;
	
	//	check memory for flag
	test = mem_ptr->flags[DONE];
	
	return test ^0x01;	//	return a TRUE value if we're *not* done
}

unsigned char checkDataRead( void )
{
	//	check memory for flag
//	return memory->doneFlag;
	return 1;
}

void setMovedFlag( Rmem_ptr mem_ptr )
{
	mem_ptr->flags[MOVE_COMP] = TRUE;
	
	return;
}

void setCurrentAngle( Rmem_ptr mem_ptr, float Angle )
{
	mem_ptr->currentAngle = Angle;
	
	return;
}

float getCurrentAngle( Rmem_ptr mem_ptr )
{
	return mem_ptr->currentAngle;
}
	
int getCurrentRange( Rmem_ptr mem_ptr )
{
	return mem_ptr->currentRange;	
}

int getShmid( const char *mount_pt, int project_id, int size )
{
	key_t	key;
	int 	shmid = 0;
	
	/* Create unique key via call to get shared memory */
	key = ftok(mount_pt, project_id);
	
	/* Open the shared memory segment - create if necessary */
	if((shmid = shmget(key, size, IPC_CREAT|IPC_EXCL|0666)) == -1) 
	{
		printf("Shared memory segment exists - opening as client\n");

		/* Segment probably already exists - try as a client */
		if((shmid = shmget(key, size, 0)) == -1) 
		{
			perror("shmget");
			exit(1);
		}
	}
	else
	{
		printf("Creating new shared memory segment\n");
	}

	printf("shmid = %d\n", (int)shmid );
	
	return shmid;	
}

void setDoneFlag( Rmem_ptr mem_ptr)			//	tell other processes we're finished
{
	mem_ptr->flags[DONE] = TRUE;
	
	return;
}

void setMoveOkFlag( Rmem_ptr mem_ptr )		//	 set the ANGLE read (past tense) flag - allows motor to move
{
	mem_ptr->flags[OK_2_MOVE] = TRUE;
	
	return;
}

unsigned char readMoveOkFlag( Rmem_ptr mem_ptr )		//	 set the OK_2_MOVE flag - allows motor to move
{
	unsigned char test = FALSE;
	
	//	check memory for flag
	test = mem_ptr->flags[OK_2_MOVE];
	
	return test;	//	return a TRUE value if it's okay to move motor
}

void clearMoveOKflag( Rmem_ptr mem_ptr )		//	clear the MOVE_OK flag
{
	mem_ptr->flags[OK_2_MOVE] = FALSE;
	
	return;
}

unsigned char checkMoveComp( Rmem_ptr mem_ptr )	//	get the value of the MOVE_COMP flag
{
	unsigned char test = FALSE;
	
	//	check memory for flag
	test = mem_ptr->flags[MOVE_COMP];
	
	return test;	//	return a TRUE value if it's okay to move motor
}

void setCurrentRnage( Rmem_ptr mem_ptr, int Range )			//	set the latest range
{
	mem_ptr->currentRange = Range;
	
	return;
}

void clearMoveCompflag( Rmem_ptr mem_ptr )		//	clear the MOVE_COMP flag
{
	mem_ptr->flags[MOVE_COMP] = FALSE;
	
	return;
}

void setDataAvailFlag( Rmem_ptr mem_ptr )				//	set the DATA_AVAIL flag - allows processing of next data
{
	mem_ptr->flags[DATA_AVAIL] = TRUE;
	
	return;
}

unsigned char readMDataAvailFlag( Rmem_ptr mem_ptr )	//	 check the DATA_AVAIL flag
{
	unsigned char test = FALSE;
	
	//	check memory for flag
	test = mem_ptr->flags[DATA_AVAIL];
	
	return test;	//	return a TRUE value if it's okay to move motor
}

void clearDataAvailflag( Rmem_ptr mem_ptr )				//	clear the DATA_AVAIL flag
{
	mem_ptr->flags[DATA_AVAIL] = FALSE;
	
	return;
}


void setMkPgFlag( Rmem_ptr mem_ptr )				//	set the MK_PG flag - allows processing of next data
{
	mem_ptr->flags[MK_PG] = TRUE;
	
	return;
}

unsigned char readMkPgFlag( Rmem_ptr mem_ptr )	//	 check the MK_PG flag
{
	unsigned char test = FALSE;
	
	//	check memory for flag
	test = mem_ptr->flags[MK_PG];
	
	return test;	//	return a TRUE value if it's okay to move motor
}

void clearMkPgFlag( Rmem_ptr mem_ptr )				//	clear the MK_PG flag
{
	mem_ptr->flags[MK_PG] = FALSE;
	
	return;
}
