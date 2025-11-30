/*  
	TFMini.h - Benwake TFmini Infrared ToF LiDAR module
	
	Original library        (0.1)   by Tom Burke

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

	Optical characteristics:
		2.3 deg beam angle
		minimum object @2m = 20mm (on a side, I assume)
		5mm distance resolution
		850nm wavelength
		max distance 12m @ 90% reflectivity
					  5m @ 10%
					  7m @ 70klux environemnt
					  3m @ 100klux 
					  
	Honestly, I think this thing is likely a phase detection device, not ToF, but what do I know?
	
	The TFMini operates over CMOS Serial, 8N1, 115200 baud
	
	9-byte protocol
	1: 0x59 - frame header, all frames the same
	2: 0x59 - frame header, all frames the same
	3: Dist_L - low byte of distance (cm) in HEX (e.g. Distance 1000 cm, Dist_L = E8)
	4: Dist_H - high byte of distance (cm) in HEX (e.g. Distance 1000 cm, Dist_L = 03)
	5: Strength_L - low byte of signal strength
	6: Strength_H - high byte of signal strength
	7: RESERVED
	8: Signal Quality
	9: Checksum - add first 8 bytes and take the lower 8 bits
	
public:
    TFMini(  );    							// constructor
	
	unsigned char SetSerial( Stream* sPtr );			//	Pointer to serial stream for controlling TFMini
	
    float    version( void );				//	Returns the version
	void 	 encode( unsigned char inData );		//	Encodes a data stream
	unsigned int rDistance( void );				//	Returns the (raw) distance (in cm)
	unsigned int aDistance( void );				//	Returns the (windowed aver) distance (in cm)
	unsigned int rStrength( void );				//	Returns the (raw) signal strength
	unsigned int aStrength( void );				//	Returns the (windowed aver) signal strength
	unsigned char	 SignalQuality( void );			//	Returns the raw signal quality
	unsigned char	 isValid( void );				//	Returns the validity of current data (based on age of last good read)
	unsigned long int rSentences( void );			//	Returns the number of received sentences
	unsigned long int rbytes( void );				//	Returns the number of received bytes
	unsigned long int fChkSums( void );				//	Returns the number of bad checksums
	unsigned int getAge( void );				//	Returns the age of last good data

private:
    unsigned int window( unsigned int *vector, unsigned int val, unsigned char num_samples );		//	Perform a windowed average on data
	void 	 shift_vector_c( unsigned char *vector, unsigned char num_samples  );			//	Perform a data single left shift on an array of chars
	void 	 shift_vector_i( unsigned int *vector, unsigned char num_samples  );			//	Perform a data single left shift on an array of ints
	unsigned char	 CheckDataValid( unsigned char *Data_vector );							//	Check the validity of the data vector
	unsigned char	 CheckHeader( unsigned char *Data_Vector );								//	check for valid data header
	unsigned char	 CheckChksum( unsigned char *data_Vector );								//	check for valid data checksum
	void 	 UpdateAge( void );													//	update the data age
	unsigned char	 TestAge( unsigned int age );											//	Test data age for validity
	void 	 ParseData( void );													//	Parses fata into private variables

	void 	 setConfigMode( void ); 											// Set configuration mode
	void 	 setStandardOutputMode( void );										//	Set to standard output mode
	void  	 setSingleScanMode( void ) ;										// Set single scan mode (external trigger)
	void 	 externalTrigger( void );											// Send external trigger
	
    unsigned int raw_distance;		// Distance to object
    unsigned int ave_distance;
    unsigned int raw_strength; 		//	Signal strength	
    unsigned int ave_strength;
	unsigned char  sig_qual;			//	signal quality
	
	unsigned long int rxd_bytes;			//	Number of bytes received
	unsigned long int rxd_sentences;		//	Number of sentences received
	unsigned long int num_chksum_fail;	//	number of failed checksums
	unsigned char     valid;				//	Validity of current data
	unsigned int age;				//	age of current data (ms)
	unsigned long int old_age_time;		//	time the last good data was taken (ms from reset)
	
	unsigned int Dist_vector[ NUM_SAMPLES ];	//	container for raw distances for windowing
	unsigned int Str_vector[ NUM_SAMPLES ];		//	container for raw signal strengths for windowing
	unsigned char  Data_vector[ TF_BYTES ];	//	container for raw signal strengths for windowing
	
	Stream*	 TF_Serial;			//	pointer to serial stream...  Only needed if we want to use something besides default settings
*/

#include "TTFMini.h"
#include <wiringSerial.h>

//	constructor
void TFMini( void )
{
	int i;
	
	//	initialize private variables
	raw_distance = 0;
    ave_distance = 0;
	raw_strength = 0;
	ave_strength = 0;
	
	rxd_bytes = 0;
	rxd_sentences = 0;
	num_chksum_fail = 0;
	valid = FALSE;
	age = 1000;
	old_age_time = 0;
	
	sig_qual = 0;
	
	old_age_time = millis();
	
	for( i=0 ; i<NUM_SAMPLES ; i++ )
	{
		Dist_vector[ i ] = 0;
		Str_vector[ i ] = 0;
	}
	
	for( i=0 ; i<TF_BYTES ; i++ )
		Data_vector[ i ] = 0;
	
	return;
}

//	return software version
float Tversion( void )
{
	return VERSION;
}

