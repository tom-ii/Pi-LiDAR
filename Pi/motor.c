#include <wiringPi.h>
#include "motor.h"

//	set up home switch pin as an input with pull up turned on
void setHome( unsigned char pin )
{
	pinMode( pin, INPUT );
	pullUpDnControl (pin, PUD_UP) ;
	return;
}

unsigned char  motorHome( float *Angle )
{
	//  enable stepper motor
	Disable( OFF );							//  Enable the stepper (also sets pins to outputs)

	//  move CCW until home switch is turned on
	while( checkHomed() == 1 )				//  pin will go low when switch is engaged
	{
//		Step( STEPS_PER_MOVE, CCW, FULL );	//  move the sensor
		MoveMotor( 20, CCW );
//		sleep(1);
	}

	Disable( ON );							//  Disable the stepper (also sets pins to inputs)

	*Angle = MAX_ANGLE+10;					//  update the angular position 
												//	(home = fully CCW, or maximum angle)
	return SetDirection( CCW, *Angle );			//	return the home angle
}

unsigned char checkHomed( void )
{
	return digitalRead(HOME_SW);	//  pin will go low when switch is engaged
}

unsigned char SetDirection( unsigned char dir, float Angle )
{
	if( Angle >= MAX_ANGLE )
	{
//		printf("angle >= max\n");
		return (unsigned char)CW;		//  reverse direction at end of travel
	}

	if( Angle <= MIN_ANGLE )
	{
//		printf("angle <= min\n");
		return (unsigned char)CCW;
	}
    
//    printf("direction = %d\n", (int)dir);
  return dir;
}

float MoveMotor( float Angle, unsigned char direction )
{
	Disable( OFF );									//  Enable the stepper (also sets pins to outputs)
		Step( STEPS_PER_MOVE, direction, HALF );	//  move the sensor

    if( direction == CCW )
      Angle += ((360.0/STEPS_PER_REV) * STEPS_PER_MOVE);
    else
      Angle -= ((360.0/STEPS_PER_REV) * STEPS_PER_MOVE);

//			printf("Angle = %f Dir=%d\n", Angle, (int)direction);
    
    //  we should check the activation of a home switch, just in case steps got skipped
//	if( checkHomed() == 0 )
//		Angle = MIN_ANGLE-5;						//  update the angular position 
	
//	Disable( ON );		//  Disable the stepper (also sets pins to inputs)
									//	Seems to cause issues, so disabled

    return Angle;
}

