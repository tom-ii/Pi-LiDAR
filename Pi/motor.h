#ifndef	__MOTOR_H__
#define __MOTOR_H_

#define BLUE 	24
#define PINK	25
#define YELLOW	26
#define ORANGE	27
 
void setHome( unsigned char pin );
unsigned char motorHome( float *Angle );
unsigned char checkHomed( void );
unsigned char SetDirection( unsigned char dir, float Angle );
float MoveMotor( float Angle, unsigned char direction );
 
#endif
