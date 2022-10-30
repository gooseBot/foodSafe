//copy libraries to below if needed.  Should work in local libraries dir but didnt!
// C:\Users\Eric\Documents\Arduino\libraries
#include <Servo.h> 
#include "JeeLib.h"
#include <Wire.h>
#include "RTClib.h" 
#include "Time.h"
#include <SoftwareSerial.h>
#include <serLCD.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
ISR(WDT_vect) { Sleepy::watchdogEvent(); }
struct appConfig
{
  DateTime openTime;
};
typedef struct appConfig AppConfig;
AppConfig _myConfig;
byte _lockButton=2;
  
void setup() 
{ 
  //calcMin2UnlockTime();  //use when debugging, run once to make a shorter timeout work
  EEPROM_readAnything(0, _myConfig);
  DateTime now = getTimeDate();

  if (now.unixtime() > _myConfig.openTime.unixtime()){
    openDoor(true);
    displayCountDown(0);
  } else {
    int min2unlock = (_myConfig.openTime.unixtime()-now.unixtime())/60L;
    lockDoorForDuration(min2unlock);    
  }
  pinMode(_lockButton, INPUT_PULLUP);  //pull signal for button high
} 
 
void loop() 
{ 
  //if button pushed then lock the door and wait for the lock duration to end
  int sensorVal = digitalRead(_lockButton);
  if (sensorVal == LOW) {
    int min2unlock=calcMin2UnlockTime();
    printCurrentTime(min2unlock);
    lockDoorForDuration(min2unlock);
  }   
}

void myDelay(unsigned long mseconds) {
  // this delay keeps the arduino working, built in delay stops most activity
  //   this type of delay seems to be needed to make the servo libarary work
  //   not sure why.
  unsigned long starttime = millis();   //going to count for a fixed time
  unsigned long endtime = starttime;
  while ((endtime - starttime) <= mseconds) // do the loop
  {
    endtime = millis();                  //keep the arduino awake.
  }  
}

void openDoor(boolean openLock) {
  static Servo myservo;  // create servo object to control a servo
  byte _doorServo=7;
  byte _open=0;
  byte _close=100;
  myservo.attach(_doorServo,544,2400);  
  if (openLock) {
    myservo.write(_open); 
  } else {
    myservo.write(_close);    
  }
  myDelay(300);
  myservo.detach();     
}

void printCurrentTime(int min2unlock) {
  DateTime now = getTimeDate();
  Serial.begin(57600);
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  Serial.print(min2unlock, DEC);
  Serial.println(); 
  Serial.end();
}

void lockDoorForDuration(int numMinutes) {
  openDoor(false);   //lock door
  for (byte i = 0; i < numMinutes; ++i){ 
    displayCountDown(numMinutes-i);  
    printCurrentTime(numMinutes-1);
    Sleepy::loseSomeTime(57000);   // this low power sleep can only last 1 min.
  }
  displayCountDown(0);
  openDoor(true);    //open door
}

int calcMin2UnlockTime() {  
  unsigned long seconds2open;
  DateTime openTime;
  
  DateTime now = getTimeDate();
  openTime = now +(2*60*60);   //move forward 2 hours
  //openTime = now +(5*60);        //move forward some min
  _myConfig.openTime = openTime;
  EEPROM_writeAnything(0, _myConfig);

  seconds2open=openTime.unixtime()-now.unixtime();   
  return seconds2open/60;
}

DateTime getTimeDate() {
  RTC_DS1307 RTC;
  Wire.begin();
  RTC.begin();  
  pinMode (A3, OUTPUT);              //rtc power 5v
  digitalWrite (A3, HIGH);           //trun it on
  pinMode (A2, OUTPUT);              //rtc ground
  digitalWrite (A2, LOW);            //turn on    
  DateTime now = RTC.now();
  digitalWrite (A3, LOW);            //trun off power to rtc for battery saving
  return now;  
}

void displayCountDown(int minutesLeft) {
  serLCD lcd(3);  
  lcd.clear();
  lcd.setBrightness(10);
  lcd.print(minutesLeft);
  if (minutesLeft>1) {
    lcd.print(" more minutes");
  } else if (minutesLeft==1) {
    lcd.print(" more minute!");
  } else {
    lcd.clear();
    lcd.print("OPEN!");
  }
}
