/*******************************************************************
 * 
 *	TStepper.h
 *
 *	28BYJ-48 / ULN2003 Stepper motor driver
 * 	Based on https://www.youtube.com/watch?v=B86nqDRskVU
 * 
 *  Tom Burke
 *   tomburkeii@gmail.com
 * 
 * 	v0.2 -> ported to Pi - 29 Jul 2018  (uses wiringPi)
 *  v0.1 -> Arduino library - 23 Jun 2018
 * 
 * GNU copyleft, etc.  Free for use.  Give me some love (please).  
 * no waranties expressed or implied.  Use at your own risk.
 * 
   public:
    TStepper(  );    							// constructor
	
	void SetConnects( unsigned char BLUE, unsigned char PINK, unsigned char YELLOW, unsigned char ORANGE );	//	initializes connections to stepper
	float	version( void );					//	Returns the version
	void	SetRPM( unsigned char RPMs );				//	set the rotational speed
	void	SetStepsPerRev( unsigned int steps );	//	Sets the number of steps per revolution

	void	Disable( unsigned char  command );			//	Disables stepper (lets it freewheel)
	void	Step( unsigned int steps, unsigned char dir, unsigned char mode );	//	steps the given number of steps in the specified direction
																	//	at the specified RPM
																	//	steps = no steps
																	//	dir = CW | CCW
																	//	mode = HALF | WAVE | FULL
																	
  private:
	void	SetSteps( unsigned char mode, unsigned char phases );	//  sets the phases as needed for a given mode and phas
														//	mode = FULL		phase = current phase
														//			HALF
														//			WAVE
														
	unsigned char	ChoosePhase( unsigned char dir, unsigned char mode );	//	chooses the next phase based on current phase and direction
															//	mode = CW | CCW
															
	//  driver connections
	unsigned char BLUE;
	unsigned char PINK;
	unsigned char YELLOW;
	unsigned char ORANGE;

	unsigned int STEPS_PER_REV;		//  0.18 deg step angle (full steps)
	float	stepAngle;			
	unsigned char RPM;             	//  minimum is about 1.8 due to limitations of delayMicroseconds() function
	unsigned int SpeedDelay;		//	us delay for pausing between steps for given speed
	
	static  unsigned char state;		// current position of state machine
 */

#include "TStepper.h"
#include <wiringPi.h>
#include <stdio.h>	//	for printf()
#include <unistd.h>				//	contains sleep()

//	Constructor
void TStepper( unsigned int steps )
{
	//	set the default values
	stepsPerRev = steps;		//  0.18 deg step angle (full steps)
	stepAngle = 360.0/stepsPerRev;
	RPM = 17;             	//  minimum is about 1.8 due to limitations of delayMicroseconds() function
	SpeedDelay = OFFSET+RPM*SLOPE;

	//  Define connections
	BLUE = 8;
	PINK = 9;
	YELLOW = 10;
	ORANGE = 11;
}

//	initializes connections to stepper (not needed if using defaults)
void SetConnects( unsigned char blue, unsigned char pink, unsigned char yellow, unsigned char orange )
{
	BLUE = blue;
	PINK = pink;
	YELLOW = yellow;
	ORANGE = orange;
	
	return;
}

//	Returns the version
float	version( void )
{
	return VERSION;
}

//	set the rotational speed
void	SetRPM( unsigned char RPMs )
{
	RPM = RPMs;
	
	return;
}

//	Sets the number of steps per revolution
void	SetStepsPerRev( unsigned int steps )
{
	stepsPerRev = steps;	
	stepAngle = 360.0/stepsPerRev;

	return;
}


//  Shuts off all outputs
void  Disable( unsigned char  command )
{
  if( command == OFF )
  {
      pinMode( BLUE, OUTPUT );
      pinMode( PINK, OUTPUT );
      pinMode( YELLOW, OUTPUT );
      pinMode( ORANGE, OUTPUT );

      digitalWrite( BLUE, OFF );
      digitalWrite( PINK, OFF );
      digitalWrite( YELLOW, OFF );
      digitalWrite( ORANGE, OFF );
  }
  else
  {
      pinMode( BLUE, INPUT );
      pinMode( PINK, INPUT );
      pinMode( YELLOW, INPUT );
      pinMode( ORANGE, INPUT );

      digitalWrite( BLUE, OFF );
      digitalWrite( PINK, OFF );
      digitalWrite( YELLOW, OFF );
      digitalWrite( ORANGE, OFF );
  }
  
  return;
}

//  Step the number of steps in the specified direction
void  Step( unsigned int steps, unsigned char dir, unsigned char mode )
{
    unsigned long count = 0;

    if( dir == CW )
      for( count=0 ; count<steps ; count++ )
      {
        delayMicroseconds( 1900 );	// consider using micros() function
//        delayMicroseconds( SpeedDelay );	// consider using micros() function
        phase = ChoosePhase( CW, mode );
        SetSteps( mode, phase );
        
      }
    else
      for( count=0 ; count<steps ; count++ )
      {
        delayMicroseconds( 1900 );			// consider using micros() function
        phase = ChoosePhase( CCW, mode );
        SetSteps( mode, phase );
      }

  return;
}

