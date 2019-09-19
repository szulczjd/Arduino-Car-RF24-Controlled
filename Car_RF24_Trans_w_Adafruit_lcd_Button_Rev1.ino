 /* 
Sending formatted data packet with nRF24L01. 
Maximum size of data struct is 32 bytes.

1 - GND
2 - VCC 3.3V !!! NOT 5V
3 - CE to Arduino pin 9
4 - CSN to Arduino pin 10
5 - SCK to Arduino pin 13
6 - MOSI to Arduino pin 11
7 - MISO to Arduino pin 12
8 - UNUSED


 Adafruit LCD shield
  Bottom PIN starting from right 

  1 -- SCL
  2 -- SDA
  8 -- GND
  10 -- 5V

 *  Libraries
 *  ---------
 *  - SPI
 *  - RF24
 *  Adafruit_RGBLCDShield
 *  utility/Adafruit_MCP23017
*/
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <SPI.h>

#include <RF24.h>
#include <LiquidCrystal_I2C.h>


Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#define RED 0x1

int serial_putc( char c, FILE * ) // For printf support 
{
  Serial.write( c );
  return c;
}

const uint64_t pipe = 0xE8E8F0F0E1LL;

RF24 radio(9, 10);  

// The sizeof this struct should not exceed 32 bytes
struct MyData {
//  byte byte0;  //Only used for 1 int data transmit uncomment in other data types are needed
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


boolean sender = true;  // Change this when uploading to each Arduino


unsigned long printRate = 200;  // Print some info to serial after this number of packets

void setup()
{

  lcd.begin(16, 2);   // set up the LCD's number of columns and rows: 
  lcd.setBacklight(RED);
  lcd.setCursor(0, 0);
  lcd.print("LCD Operating");

  Serial.begin(9600);
  fdevopen( &serial_putc, 0 ); // for printf

  radio.begin(); // Set up radio module
  //radio.setDataRate(RF24_2MBPS); // Both endpoints must have this set the same
  //radio.setAutoAck(false);       // Either endpoint can set to false to disable ACKs
  //radio.setPALevel(RF24_PA_MIN); // Power amplifier level. Also LOW,MED,HIGH (default is HIGH)
   radio.printDetails();

  if ( sender )
    radio.openWritingPipe(pipe);
  else {
    radio.openReadingPipe(1,pipe);
    radio.startListening();
  }

  int dataSize = sizeof(MyData);
  printf("Size of MyData: %d\n", dataSize);
  if ( dataSize > 32 )
    printf("*** MyData struct is too large ***\n");

  printf("This Arduino is set to be a %s\n", sender ? "sender" : "receiver");
}

/**************************************************/

  unsigned long nextPacketId = 0;   // Stamp packets with their sequence
  int button_value = 0;            // pushbutton input 
  unsigned long unAckedPackets = 0; // To estimate how many packets were not ACKed
  unsigned long lastPrintTime = 0;  // To measure time between printouts
  
  void sendData() 
  {
    MyData data;
  
    // Set properties of your data here...
    data.int0 = button_value; //assigning sensor values to variable in Mydata array
    
    data.packetId = nextPacketId;
    
  
  unsigned long beforeWrite = millis(); // Record a before and after time to see how long the write takes

  radio.write(&data, sizeof(MyData));

  unsigned long afterWrite = millis();

  // Although the RF24 documentation states that the timeout for 
  // receiving an ACK is 60ms, in my experience it seems to wait
  // for only about 2-3ms. The value 2 here was chosen by printing
  // out the time taken for 1000 writes when the receiver was non
  // existent (ie. all packets dropped) and this was around 2788ms
  // between two Arduino UNOs. So I figure that any writes taking
  // 3ms or more have likely been abandoned without receiving any
  // ACK. Note that this does not mean the packet was not actually
  // received at the other end, it just means that the sender could
  // not confirm that the packet arrived.
  if ( afterWrite - beforeWrite > 2 ) 
    unAckedPackets++;

  // Every n packets, print out some info
  if ( data.packetId % printRate == 0 ) {

    if ( data.packetId > 0 ) {
      
      // Use bytes/msec as an approximate kbytes/sec value
      unsigned long now = millis();      
      unsigned long packetsec = ((printRate - unAckedPackets) * 1000) / (now - lastPrintTime);
      int kbsec = (sizeof(MyData) * (printRate - unAckedPackets)) / (now - lastPrintTime);
      lastPrintTime = now;

      // Format the data for printing
      char buf[2]; // buf size depends on data being transfered
      dtostrf(data.int0, 0, 0, buf);

      // As mentioned above, the dropped count is only a rough estimate.
      // You may see some non-zero values even when no packets were really
      // dropped, but it's not bad as a rough guide. You may need to alter
      // the value 2 used above depending on the speed of the microprocessors
      // you are using, to get a better estimate.
      printf("Wrote %ld packets, last id = %ld, int0 = %s, unAcked %ld, %ld packets/sec (%d kbyte/sec)\n", printRate, data.packetId, buf, unAckedPackets, packetsec, kbsec);
    }
    
    unAckedPackets = 0;
  }

  // Increment the packet id for next time
  nextPacketId++;

  uint8_t buttons = lcd.readButtons();

    if (buttons) {
      lcd.clear();
      lcd.setCursor(0,0);
    if (buttons & BUTTON_SELECT) {
      lcd.print("STOP ");
      lcd.setBacklight(RED);
      button_value = 1020;
      lcd.setCursor(0,1);
      lcd.print(button_value);
    }
    if (buttons & BUTTON_UP) {
      lcd.print("FORWARD ");
      lcd.setBacklight(RED);
      button_value = 810;
      lcd.setCursor(0,1);
      lcd.print(button_value);
    }
    if (buttons & BUTTON_DOWN) {
      lcd.print("BACKWARD");
      lcd.setBacklight(RED);
      button_value = 600;
      lcd.setCursor(0,1);
      lcd.print(button_value);
    }

    if (buttons & BUTTON_RIGHT) {
      lcd.print("RIGHT ");
      lcd.setBacklight(RED);
      button_value = 400;
      lcd.setCursor(0,1);
      lcd.print(button_value);
    }
    if (buttons & BUTTON_LEFT) {
      lcd.print("LEFT ");
      lcd.setBacklight(RED);
      button_value = 200;
      lcd.setCursor(0,1);
      lcd.print(button_value);
    }    
   
  }
  
}

/**************************************************/

unsigned long packetsRead = 0;      // Counts total packets read
unsigned long expectedPacketId = 0; // To track whether the packet id was what we expected
int missedPackets = 0;              // How many packets missed since last printout

void recvData()
{
//Code removed for transmit only
}

/**************************************************/

void loop()
{
  if ( sender )
    sendData();
  else
    recvData();
}
