
#include "nRF24L01.h"
#include "RF24.h"
#include "RF24_config.h"
#include "SPI.h"
#define DEBUG // Outcomment for stealth/live operation...
// nRF24L01 defines...
#define CE_PIN  9
#define CSN_PIN 10
int joystick[9]; // Communications array holding state of buttons and joystick X- and Y-reading 
byte address[][6] = {"pipe1", "pipe2"}; // Set addresses of the 2 pipes for read and write

/*
 * SCK 52 on mega, 51 MOSI, 50 MISO
 */

int valX;

//////CONNECT nRF to 3.3V !!!!!!!!!

RF24 radio(CE_PIN,CSN_PIN);

#include <Servo.h> 
Servo myservo;

int d = 300;

void setup() {
  myservo.attach(6);
Serial.begin(38400);

#ifdef DEBUG
    Serial.println("nRF24L01 setup");
#endif

  radio.begin();
  radio.openReadingPipe(1, address[0]); // Open reading pipe from address pipe 1
 // radio.openWritingPipe(address[1]); // Open writing pipe to address pipe 2 
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN); // Set RF power output to minimum: RF24_PA_MIN (change to RF24_PA_MAX if required) 
  radio.setRetries(3,5); // delay, count
  radio.setChannel(110); // Set frequency to channel 110
  delay(2000); // Signal power-up...
  radio.startListening();
  
   myservo.write(100);
}


void loop() {

   if (radio.available()) { 
      radio.read(joystick, sizeof(joystick));
  }
  for (int i=0; i <9 ; i++) {
#ifdef DEBUG
    Serial.print(joystick[i]);
    if (i<8)
      Serial.print(", ");
#endif
  }
#ifdef DEBUG
  Serial.print("\n");
#endif

if ((joystick[1] == LOW || joystick[3] == LOW)) //blue
{  
  Serial.println("here");
   myservo.write(160);
   delay(d);
   myservo.write(100);

}

if ((joystick[0] == LOW || joystick[2] == LOW)) //yellow
    {myservo.write(45);
   delay(d);
   myservo.write(100);}
  
}
