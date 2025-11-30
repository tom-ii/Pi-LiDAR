#include <SPI.h>
#include <SD.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <SoftwareSerial.h>
#include <TTFMini.h>
#include <TStepper.h>

//  Ethernet on 10, 11, 12, 13 (11-13 = SPI bus)
//  SD on 4, 11, 12, 13 (11-13 = SPI bus)
//  PC Serial on 0, 1
//  Sensor Serial on 2, 3
//  Motor on 5, 6, 7, 8
//  Home switch on 9

void MoveMotor( void );         //  move the motor 1 step in the desired direction
void Home( void );              //  home the motor to the fully CCW position
void SetDirection( void );      //  update the movement direction
void WriteData( void );         //  writes latest data to the curent data file
void UpdateDataFiles( void );   //  updates names of datafiles for reading/writing
void ReadFileWriteClient( const char *filename ); //  reads data from a file and writes it to the ethernet client

//  Home switch constants
#define HOME            9

//  stepper constants
#define BLUE            5
#define PINK            6
#define YELLOW          7
#define ORANGE          8
#define STEPS_PER_REV   2048
#define RPM             15

//  PC serial constants
//#define Serial   Serial
#define PC_BAUD_RATE 115200

//  parameters for comms with TFMini (which pin is the TFMini tx connectedto?)
#define TF_TX_PIN     2           //  GREEN
#define TF_RX_PIN     3           //  WHITE
#define TF_BAUD_RATE  115200

//  Need to deconflict Ethernet and SD card, as they are both on SPI
#define SS_Eth  10      //  Slave select for ethernet controller
#define SS_SD   4       //  Sleave select for SD card

//  control parameters
#define PAUSE           55      //  minimum ms allowed between sensor moves
#define STEPS_PER_MOVE  3       //  ~0.175 deg per step (2048 full steps per revolution)
#define MAX_ANGLE       150     //  maximum angular travel (home is full CCW)
#define MIN_ANGLE       30

//  mapping constants
#define PLT_WID         1400                      //  (X x Y)
#define PLT_HT          750 
#define XOFFSET         PLT_WID/2                 //  x-center of plot
#define YOFFSET         150                       //  how far "north" is the center of the sweep
#define YCORRECT        PLT_HT-YOFFSET            //  used to compute y coordinates
#define SWP_ANG         (MAX_ANGLE-MIN_ANGLE)     //  half-angle of the sweep
#define OFF_ANG         90-SWP_ANG/2              //  Offset angle for sensor array (sensor angle-half angle)
#define DEG2RAD         3.14159/180.0             //  conversion factor to go from degrees to radians
#define lRANGE          500                       //  length of the beam indicator

//  Set up soft serial port to talk with the sensor
SoftwareSerial  SenSer(TF_TX_PIN, TF_RX_PIN);    //  Set up the SoftSerial HW (Uno RX/TX)

//  Instantiate devices
  //  Stepper Motor
  TStepper motor;

  //  TFMini LiDAR sensor
  TFMini  Sensor;

  // Initialize the Ethernet server library
    // Enter network information below.
    byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x3B, 0x95 };    //  Read this off the sticker on the bottom of your ethernet shield...
//    byte gateway[] = { 192, 168, 68, 1 };                 // the router's gateway address
//    byte subnet[] = { 255, 255, 255, 0 };                 // the subnet
//    IPAddress ip(192, 168, 68, 210);                      // For a static IP, put the address you want in here...

    // with the IP address and port you want to use
    // (port 80 is default for HTTP):
    EthernetServer server(80);
    EthernetClient client;

//  Filenames for use on SD card
File  readFile;
File  writeFile;

const char preludeName[] = "prelude.htm";
const char comma[] = ",";
const char stroke[] = "ctx.stroke();";
const char style1[] = "s1.htm";
const char style2[] = "s2.htm";


//  Global variables - using more than I might otherwise, just so I can keep track of memory tally in compiler
unsigned long LastMoveTime = 0;
unsigned int  Range = 0;
float  Angle = 0;
unsigned int X = 0;
unsigned int Y = 0;
unsigned int lX = 0;
unsigned int lY = 0;
byte  direction = CCW;
boolean currentLineIsBlank=true;
unsigned int fileCount = 0;
char oldDataName[13];           //  filenames for data files
char newDataName[13];
char c = 0; 

