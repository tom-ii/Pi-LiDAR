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
 * 	v0.2 -> ported to Pi - 29 Jul 2018
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

 // ensure this library description is only included once
#ifndef TStepper_h
#define TStepper_h

#define VERSION		0.2		//	Current version

//  Define modes
#define WAVE  1
#define FULL  2
#define HALF  3

//  define states
#define	LOW		0
#define	HIGH	1
#define OFF	 	0
#define ON  	1

//  Define directions
#define CW  0   //  Clockwise when looking down at the motor shaft
#define CCW 1

#define SLOPE -958
#define OFFSET  18083

#define DELAY 1800    //  1800 us (~17 rpm) is the minimum delay to reliably run the motor at full steps
                        // 800 for half-stepping (also ~17 rpm) 

// library interface description
//class TStepper {
//  public:
    void TStepper( unsigned int steps );    							// constructor
	
	void SetConnects( unsigned char blue, unsigned char pink, unsigned char yellow, unsigned char orange );	//	initializes connections to stepper
	
    float	version( void );					//	Returns the version
	void	SetRPM( unsigned char RPMs );				//	set the rotational speed
	void	SetStepsPerRev( unsigned int steps );	//	Sets the number of steps per revolution

	void	Disable( unsigned char  command );			//	Disables stepper (lets it freewheel)
	void	Step( unsigned int steps, unsigned char dir, unsigned char mode );	//	steps the given number of steps in the specified direction
																	//	at the specified RPM
																	//	steps = no steps
																	//	dir = CW | CCW
																	//	mode = HALF | WAVE | FULL
																	
//  private:
	void	SetSteps( unsigned char mode, unsigned char phases );	//  sets the phases as needed for a given mode and phas
														//	mode = FULL		phase = current phase
														//			HALF
														//			WAVE
														
	unsigned char	ChoosePhase( unsigned char dir, unsigned char mode );	//	chooses the next phase based on current phase and direction
															//	mode = CW | CCW
															
	//  Driver connections
	unsigned char BLUE;
	unsigned char PINK;
	unsigned char YELLOW;
	unsigned char ORANGE;

	unsigned int stepsPerRev;		//  0.18 deg step angle (full steps)
	float	stepAngle;			
	unsigned char RPM;             	//  minimum is about 1.8 due to limitations of delayMicroseconds() function
	unsigned int SpeedDelay;		//	us delay for pausing between steps for given speed
	
	unsigned char state;		// current position of state machine
	unsigned char phase;		//	current phase
//};

#endif