//	encode data into structures - call this with newly received data
void encode( unsigned char SerData )
{
	shift_vector_c( Data_vector, TF_BYTES, SerData );	//	Shift the data vector 1 byte & insert new data
	rxd_bytes++;										//	increment bytes received
	if( CheckDataValid( Data_vector ) )					//	Chec for valid data from range finder
		ParseData(  );									//	Parse the data into our class elements
	
	UpdateAge();										//	update the data age
	valid = TestAge( age );								//	Test validity of data against timeout value
	
	return;
}

//	report the current raw distance
unsigned int rDistance( void )
{
	UpdateAge();						//	update the data age
	return raw_distance;
}

//	report the current average distance
unsigned int aDistance( void )
{
	UpdateAge();						//	update the data age
	return ave_distance;
}

//	report the current raw signal strength
unsigned int rStrength( void )
{
	UpdateAge();						//	update the data age
	return raw_strength;
}

//	report the current average signal strength
unsigned int aStrength( void )
{
	UpdateAge();						//	update the data age
	return ave_strength;
}

//	report the current raw signal quality
unsigned char SignalQuality( void )
{
	UpdateAge();						//	update the data age
	return sig_qual;
}

//	report the latest validity calculation
unsigned char isValid( void )
{
	UpdateAge();						//	update the data age
	return valid;
}

//	report the current data age
unsigned int getAge( void )
{
	UpdateAge();						//	update the data age
	return age;
}

//	Perform a windowed average on data
unsigned int window( unsigned int *vector, unsigned int val, unsigned char num_samples )
{
	unsigned char i = 0;
	unsigned long int sum = 0;					//	uint32 to ensure there's space for everything...
	unsigned int average = 0;
	
	shift_vector_i( vector, num_samples, val  );		//	shift the vector values for windowing
	
	for( i=0; i<num_samples; i++ )		//	Compute the sum
		sum += vector[ i ];
	
	average = sum/num_samples;			//	Compute the average

	UpdateAge();						//	update the data age
	
	return average;
}		

//	Perform a data single left shift on an array of chars
void shift_vector_c( unsigned char *vector, unsigned char num_samples, unsigned char data  )
{
	unsigned char i;
	
	for( i=num_samples-1 ; i>0 ; i--)	//	Move the data entries one "step" left 
		vector[ i ] = vector[ i-1 ];	//		(0th position is considered furthest to the right)
	
	vector[ 0 ] = data;					//	add data to the vector
	
	UpdateAge();						//	update the data age

	return;
}

//	Perform a data single left shift on an array of ints
void shift_vector_i( unsigned int *vector, unsigned char num_samples, unsigned int data  )
{
	unsigned char i;
	
	for( i=num_samples-1 ; i>0 ; i--)	//	Move the data entries one "step" left 
		vector[ i ] = vector[ i-1 ];	//		(0th position is considered furthest to the right)
	
	vector[ 0 ] = data;					//	add data to the vector
	
	UpdateAge();						//	update the data age

	return;
}		

//	Check the validity of the data vector
unsigned char CheckDataValid( unsigned char *Data_vector )	
{
	return ( CheckHeader(Data_vector) && CheckChksum(Data_vector) );
}						

//	check for valid data header
unsigned char CheckHeader( unsigned char *Data_Vector )
{
	unsigned char test = FALSE;
	
	if( Data_Vector[HDR_1] == HDR_VAL )
		if( Data_Vector[HDR_2] == HDR_VAL )
		{
			test = TRUE;						//	If both header bytes = HDR_VAL, then we have a valid header
			rxd_sentences++;

		}
		
	UpdateAge();								//	update the data age
	
	return test;	
}								

//	check for valid data checksum
unsigned char CheckChksum( unsigned char *Data_Vector )
{
	unsigned int checksum = 0;
	unsigned char byte = 0;
	unsigned char test = FALSE;
	
	for( byte=1; byte<TF_BYTES ; byte++ )
		checksum += Data_Vector[ byte ];
	
	if( ( (unsigned char)(checksum & 0x00FF) == Data_Vector[CHKSUM] ))
		test = TRUE;

	if( !test )
		num_chksum_fail++;
	
	UpdateAge();
	return test;
}							

//	update the data age
void UpdateAge( void )				
{
	age = millis() - old_age_time;
	
	return;
}					

//	Test data age for validity
unsigned char TestAge( unsigned int age )
{
	return (age <= TIMEOUT);
}								

//	Returns the number of received sentences
unsigned long int rSentences( void )
{
	UpdateAge();						//	update the data age
	return rxd_sentences;
}	

//	Returns the number of received bytes
unsigned long int rbytes( void )
{
	UpdateAge();						//	update the data age
	return rxd_bytes;
}		


//	Returns the number of bad checksums
unsigned long int fChkSums( void )
{
	UpdateAge();						//	update the data age
	return num_chksum_fail;
}		

//	parse received data into our various elements
void ParseData( void )
{
	age = 0;																	//	reset the age
	old_age_time = millis();													//	get the current time

	raw_distance = ( (Data_vector[ DIST_H ] << 8) | Data_vector[ DIST_L ] ); 	//	form distance
	raw_strength =  ( (Data_vector[ STR_H ] << 8) | Data_vector[ STR_L ] ); 	//	form signal strength
	sig_qual = Data_vector[ QUAL ];												//	get signal quality
	
	ave_distance = window( Dist_vector, raw_distance, NUM_SAMPLES);				//	compute the average distance
	ave_strength = window( Str_vector, raw_strength, NUM_SAMPLES);				//	compute the average signal strength
	
	valid = TRUE;			//	data is now valid
	
	return;
}

void sendCommand( int stream, const char *command )
{
	for( int i=0 ; i<CMD_BYTES ; i++ )
		serialPutchar( stream, command[i] );
		
	return;		
}
