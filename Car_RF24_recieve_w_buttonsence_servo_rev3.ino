/
* 
Sending formatted data packet with nRF24L01. 
Maximum size of data struct is 32 bytes.


3 - CE to Shield S pin 7
4 - CSN to Shield S pin 8
5 - SCK to Shield S pin 13
6 - MOSI to Shield S pin 11
7 - MISO to Shield S pin 12
8 - UNUSED

Motot Shield
API220 
2 - GND
1 - VCC 3.3V Bottom near Analog In


trigger shield Pin 4;
echo shield Pin 2;

*/

#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

Servo servorightfront;
Servo servoleftfront;
Servo servorightrear;
Servo servoleftrear;
int pinservorightfront = 10;   // a maximum of eight servo objects can be created but PWM pins must be used
int pinservoleftfront = 9;
int pinservorightrear= 5;
int pinservoleftrear = 3;


int trigPin = 4;
int echoPin = 2; 

long duration, distance;

int button_value = 0;
bool back_flag = false;
int back_start = 5; // when to start the back distance
int back_stop = 20; // when to stop the back distance

int for_speed = 10;  //speed to travel forward and back forward speed = 90 + for_speed --- backspeed = 90 - for_speed 
int side_speed = 5;  //speed to turn right and left 
//right turn ----  right wheels = 90 -  side_speed left wheels = 90 - for_speed
//left turn ----   right wheels = 90 +  side_speed left wheels = 90 + for_speed
//because servo orientation is a mirror image front to back speed setting are not obvious  


int newspeedr = 90; // right speed calculation to pass to servo write function
int newspeedl = 90; // left speed calculation to pass to servo write function

char buf[16];   // buffer for incomming character array 

int serial_putc( char c, FILE * ) // For printf support
{
Serial.write( c );
return c;
}

const uint64_t pipe = 0xE8E8F0F0E1LL;

RF24 radio(7,8);  //RF24 (_cepin, _cspin)

// The sizeof this struct should not exceed 32 bytes
struct MyData {
//  byte byte0;
//  byte byte1;
//  byte byteArray[2];
    int int0;
//  int int1; 
//  int intArray[2];
//  float float0;
//  float float1;
//  float floatArray[2];
    unsigned long packetId; // 16 month rollover at 100 packets/sec
};

/**************************************************/

// Change this when uploading to each Arduino
boolean sender = false;

// Print some info to serial after this number of packets
//unsigned long printRate = 200;

void setup()
{
  Serial.begin(9600);
  
  servorightfront.attach(pinservorightfront);  // attaches the servo to the PWM pins defined above
  servoleftfront.attach(pinservoleftfront);
  servorightrear.attach(pinservorightrear);  
  servoleftrear.attach(pinservoleftrear);

    
  pinMode(trigPin, OUTPUT); //Range finder Trigger Pin assignment and OUTPUT
  pinMode(echoPin, INPUT);  //Range finder Echo Pin assignment and IINPUT

  //fdevopen( &serial_putc, 0 ); // for printf

  // Set up radio module much of code is commented out as this is ONLY a reciever
  radio.begin();
  //radio.setDataRate(RF24_2MBPS); // Both endpoints must have this set the same
  //radio.setAutoAck(false);       // Either endpoint can set to false to disable ACKs
  //radio.setPALevel(RF24_PA_MIN); // Power amplifier level. Also LOW,MED,HIGH (default is HIGH)
  radio.printDetails();

  //if ( sender )
   // radio.openWritingPipe(pipe);
  //else {
    radio.openReadingPipe(1,pipe);
    radio.startListening();
 // }

  int dataSize = sizeof(MyData);
  //printf("Size of MyData: %d\n", dataSize);
 // if ( dataSize > 32 )
  //  printf("*** MyData struct is too large ***\n");

 // printf("This Arduino is set to be a %s\n", sender ? "sender" : "receiver");
}

/**************************************************/
//
//unsigned long nextPacketId = 0;   // Stamp packets with their sequence
//float floatSensor = 0;            // A fake 'sensor' so we can see some data changing
//unsigned long unAckedPackets = 0; // To estimate how many packets were not ACKed
//unsigned long lastPrintTime = 0;  // To measure time between printouts

void sendData() 
{
//code removed recieve only
}

//recieve data function
void recvData()
{
  if ( radio.available() ) {
  MyData data;

    radio.read(&data, sizeof(MyData));

      dtostrf(data.int0, 0, 0, buf);
      button_value = data.int0;    
}
} 
//distance function 
void dist()
{
    
    digitalWrite(trigPin, LOW); 
    delayMicroseconds(2); 
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = (duration/2) / 29.1;
    Serial.print("distance  ");
    Serial.print(distance);
    Serial.print(" cm ");
}

/**************************************************/

void loop()
{

 recvData();//call recieve data from controller to get button value which controls car motion

 dist();//call distance calculation function

   
  if (distance < back_start || back_flag == true ) 
{ 
    button_value = 606;
    back_flag = true;
    Serial.println("Button value = 606 Backing up");
    newspeedr = 90 - for_speed;
    newspeedl = 90 - for_speed;
     
}

   if (distance > back_stop && back_flag == true)
{
  back_flag = false;
  Serial.println(" button_value = 1020 Stopping");
  newspeedr = 90 + side_speed ;
  newspeedl = 90 + side_speed ;
 
      
}
  if (button_value < 1024 && button_value > 1004)
{
  Serial.print("Stop   ");
  newspeedr = 90;
  newspeedl = 90;
  
}

  if (button_value < 832 && button_value > 792)
{
  Serial.print("Forward   ");
   newspeedr = 90 - for_speed;
   newspeedl = 90 + for_speed;    
}    

if (button_value < 626 && button_value > 586)
{
  Serial.print("Back   ");
  newspeedr = 90 + for_speed;
  newspeedl = 90 - for_speed;   
}
if (button_value < 424 && button_value > 386)
{
  Serial.print("Right   ");
  newspeedr = 90 - side_speed;    
  newspeedl = 90 - side_speed; 

}
  if (button_value < 222 && button_value > 182)
{  
  Serial.print("Left   ");

  newspeedr = 90 + side_speed;    
  newspeedl = 90 + side_speed; 

}
  //  90 is the resting value or no speed mid value 0-180

  servorightfront.write(newspeedr);   //  Sets right side wheel speed
  servorightrear.write(newspeedr);    //  Sets right side wheel speed
  servoleftfront.write(newspeedl);    //  Sets left side wheel speed
  servoleftrear.write(newspeedl);     //  Sets left side wheel speed
  Serial.print("  newspeedr=  ");
  Serial.print(newspeedr);
  Serial.print("  newspeedl=  ");
  Serial.println(newspeedl);    
}

    
