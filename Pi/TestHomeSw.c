#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>			//	contains sleep()

//#define	HOME_SW	0		//	pi pin
#define	HOME_SW	17		//	gpio pin	- this matches the numbering on the T-shield
//#define	HOME_SW	11		//	physical pin

int main( int argc, char*argv[] )
{
//	char	status;
	
//	wiringPiSetup();		//	set up using wiPi numbering
	wiringPiSetupGpio();	//	set up using gpio numbering
//	wiringPiSetupPhys();	//	set up using physical pin numbering
	
	//	set up home switch pin as an input with pull up turned on
	pinMode( HOME_SW, INPUT );
	pullUpDnControl (HOME_SW, PUD_UP) ;
	
	while(1)
	{
		if( digitalRead(HOME_SW) )
			printf("switch: 1\n");
		else
			printf("switch: 0\n");
			
		sleep(1);
	}
}