void setup() {
  // put your setup code here, to run once:

  // disable the SD card by switching pin 4 high
    //  not using the SD card in this program, but if an SD card is left in the socket,
    //  it may cause a problem with accessing the Ethernet chip, unless disabled
    //  as they both talk over SPI
    pinMode(SS_SD, OUTPUT);
    digitalWrite(SS_SD, HIGH);

  motor.SetConnects( BLUE, PINK, YELLOW, ORANGE );  //  set non-default pins
  motor.Disable( ON );                              //  Disable the stepper (also sets pins to inputs)
  motor.SetRPM(RPM);
  SenSer.begin( TF_BAUD_RATE );                     //  start up soft serial to sensor
  Serial.begin( PC_BAUD_RATE );                      //   start comms with PC via serial

//  Ethernet.begin(mac, ip, gateway, subnet);         //  Initialize the Ethernet Shield
  Ethernet.begin(mac);                              //  Initialize the Ethernet Shield using DHCP
  server.begin();                                   //  Start listening for client connections
  Serial.print( F("Ethernet server is at: ") );     //  Let console know our Ethernet Address
  Serial.println(Ethernet.localIP());

  //  Initialize the SD card
  if (!SD.begin(SS_SD))                             //  Initialize the SD card library (and card) using the SD Chip Select pin specified
  {                                                 //  Returns TRUE on success, FALSE on failure
      Serial.println( F("SD card initialization failed!") );
      return;    // init failed
  }

  //  really need to put a home function in here...
  Home();                                           //  Home the position of the sensor head
  LastMoveTime = millis();

  UpdateDataFiles();
}



void loop() {
  // put your main code here, to run repeatedly:

  //  Every pass, encode data if available
  //  The LIDAR module is sending a firehose of data at us, so just grab it any time
  if( SenSer.available() )              //  if sensor data is available
    Sensor.encode( SenSer.read() );     //    then grab it


  //  Sensor operates at ~100Hz - want to get 5 reads for windows to stabilize before we take a eadng and make move (~50ms)
  if( (millis()-LastMoveTime) > PAUSE )
  {
    
    LastMoveTime = millis();                        //  uUpdate the time
    Range = Sensor.aDistance();                     //  Get the current average distance (change to rDistance for raw data)
//    Range = Sensor.rDistance();                     //  Get the current raw distance (change to aDistance for ave data)
    //  Compute plot values
    ComputeXY( );
    //  Write values to new data file (CSS format so ready to be read to html request)
    WriteData();

    //  if we're at an end of scan, it's time to switch over files (it's a shame there's no "rename" function.  d'oh.
    //    The idea is that we'll display new data in one color, and old data in another.  We have to use SD files for this, 
    //    as the Uno doesn't have nough RAM memory to support the data storage needs
    if( (Angle >= MAX_ANGLE) || (Angle <= MIN_ANGLE) )
    {
      UpdateDataFiles();      
    }

    MoveMotor();                                    //  Move to next motor position    
  }

  //  Check for an ethernet request - if it happens, send back a new web page
  client = server.available();
  
  if (client)                                         //  Check if we have a client request
  {
    Serial.println( F("new client") );                 //  Update the serial terminal
    currentLineIsBlank = true;                        //  An http request ends with a blank line
    
    while (client.connected())                        //  Client is still connected
    {
      if (client.available())                         //  Data is available from the client
      {
        c = client.read();                            //  Read a single byte from the client
//        Serial.write(c);                               //  Print it to the console
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) 
        {
          //  Let console know we're sending html
          Serial.println( F("Sending html") );
          
          // send a standard http response header and beginning of html
          //  Open the file
//          Serial.print( F("Opening ") );                   //  Notify the console
//          Serial.println(preludeName);
          
//          preludeFile.seek(0);                            //  rewind prelude file
            ReadFileWriteClient( preludeName );

          //  we've sent the stuffto set up the CSS canvas and draw scan "braces".
            //  Time tosend the current position of thescan headto make beam indicator
            client.print( lX );                          // send X, Y
            client.print( comma );
            client.print( lY );
            client.println( F(");") );                  //  <!-- Pointer line end -->
            client.println( stroke );                   //  <!--  Draw the line  -->

          //  send the old data's display style
          ReadFileWriteClient( style1 );
          
          //  time to send the old data points
          ReadFileWriteClient( oldDataName );
                   
          //  send the new data's display style
          ReadFileWriteClient( style2 );
          
          //  time to send the new data points
          ReadFileWriteClient( newDataName );

          //  close out the http response
          client.println( F("</script>") );
          client.println( F("</body>") );
          client.println( F("</html>") );
          break;                                              //  all done - break out 
        } //  End if - Time to output html?
        
        // every line of text received from the client ends with \r\n
        if (c == '\n') 
        {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        } 
        else if (c != '\r') 
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      } //  End if - client.available()
    } //  End while - client.connected()
    
    // give the web browser time to receive the data
    delay(1);
    
    // close the connection:
    client.stop();

    //  Let the console know we closed the connection
    Serial.println( F("client disconnected") );
  }
  
}

  

void MoveMotor( void )
{
    motor.Disable( OFF );                           //  Enable the stepper (also sets pins to outputs)
    motor.Step( STEPS_PER_MOVE, direction, FULL );  //  move the sensor

    if( direction == CW )
      Angle -= ((360.0/STEPS_PER_REV) * STEPS_PER_MOVE);
    else
      Angle += ((360.0/STEPS_PER_REV) * STEPS_PER_MOVE);

    SetDirection( );                                //  update the movement direction
    
    //  we should check the activation of a home switch, just in case steps got skipped
    
//    motor.Disable( ON );                            //  Disable the stepper (also sets pins to inputs)

    return;
}

