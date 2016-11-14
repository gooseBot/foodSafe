
//#include "pitches.h"
#include <Servo.h> 
#include <Wire.h>
#include "RTClib.h" 
#include "TimeLib.h"
#include <TimeAlarms.h>
#include <SoftwareSerial.h>
#include <serLCD.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
struct appConfig
{
  DateTime openTime;
};
typedef struct appConfig AppConfig;
AppConfig _myConfig;
byte _lockButton=2;
byte _doorCloseButton = 8;
uint8_t _myAlarms[5];
bool _doorLockStatus;
serLCD _lcd(3);
 
void setup() 
{ 
  //setTimeDate();  //uncomment this to set the date time to the compile date time
  Serial.begin(57600);

  _lcd.clear();
  _lcd.setBrightness(10);

  DateTime now = getTimeDate();
  setTime(now.hour(),now.minute(), now.second(), now.month(), now.day(), now.year()); //set time to RTC time
  printCurrentTime(0);

  _myAlarms[0] = Alarm.alarmRepeat(10, 0, 0, openDoor);  
  _myAlarms[1] = Alarm.alarmRepeat(12, 0, 0, openDoor);  
  _myAlarms[2] = Alarm.alarmRepeat(15, 0, 0, openDoor);  
  _myAlarms[3] = Alarm.alarmRepeat(17, 0, 0, openDoor);
  _myAlarms[4] = Alarm.alarmRepeat(19, 0, 0, openDoor);
  _myAlarms[5] = Alarm.alarmRepeat(21, 0, 0, openDoor);
  repeats();                       //update the display now 
  Alarm.timerRepeat(60, repeats);  //setup a repeat alarm to update lcd

  //EEPROM_readAnything(0, _myConfig);
  
  pinMode(_lockButton, INPUT_PULLUP);  //pull signal for button high
  pinMode(_doorCloseButton, INPUT_PULLUP);

  doorServo(false);   //on startup lock the door

  if (_doorLockStatus) {
    showLCDmessage("Door locked");
  } else {
    showLCDmessage("Door unlocked");
  }

  //playMelody();
} 
 
void loop() 
{ 
  //if button pushed then lock the door 
  if (digitalRead(_lockButton) == LOW) {
    if (digitalRead(_doorCloseButton) == LOW) {
      doorServo(false);
      Alarm.delay(300);
      showLCDmessage("Door locked!");
    }
    else {
      showLCDmessage("Cant lock door  open!");
    }
  }   
  Alarm.delay(100); 
}

void repeats() {
  time_t timeMidnight = previousMidnight(now());
  time_t minSpan = 86400; //num seconds in a day
  for (int i = 0; i < sizeof(_myAlarms); i++)
  {
    time_t alarmTime = Alarm.read(_myAlarms[i]) + timeMidnight;
    if (alarmTime > now()) {
      time_t alarmFromNowTime = alarmTime - now();
      if (alarmFromNowTime < minSpan)
        minSpan = alarmFromNowTime;
    }      
  }
  displayCountDown(minSpan/60);
}

// function to be called when an alarm triggers:
void openDoor() {
  doorServo(true);
  showLCDmessage("Door unlocked!");
  playMelody();
  Alarm.timerOnce(0, 20, 0, relockDoor);
}

void relockDoor() {
  if (digitalRead(_doorCloseButton) == LOW) {
    doorServo(false);  //relock door if door is still closed
    showLCDmessage("Door relocked!");
    Alarm.delay(60);
  } else {
    showLCDmessage("Can't relock,   door still open.");
  }
}

void doorServo(boolean openLock) {
  static Servo myservo;  // create servo object to control a servo
  byte _doorServo=7;
  byte _open=0;
  byte _close=100;
  _doorLockStatus = false;
  myservo.attach(_doorServo,544,2400);  
  if (openLock) {
    myservo.write(_open);     
  } else {
    //lock if door closed
    if (digitalRead(_doorCloseButton) == LOW) {
      myservo.write(_close);
      _doorLockStatus = true;
    } else {
      //open lock if door is open
      myservo.write(_open);
      _doorLockStatus = false;
    }
  }
  Alarm.delay(300);
  myservo.detach();     
}

void printCurrentTime(int min2unlock) {
  DateTime now = getTimeDate();
  //Serial.begin(57600);
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
  //Serial.end();
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

void setTimeDate() {
  RTC_DS1307 RTC;
  Wire.begin();
  RTC.begin();
  pinMode(A3, OUTPUT);              //rtc power 5v
  digitalWrite(A3, HIGH);           //trun it on
  pinMode(A2, OUTPUT);              //rtc ground
  digitalWrite(A2, LOW);            //turn on    
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  digitalWrite(A3, LOW);            //trun off power to rtc for battery saving
}

void displayCountDown(int minutesLeft) {
  // dont display countdown if door is open or unlocked
  if (digitalRead(_doorCloseButton) == HIGH || !_doorLockStatus )
    return;

  _lcd.clear(); 
  if (minutesLeft >=1440) {
    _lcd.print("Done today :(");
  } else if (minutesLeft > 0) {
    if ((minutesLeft / 60) > 0) {
      _lcd.print(minutesLeft / 60);
      _lcd.print(" hr ");
    }
    _lcd.print(minutesLeft % 60);
    _lcd.print(" more min");
  } else if (minutesLeft == 0) {
    _lcd.print("1 more minute!");
  } 
}

void showLCDmessage(String message) {
  _lcd.clear();
  _lcd.print(message);
}
