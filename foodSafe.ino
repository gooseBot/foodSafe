#include <Servo.h> 
Servo myservo;  // create servo object to control a servo 
//unsigned long _lockDuration = 3600000;   // 1hr
//unsigned long _lockDuration = 600000;  // 1 min
unsigned long _lockDuration = 1000;  // 1 min
byte _open=0;
byte _close=100;
byte _doorServo=7;
byte _lockButton=2;

void setup() 
{ 
  //setup power to clock module
  pinMode (A3, OUTPUT);              // I want to simply plug the clock board into pins A2 through A5
  digitalWrite (A3, HIGH);               // I am using this line as a supply voltage to the clock board
  pinMode (A2, OUTPUT);             // This pin can't be left floating if we are going to use it as ground for the RTC
  digitalWrite (A2, LOW);               // Set this pin low so that it acts as Ground for the clock

  pinMode(_lockButton, INPUT_PULLUP);  //pull signal for button high
  //unlock the door on a reset
  //  will assume I can run it off a battery
  //  will need to add ability to monitor voltage and unlock door
  //  before battery dies.  Good article about this on web
  controlDoor(true);
} 
 
void loop() 
{ 
  //if button pushed then lock the door and wait for the lock duration to end
  int sensorVal = digitalRead(2);
  if (sensorVal == LOW) {
    controlDoor(true);   //lock door
    myDelay(_lockDuration);  // wait duration
    controlDoor(false);   //oopen door
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

void controlDoor(boolean openLock) {
  myservo.attach(_doorServo,544,2400);  
  if (openLock) {
    myservo.write(_open); 
  } else {
    myservo.write(_close);    
  }
  myDelay(300);
  myservo.detach();     
}
