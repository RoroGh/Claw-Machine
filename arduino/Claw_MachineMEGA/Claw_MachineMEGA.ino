//////////////nRF and joystick

#include "nRF24L01.h"
#include "RF24.h"
#include "RF24_config.h"
#include "SPI.h"
#define DEBUG // Outcomment for stealth/live operation...
// nRF24L01 defines...
#define CE_PIN  7
#define CSN_PIN 8
#define safeRange 50   // Safe range to ignore round midpoint of joystick readings...
int joystick[9]; // Communications array holding state of buttons and joystick X- and Y-reading 
byte address[][6] = {"pipe1", "pipe2"}; // Set addresses of the 2 pipes for read and write

/*
 * SCK 52 on mega, 51 MOSI, 50 MISO
 */

//////CONNECT nRF to 3.3V !!!!!!!!!

RF24 radio(CE_PIN,CSN_PIN);

/////////////LCD

#include <LiquidCrystal.h>
const int rs = 24, en = 25, d4 = 26, d5 = 27;
const int d6 = 28, d7 = 29;


LiquidCrystal lcd (rs, en, d4, d5, d6, d7);

////////////Motors

#include <Servo.h> 
Servo servo1; //X fork
Servo servo2; //Y fork
Servo servo3; //Z fork
Servo servo4; //Gripper

int valX;
int valY;

////////////Claw Variables

int closed = 110;
int opened = 165;

//delay needed for the z fork to go down
int timeNeeded = 1000; 


///////////Ultrasonic

#define trig0 A3
#define echo0 A2
long distance;

#define trigX 34
#define echoX 35
long distanceX;

#define trigY 38
#define echoY 39
long distanceY;


int turn; 
boolean starting; //yellow
boolean stopping; //blue

///////note to self
/* the servo speed 90 to stop, 
 *  135 to go CW,
 *  45 to go CCW 
 * for the joystick:
 * less than 10 means left and down
 * around 670 means up and right
 * 335 is the middle i.e it does not move
 */


void setup() {
  
  Serial.begin(38400);

  turn = 0;
  
  servo1.attach(2);
  servo2.attach(3);
  servo3.attach(4);
  servo4.attach(5);

  pinMode (trig0, OUTPUT);
  pinMode (echo0, INPUT);
  pinMode (trigX, OUTPUT);
  pinMode (echoX, INPUT);
  pinMode (trigY, OUTPUT);
  pinMode (echoY, INPUT);


  
  lcd.begin(16,2); 
  lcd.setCursor(3,0);
  lcd.print("Setting Up");
  lcd.setCursor(0,1);
  lcd.print("Please wait! ^_^");

  /////Wire.setClock(400000); // Set I²C-Frequenz at 400 kHz
  // Setup nRF24L01 communication...
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

  lcd.clear(); 
  lcd.setCursor(3,0);
  lcd.print("Ready for");
  lcd.setCursor(2,1);
  lcd.print("a new player!");
}


void loop() {
  
  while (turn)
  {
    lcd.setCursor(0,0);
    lcd.print(String("You can do it!!"));

    Serial.println("NORMAL");
    
    if (radio.available()) { // Get remote transmission
      radio.read(joystick, sizeof(joystick));
      readingJS(); 
    
    
      stopping = (joystick[1] == LOW || joystick[3] == LOW);

      if (stopping)
      {
        Serial.println("DONE");
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("Checking");
        lcd.setCursor(7,1);
        lcd.print("Results");
        grab();
        delay(500);
        goBackToPos0();
        delay(1000);
        distance = checkingDistance(echo0, trig0);
        delay(100);
        checkWin();
        turn = 0;
        stopping = false;
        Serial.println("END");
        delay(1000);
      }
      else 
      {
        servo1.write(valX);
        servo2.write(valY);
        delay(15);
      }
    }
    else 
    {
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Warning!!!");
      lcd.setCursor(0,1);
      lcd.print("Problem in nRF");
    }
  }

  if (radio.available()) { 
      radio.read(joystick, sizeof(joystick));
  }
  
  starting = (joystick[0] == LOW || joystick[2] == LOW);
  if (starting)
  {
    Serial.println("NEW");
    turn = 1;
    starting = false;
    lcd.begin(16,2); 
    lcd.setCursor(0,0);
    lcd.print("Starting ur turn!");
    lcd.setCursor(3,1);
    lcd.print("Enjoy!! :D");
    delay(2000);
//    pos1 = 0;
//    pos2 = 0;
    lcd.clear();
  }
  delay(50);
}



