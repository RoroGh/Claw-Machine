#include "nRF24L01.h"
#include "RF24.h"
#include "SPI.h"

#define DEBUG

#define CE_PIN  9
#define CSN_PIN 10

#define button_A  2 // Button Yellow - A
#define button_B  3 // Button Blue - B
#define button_C  4 // Button Yellow - C 
#define button_D  5 // Button Blue - D 
#define button_E  7 // SMD button E on pcb
#define button_F  6 // SMD button F on pcb
#define button_joystick 8 // Button in joystick
#define x_axis A0
#define y_axis A1
int buttons[]={ button_A, button_B, button_C, button_D, button_E, button_F, button_joystick };

byte address[][6] = {"pipe1", "pipe2"}; // Set addresses of the 2 pipes for read and write
RF24 radio(CE_PIN,CSN_PIN);
int joystick[9]; // Array holding state of buttons and joystick X- and Y-reading

void setup(){
  for (int i=0; i <7 ; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    digitalWrite(buttons[i], HIGH);  
  }

#ifdef DEBUG
  Serial.begin(38400);
#endif 

  // Setup nRF240...
  radio.begin();
  radio.openWritingPipe(address[0]); // Open writing pipe to address pipe 1
  //radio.openReadingPipe(1, address[1]); // Open reading pipe from address pipe 2
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN); // Set RF power output to minimum: RF24_PA_MIN (change to RF24_PA_MAX if required) 
  radio.setRetries(3,5); // delay, count
  radio.setChannel(110); // Set frequency to channel 110
}

void loop(){
  // Read digital buttons...
  for (int i=0; i <7 ; i++)
    joystick[i] = digitalRead(buttons[i]);

  // Read joystick values...
  joystick[7] = analogRead(x_axis);
  joystick[8] = analogRead(y_axis);

  // Write out values array...
  radio.write(joystick, sizeof(joystick));
  delay(20);

#ifdef DEBUG
  // Log...
  for (int i=0; i <9 ; i++) {
    Serial.print(joystick[i]);
    if (i<8)
      Serial.print(", ");
  }
  Serial.print("\n");
#endif

//  radio.startListening();
//  if (radio.available()) { // Get remote transmission
//    radio.read(&received_value, sizeof(received_value));
//    if (received_value>10) {
//      analogWrite(RECEIVE_LED, 255);
//#ifdef DEBUG
//      Serial.print("received_value=");
//      Serial.println(received_value);
//#endif
//    } 
//  } else
//    analogWrite(RECEIVE_LED, 0);
//  delay(20);
//  radio.stopListening();
}
