#ifndef __MEM_C__
#define	__MEM_C__

//	memory stuff
#define PROJ_ID		6942
const char configFile[] = ".\\LiDAR.conf";
#define NUM_FLAGS	5
#define	DATA_AVAIL	0
#define	MOVE_COMP	1
#define	OK_2_MOVE	2
#define DONE		3
#define	MK_PG		4

#define SENSOR_POS	0		//	array location that we're keeping the data for drawing the az line

typedef struct data
	{
		unsigned int	X;
		unsigned int	Y;
		unsigned int	size;
	}data_typ, *data_ptr;


typedef struct Rmem
	{
		unsigned char	flags[NUM_FLAGS];		//	flags - see definitions above
		float		 	currentAngle;
		unsigned int	currentRange;
		data_typ		currentPass[NUM_DATAPTS];
		data_typ		lastPass[NUM_DATAPTS];
		int				cmRange;
	}Rmem_typ, *Rmem_ptr;

int getShmid( const char *path, int key, int size );//	get shmid

unsigned char notDone( Rmem_ptr mem );				//	check if we're exiting out or not
void setDoneFlag( Rmem_ptr mem) ;					//	tell other processes we're finished

void setMovedFlag( Rmem_ptr mem );					//	motoor has moved a step
void clearMoveCompflag( Rmem_ptr mem_ptr );			//	clear the MOVE_COMP flag - is a move completed?
unsigned char checkMoveComp( Rmem_ptr mem_ptr );	//	get the value of the MOVE_COMP flag

void setMoveOkFlag( Rmem_ptr mem_ptr );				//	set the OK_2_MOVE flag - allows motor to move
unsigned char readMoveOkFlag( Rmem_ptr mem_ptr );	//	get the OK_2_MOVE flag - is the motor allowed to move?
void clearMoveOKflag( Rmem_ptr mem_ptr );			//	clear the OK_2_MOVE flag

void setCurrentAngle( Rmem_ptr mem_ptr, float Angle );	//	updatwe the current pointing angle
float getCurrentAngle( Rmem_ptr mem_ptr );			//	get the current move angle

void setCurrentRnage( Rmem_ptr mem_ptr, int Range );//	set the latest range
int getCurrentRange( Rmem_ptr mem_ptr );			//	get the current move angle

void setDataAvailFlag( Rmem_ptr mem_ptr );				//	set the DATA_AVAIL flag - allows processing of next data
unsigned char readMDataAvailFlag( Rmem_ptr mem_ptr );	//	 check the DATA_AVAIL flag
void clearDataAvailflag( Rmem_ptr mem_ptr );			//	clear the DATA_AVAIL flag

void setMkPgFlag( Rmem_ptr mem_ptr );				//	set the MK_PG flag - allows processing of next data
unsigned char readMkPgFlag( Rmem_ptr mem_ptr );	//	 check the MK_PG flag
void clearMkPgFlag( Rmem_ptr mem_ptr );			//	clear the MK_PG flag

unsigned char checkDataRead( void );				//	need to update this to have mem ptr as arg

#endif