void readingJS() { 
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

distanceX = checkingDistance(echoX, trigX);

if (joystick[7] < 50) 
{
  Serial.print(distanceX);
  if (distanceX < 5)
    {
    valX = 90;
  }
  else
    valX= 45;
}
else if (joystick[7] > 625) 
{
  Serial.print(distanceX);
  if (distanceX < 45)
  {
    valX = 135;
  }
  else
    valX= 90;
}
else 
  valX = 90;

distanceY = checkingDistance(echoY, trigY);

if (joystick[8] < 50) 
{
  Serial.print(distanceY);
  if (distanceY < 5)
    {
    valY = 90;
    }
  else
    valY= 45;
}
else if (joystick[8] > 625) 
{
  Serial.print(distanceY);
  if (distanceY < 45)
  {
    valY = 135;
  }
  else
    valY= 90;
}
else 
  valY = 90;
}


void checkWin(){
  
  Serial.println("CHECK");
  
  lcd.clear();
  lcd.begin(16,2);
  
  if (distance < 8)
  {
    lcd.setCursor(1,0);
    lcd.print("YOU WON!!!! :D");
    lcd.setCursor(1,1);
      lcd.print("CONGRATS!!! <3");
    for (int i =0; i < 40; i++)
    {
      Serial.println("Win");
      lcd.scrollDisplayRight();
      delay(175);
    }
  }
  else
  {
    lcd.setCursor(5,0);
    lcd.print("SORRY!");
    lcd.setCursor(3,1);
    lcd.print("YOU LOST :(");
    for (int i =0; i < 40; i++)
    {
      Serial.println("lost");
      lcd.scrollDisplayRight();
      delay(175);
    }
  }
  
  delay(1750);
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Waiting for");
  lcd.setCursor(2,1);
  lcd.print("a new player!");
  
}

void goBackToPos0(){
  
  Serial.println("GOBACK");
  
  while (checkingDistance(echoX, trigX)>5) // X fork
  {
     Serial.print("x fork");
     servo1.write(45);
  }
  servo1.write(90);
  while (checkingDistance(echoY, trigY)>5) // Y fork
  {
     Serial.print("y fork");
     servo2.write(45);
  }
  servo2.write(90);
  
  for (int i = closed; i < opened; i++) //the gripper to open
  {
     servo4.write(i);
     delay(15);
  }
}

void grab(){
  
  Serial.println("IN GRAB");

     servo3.write(135);
     delay(timeNeeded);
     servo3.write(90);
     
  for (int i = opened; i > closed; i--) //the gripper to hold
  {
     servo4.write(i);
     delay(15);
  }

  servo3.write(45);
     delay(timeNeeded);
     servo3.write(90);
}

long checkingDistance (int echo, int trig)
{
  Serial.println("checking");
  
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10); //periode d'onde 
  digitalWrite(trig, LOW);
  /* what we do eno bnhadi 7aga basita ba3diha 
  bn5ali ybda2 mn awel onde b periode 10 µs
  ba3diha ta7t el echo hay7sb el wa2t el a5adto*/
  long duration = pulseIn(echo, HIGH); // micro
  /* d = t * V
   distance = duration / 2 (3a4an ray7 w rage3)
   * 344 (vitesse du son) * 100 (m to cm pour v) 
   / 1000000 (µs to s pour t); so we get in cm*/
  return duration * 0.0172; //to get it in cm
}