//  sets the phases as needed for a given mode and phase
void  SetSteps( unsigned char mode, unsigned char phases )
{
//	sleep(1);
  switch( mode )
  {
    case WAVE: 
          switch( phases )
          {
            case 1: //  Phasing 1
                  digitalWrite( BLUE, ON );
                  digitalWrite( PINK, OFF );
                  digitalWrite( YELLOW, OFF );
                  digitalWrite( ORANGE, OFF );
                  break;
            case 2: //  Position 2
                  digitalWrite( BLUE, OFF );
                  digitalWrite( PINK, ON );
                  digitalWrite( YELLOW, OFF );
                  digitalWrite( ORANGE, OFF );
                  break;
            case 3: //  Position 3
                  digitalWrite( BLUE, OFF );
                  digitalWrite( PINK, OFF );
                  digitalWrite( YELLOW, ON );
                  digitalWrite( ORANGE, OFF );
                  break;
            case 4: //  Position 4
                  digitalWrite( BLUE, OFF );
                  digitalWrite( PINK, OFF );
                  digitalWrite( YELLOW, OFF );
                  digitalWrite( ORANGE, ON );
                  break;
             default: //  We shouldn't be here...
                  Disable( ON );      //  Shut the motor off...
                  printf("ouch wave...\n");
                  break;
          }
          break;
    case FULL:
          switch( phases )
          {
            case 1: //  Phasing 1
                  digitalWrite( BLUE, ON );
                  digitalWrite( PINK, ON );
                  digitalWrite( YELLOW, OFF );
                  digitalWrite( ORANGE, OFF );
                  break;
            case 2: //  Position 2
                  digitalWrite( BLUE, OFF );
                  digitalWrite( PINK, ON );
                  digitalWrite( YELLOW, ON );
                  digitalWrite( ORANGE, OFF );
                  break;
            case 3: //  Position 3
                  digitalWrite( BLUE, OFF );
                  digitalWrite( PINK, OFF );
                  digitalWrite( YELLOW, ON );
                  digitalWrite( ORANGE, ON );
                  break;
            case 4: //  Position 4
                  digitalWrite( BLUE, ON );
                  digitalWrite( PINK, OFF );
                  digitalWrite( YELLOW, OFF );
                  digitalWrite( ORANGE, ON );
                  break;
             default: //  We shouldn't be here...
                  Disable( ON );      //  Shut the motor off...
                  printf("ouch Full...\n");
                  break;
          }
          break;
    case HALF:
          switch( phases )
          {
            case 1: //  Phasing 1
                  digitalWrite( BLUE, ON );
                  digitalWrite( PINK, OFF );
                  digitalWrite( YELLOW, OFF );
                  digitalWrite( ORANGE, OFF );
                  break;
            case 2: //  Position 2
                  digitalWrite( BLUE, ON );
                  digitalWrite( PINK, ON );
                  digitalWrite( YELLOW, OFF );
                  digitalWrite( ORANGE, OFF );
                  break;
            case 3: //  Position 3
                  digitalWrite( BLUE, OFF );
                  digitalWrite( PINK, ON );
                  digitalWrite( YELLOW, OFF );
                  digitalWrite( ORANGE, OFF );
                  break;
            case 4: //  Position 4
                  digitalWrite( BLUE, OFF );
                  digitalWrite( PINK, ON );
                  digitalWrite( YELLOW, ON );
                  digitalWrite( ORANGE, OFF );
                  break;
            case 5: //  Phasing 5
                  digitalWrite( BLUE, OFF );
                  digitalWrite( PINK, OFF );
                  digitalWrite( YELLOW, ON );
                  digitalWrite( ORANGE, OFF );
                  break;
            case 6: //  Position 6
                  digitalWrite( BLUE, OFF );
                  digitalWrite( PINK, OFF );
                  digitalWrite( YELLOW, ON );
                  digitalWrite( ORANGE, ON );
                  break;
            case 7: //  Position 7
                  digitalWrite( BLUE, OFF );
                  digitalWrite( PINK, OFF );
                  digitalWrite( YELLOW, OFF );
                  digitalWrite( ORANGE, ON );
                  break;
            case 8: //  Position 8
                  digitalWrite( BLUE, ON );
                  digitalWrite( PINK, OFF );
                  digitalWrite( YELLOW, OFF );
                  digitalWrite( ORANGE, ON );
                  break;
             default: //  We shouldn't be here...
                  Disable( ON );      //  Shut the motor off...
                  printf("ouch half...\n");
                  break;
		  }
		  break;
    default: //  We shouldn't be here...
			Disable( ON );      //  Shut the motor off...
			printf("ouch wave...\n");
			break;
  }
}

//	choose the next phase, based on current phase and direction
unsigned char  ChoosePhase( unsigned char dir, unsigned char mode )
{
  if( dir == CW )
    state++;    //  increment the phase
  else
    state--;    //  decrement the phase

  if( mode == HALF )
    return (state%8)+1;  //  half-stepping has 8 phases
  else
    return (state%4)+1;  //  make sure we're in range
}
