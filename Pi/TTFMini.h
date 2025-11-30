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
	
	unsigned char SetSerial( Stream* TF_Serial );	//	Pointer to serial stream for controlling TFMini
	
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
	unsigned int getAge( void );				//	age of last good data


*/

// ensure this library description is only included once
#ifndef TTFMini_h
#define TTFMini_h

#define VERSION		0.2		//	port for pi
//#define VERSION		0.1		//	Original (Arduino) version

#define	NUM_SAMPLES	5		//	number of samples to average in our window
#define TF_BYTES	9		//	bytes expected in a sensor packet
#define CMD_BYTES	8		//	bytes in a sensor command

#define	TIMEOUT	750			//	ms until data is considered no longer valid (time to read a block from deivce is ~0.6)

//	TFMini packet positin definitions (reversed due to order of shifting)
#define	HDR_1	8
#define	HDR_2	7
#define	DIST_L	6
#define DIST_H	5
#define	STR_L	4
#define	STR_H	3
#define	RES		2
#define	QUAL	1
#define	CHKSUM	0

#define	HDR_VAL	0x59

// library interface description
//class TFMini {
//  public:
    void TFMini( void );    							// constructor
	
	
    float	 version( void );				//	Returns the version
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
	unsigned int getAge( void );				//	age of last good data

//  private:
    unsigned int window( unsigned int *vector, unsigned int val, unsigned char num_samples );		//	Perform a windowed average on data
	void 	 shift_vector_c( unsigned char *vector, unsigned char num_samples, unsigned char data  );	//	Perform a data single left shift on an array of chars & add new data
	void 	 shift_vector_i( unsigned int *vector, unsigned char num_samples, unsigned int data  );			//	Perform a data single left shift on an array of ints & add new data
	unsigned char	 CheckDataValid( unsigned char *Data_vector );							//	Check the validity of the data vector
	unsigned char	 CheckHeader( unsigned char *Data_Vector );								//	check for valid data header
	unsigned char	 CheckChksum( unsigned char *data_Vector );								//	check for valid data checksum
	void 	 UpdateAge( void );													//	update the data age
	unsigned char	 TestAge( unsigned int age );											//	Test data age for validity
	void 	 ParseData( void );													//	Parses fata into private variables

	void	 sendCommand( int stream, const char *command );					//	send a command to the sensor
	const unsigned char setStandardOutputMode[] = {0x42, 0x57, 0x02, 0x00, 0x00, 0x00, 0x01, 0x06};
	const unsigned char setConfigMode[] = {0x42, 0x57, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02};
	const unsigned char setSingleScanMode[] = {0x42, 0x57, 0x02, 0x00, 0x00, 0x00, 0x00, 0x40};
	const unsigned char externalTrigger[] = {0x42, 0x57, 0x02, 0x00, 0x00, 0x00, 0x00, 0x41};
	
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
	unsigned char  Data_vector[ TF_BYTES ];		//	container for raw signal strengths for windowing
	

//};


#endif