void  Home( void )
{
  //  set home switch to input with pullup ON
  pinMode( HOME, INPUT_PULLUP );                //  New flag (apparently) as of Arduino 1.0.1 (who knew?)
  digitalWrite( HOME, HIGH );

  //  enable stepper motor
  motor.Disable( OFF );                         //  Enable the stepper (also sets pins to outputs)

  //  move CCW until home switch is turned on
  while( digitalRead(HOME) == 1 )               //  pin will go low when switch is engaged
      motor.Step( STEPS_PER_MOVE, CCW, FULL );   //  move the sensor

  Angle = MAX_ANGLE+5;                          //  update the angular position (home = fully CCW, or minimum angle)

  SetDirection( );                              //  update the movement direction

  motor.Disable( ON );                           //  Disable the stepper (also sets pins to inputs)

  return;
}

void SetDirection( void )
{
  if( Angle >= MAX_ANGLE )
    direction = CW;                              //  reverse direction at end of travel

  if( Angle <= MIN_ANGLE )
    direction = CCW;

  return;
}

//  coverts azimuth/range to X,Y (polar to Cartesian) coordinates for display
void ComputeXY( void )
{
  //  This get interesting, as there's no negative coordinates on our grid.
    //  it's 1400x750 (X x Y), with each pixel representing a cm (14m wide, by 7.5m tall)
    //  but the center is at 700,575, or 200 pixels "north" of zero (for looks, maybe I'll
    //  change that later), and the center at positive 700 (allowing +/-700 on either side)
    //  So, too big for a byte, so we're using unsigned ints as out coordinates...

    //  first thing to notice is that conversion from polar to Cartesion involves sines and
    //  and cosines (or sines and sines+90deg).  So...  First cut thinking is to understand the 
    //  angle to the point creates right triangles (projections) to both the X- and Y- axes.
    //  Simple rigonometry tels us that SIN theta = Opp/Hyp, and COS theta = Adj/Hyp.
    //  Some even more simple algebra tells us, then, that Opp = Hyp*sin( @ )

    // now it gets tricky, as sin(0) = 1, and sin(90deg)=0, and then progresses to the 
    //  negative axis - Cosine is similar (and tangent, etc, etc, and so on.  But, their
    //  values are symmetric in the four quadrants (it's a unit circle centered on [0,0], after all)

    //  so, our minimum angular value is (currently) in quadrant 2 (1 degree is actually -60 deg), 
    //  and our maximum angular value is (currently) in quadrant 1 (121 deg is actually +60 deg...

    //  so, the first thing we need to do is compute the proper offset angles for things to look right..

    //  this can be used to skew the "look angle" to the appropriate alignment, but we will still need to 
    //  deal with proper quadrants - that is, >90deg is quad I, and <90deg is quad II

  X = XOFFSET + (Range * cos( Angle*DEG2RAD ));
  Y = YCORRECT - (Range * cos( (90-Angle)*DEG2RAD ));  //  I think using x0s for both computations will save program space

  lX = XOFFSET + (lRANGE * cos( Angle*DEG2RAD ));
  lY = YCORRECT - (lRANGE * cos( (90-Angle)*DEG2RAD ));  //  I think using x0s for both computations will save program space

  return;
}   

//  Write values to new data file (CSS format so ready to be read to html request)
void WriteData( void )
{
    writeFile = SD.open( newDataName, FILE_WRITE );    //  Create (and open) new data file (pointer at EOF)
    writeFile.println( F("ctx.beginPath();") );           //  <!-- Start a path -->
    writeFile.print( F("ctx.arc(") );                   //  <!-- Define the circle arc(X, Y, radius, startAngle - rad, endAngle - rad, <CCW>)
    writeFile.print( X );
    writeFile.print( comma );
    writeFile.print( Y );
    writeFile.print( comma );
    writeFile.print( int(max(1,Range/100)) );
    writeFile.println( F(",0,2*Math.PI);") );
    writeFile.println( stroke );             //  <!-- draw the circle -->
    writeFile.println();
    writeFile.close();

  return;
}

//  updates names of datafiles for reading/writing
void UpdateDataFiles( void )
{
     sprintf( oldDataName, "%d", fileCount );
     sprintf( newDataName, "%d", ++fileCount );

      if( SD.exists( newDataName ) )
        SD.remove( newDataName );

     return;
}

//  reads data from a file and writes it to the ethernet client
void ReadFileWriteClient( const char *filename )
{
  readFile = SD.open( filename, FILE_READ );    
  
  if (readFile)                               //  Test that we got a handle
  {
//    Serial.print(filename);                   //  Notify the console
//    Serial.println( F(" Opened") );
    
    while( readFile.available() )             //  if we're not at EOF
    client.write(readFile.read());          // send web page to client
    
    readFile.close();                         //  All done, close the file
  }
  else  // couldn't open prelude
  {
    Serial.print( F("Failed to open ") );          //  Notify the console
    Serial.println(filename);
    return;                                      //  Failure - break out
  }
  return;
}
