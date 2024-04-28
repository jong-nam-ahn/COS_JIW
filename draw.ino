//----------LIBRARIES-------------
// invlude relevant libraries
// #1 MAX7219 8 by 8 LED MATRIX
// #include "LedControl.h"
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
// #2 Servo Motor
//#include <Servo.h>
#include <Adafruit_TiCoServo.h>

//------- VARIABLES-------------
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW // hardware type for LED
#define MAX_DEVICES 1 // # of matrix
// initializing LED matrix
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, 13, 11, 12, MAX_DEVICES);

#define YEARS 14
// Number of Surviving Comfort Women
// in a given year
int survived[YEARS][2] = {
  {2007, 108},
  {2008, 93},
  {2009, 87},
  {2010, 78},
  {2011, 63},
  {2012, 59},
  {2013, 56},
  {2014, 55},
  {2015, 46},
  {2016, 40},
  {2017, 32},
  {2018, 25},
  {2019, 20},
  {2020, 16}
};

int count = 0; // iteration counter

String message;

// measure time for delays 
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 20000;  //

//LedControl lc=LedControl(A3,A1,A2,1);

// setup servo
int servoPin = 9;
int PEN_DOWN = 20; // angle of servo when pen is down
int PEN_UP = 120;   // angle of servo when pen is up

// control size of image
float ratio = 0.7;

// degree between the robot's direction 
// and the x axis
int deg = 0;

// robot position
struct pos {
  float x;
  float y;
};

// initialize to origin
struct pos p = { 0, 0 };

bool dir = true;

// right or left turn
bool r = true;
bool l = false;

// add randomness to each drawing iteration
bool randomL = false; // length
bool randomA = false; // angle

// create servo object
//Servo penServo;
Adafruit_TiCoServo penServo; 

float wheel_dia=59; 
float wheel_dis=105; //distance between the two wheels (mm)
int steps_rev=128; //32 * 4
unsigned long delay_time=12; //time between steps (ms)
unsigned long d = 250;

// Stepper sequence
// line corresponding to stepper is
// orange pink blue yellow (2 4 5 3)
int L_stepper_pins[] = {5, 3, 2, 4};
int R_stepper_pins[] = {A2, A4, A5, A3};

// forward
int fwd_mask[][4] =  {{1, 0, 1, 0},
                      {0, 1, 1, 0},
                      {0, 1, 0, 1},
                      {1, 0, 0, 1}};

// reverse 
int rev_mask[][4] =  {{1,  0, 0, 1},
                      {0, 1, 0, 1},
                      {0, 1, 1, 0},
                      {1, 0, 1, 0}};


//------- MAIN FUNCTIONS-------------
void setup() {
  long randNum; // year

  Serial.begin(9600);
  Serial.println("setup");

  // motors
  for(int pin=0; pin<4; pin++){
    pinMode(L_stepper_pins[pin], OUTPUT);
    digitalWrite(L_stepper_pins[pin], LOW);
    pinMode(R_stepper_pins[pin], OUTPUT);
    digitalWrite(R_stepper_pins[pin], LOW);
  }
  penServo.attach(servoPin);
  
  
  // led matrix
  myDisplay.begin();
	// Set the intensity (brightness) of the display (0-15)
	myDisplay.setIntensity(5);
	// Clear the display
	myDisplay.displayClear();

  // should be YEARS
  randNum = random(0, YEARS);
  message = String();

  message.concat(survived[randNum][0]);
  message.concat(": ");
  message.concat(survived[randNum][1]);
  message.concat(" surviving"); 

  // flip the display message (due to wiring)
  myDisplay.setZoneEffect(0, true, PA_FLIP_UD);
  myDisplay.setZoneEffect(0, true, PA_FLIP_LR);
  myDisplay.displayScroll(message.c_str(), PA_CENTER, PA_SCROLL_LEFT, 150);
}



void loop(){ 
  // display message
  /*
  currentMillis = millis();
  startMillis = currentMillis;
  while(currentMillis - startMillis < period){
    myDisplay.displaySuspend(myDisplay.displayAnimate());
    currentMillis = millis();
  }
  */

  // start drawing
  penup();

  // iterate by the number of surviving comfort women
  // at a given year
  for(int x=0; x< survived[randNum][1]; x++){
    pendown();
    drawFace();
    count = count + 1;
    message = String(count);
    message.concat(" out of ");
    message.concat(survived[randNum][1]);
    currentMillis = millis();
    startMillis = currentMillis;
    myDisplay.displayReset();
    while(currentMillis - startMillis < period / 1.5){
      myDisplay.displaySuspend(myDisplay.displayAnimate());
      currentMillis = millis();
    }
  }

  penup();
  done();      // releases stepper motor
  while(1);    // wait for reset
}




//------- HELPER FUNCTIONS-------------
int step(float distance){
  int steps = distance * steps_rev / (wheel_dia * 3.1412); 
  return steps;  
}


void forward(float distance){
  int steps = step(distance);
  unsigned long track;
  unsigned long track_until;
  // Serial.println(steps);
  for(int step=0; step<steps; step++){
    for(int mask=0; mask<4; mask++){
      for(int pin=0; pin<4; pin++){
        digitalWrite(L_stepper_pins[pin], rev_mask[mask][pin]);
        digitalWrite(R_stepper_pins[pin], fwd_mask[mask][pin]);
      }
      track = millis();
      track_until = track;
      while(track - track_until < delay_time){
        track = millis();
      }
    } 
  }
}


void backward(float distance){
  int steps = step(distance);
  unsigned long track;
  unsigned long track_until;
  for(int step=0; step<steps; step++){
    for(int mask=0; mask<4; mask++){
      for(int pin=0; pin<4; pin++){
        digitalWrite(L_stepper_pins[pin], fwd_mask[mask][pin]);
        digitalWrite(R_stepper_pins[pin], rev_mask[mask][pin]);
      }
      track = millis();
      track_until = track;
      while(track - track_until < delay_time){
        track = millis();
      }
    } 
  }
}


void right(float degrees){
  float rotation = degrees / 360.0;
  float distance = wheel_dis * 3.1416 * rotation;
  unsigned long track;
  unsigned long track_until;

  dir = true;
  int steps = step(distance);
  for(int step=0; step<steps; step++){
    for(int mask=0; mask<4; mask++){
      for(int pin=0; pin<4; pin++){
        digitalWrite(R_stepper_pins[pin], rev_mask[mask][pin]);
        digitalWrite(L_stepper_pins[pin], rev_mask[mask][pin]);
      }
      track = millis();
      track_until = track;
      while(track - track_until < delay_time){
        track = millis();
      }
    } 
  }   
}


void left(float degrees){
  float rotation = degrees / 360.0;
  float distance = wheel_dis * 3.1412 * rotation;
  unsigned long track;
  unsigned long track_until;
  
  //float distance = 200 * 3.1412 * rotation;
  int steps = step(distance);
  dir = false;
  for(int step=0; step<steps; step++){
    for(int mask=0; mask<4; mask++){
      for(int pin=0; pin<4; pin++){
        digitalWrite(R_stepper_pins[pin], fwd_mask[mask][pin]);
        digitalWrite(L_stepper_pins[pin], fwd_mask[mask][pin]);
      }
      track = millis();
      track_until = track;
      while(track - track_until < delay_time){
        track = millis();
      }
    } 
  }   
}


void done(){ // unlock motor to save battery
  unsigned long track;
  for(int mask=0; mask<4; mask++){
    for(int pin=0; pin<4; pin++){
      digitalWrite(R_stepper_pins[pin], LOW);
      digitalWrite(L_stepper_pins[pin], LOW);
    }
    track = millis();
    while(millis() - track < delay_time){
   }
  }
}


void penup(){
  unsigned long track;
  penServo.write(PEN_UP);
  track = millis();
  while(millis() - track < d){
  }
}


void pendown(){
  unsigned long track;
  penServo.write(PEN_DOWN);
  track = millis();
  while(millis() - track < d){
  }
}

void movement(int length, int degree, bool direction, bool direction2 = true) {
  
  unsigned long time;
  unsigned long wait;
  // add randomness
  if (count != 0) {
    // length
    if (randomL) {
      if (length > 30)
        length = (int) (length + random(-5, 5) * ratio);
    }
    

    // angle
    if (randomA) {
      if (degree < 30) {
        degree += random(0,5);
      }
      else if (degree < 150) {
        degree += ((2 * random(-1, 1) + 1) * random(0, 4));
      }
      else {
        degree -= random(0, 5);
      }
    }
  }
   
  int adj_length = (int) round(length * ratio);

  if (degree > 0) {
    deg = (deg + degree * (-2 * direction + 1) + 720 ) % 360; 
  }

  /*
  Serial.println("deg");
  Serial.println(deg);
  */

  if (degree == 0) {
    // Serial.println("degree is 0");
    // Serial.println(deg);
  }

  

  if (direction) {
    right(degree);
  }
  else {
    left(degree);
  }

  /*
  time = millis();
  wait = time;
  while(time - wait > 250) {
    time = millis();
  }
  */

  if (direction2) {
    forward(adj_length);
  }
  else {
    backward(adj_length);
  }


  if (adj_length > 0) {
    p = update(p.x, p.y, deg, adj_length, dir, direction2);
    /*
    Serial.println("position update");
    Serial.println(p.x);
    Serial.println(p.y);
    */
  }

}


pos update(float x_pos, float y_pos, int deg, int length, bool direction, bool direction2) {
  pos updated;
  float x = x_pos;
  float y = y_pos;

  // Serial.print("before update")
  //Serial.println(x);
  //Serial.println(y);

  if (direction2) {
    x += length * cos(deg * 3.1412 / 180);
    y += length * sin(deg *  3.1412 / 180);
  }
  else {
    x += length * cos(((deg + 180) % 360) * 3.1412 / 180);
    y += length * sin(((deg + 180) % 360) *  3.1412 / 180);
  }

  updated.x = x;
  updated.y = y;

  // Serial.print("after update")
  //Serial.println(x);
  //Serial.println(y);

  return updated;
}

// robot travels to initial point
// from its current position
void toOrigin() {
  int dec_x; 
  int dec_y; 
  float x_pos;
  float y_pos;
  float x_dis;
  float y_dis;

  Serial.println("deg");
  Serial.println(deg);
  unsigned long track;
  unsigned long track_until;
  Serial.println("position");
  Serial.println("x loc");
  Serial.println(x_pos);
  Serial.println("y loc");
  Serial.print(y_pos);

  x_pos = p.x;
  y_pos = p.y;
  // first digit after decimal point
  dec_x = ((int) abs(x_pos) * 10) % 10;
  dec_y = ((int) abs(y_pos) * 10) % 10;

  x_dis = round(x_pos + ((random(0, 10) < dec_x) - 0.5) * (x_pos > 0));
  y_dis = round(y_pos + ((random(0, 10) < dec_y) - 0.5) * (y_pos > 0));

  if (deg < 180) {
    if (x_dis > 0) {
    movement(x_dis / ratio, deg, r, false);
    } else {
    movement(abs(x_dis) / ratio, deg, r, true);
    }
  } else {
    deg = (360 - deg);
    if (x_dis > 0) {
    movement(x_dis / ratio, deg, l, false);
    } else {
    movement(abs(x_dis) / ratio, deg, l, true);
    }
  }

  Serial.println("intermediate position");
  Serial.println("x loc");
  Serial.println(p.x);
  Serial.println("y loc");
  Serial.print(p.y);

  track = millis();
  track_until = track;
  while(track - track_until < 2 * d){
    track = millis();
  }

  if (y_dis > 0) {
    movement(y_dis / ratio, 90, r, true);
    left(90);
  } else {
    movement(abs(y_dis) / ratio, 90, l, true);
    right(90);
  }

  ///*
  Serial.println("final position");
  Serial.println("x loc");
  Serial.println(p.x);
  Serial.println("y loc");
  Serial.print(p.y);
  //*/

  p.x = 0;
  p.y = 0;
  deg = 0;
  
}



//-------FACE DRAWING FUNCTIONS-------------
void left_eye() {
  movement(40, 10, l);
  movement(40, 60, r);
  movement(15, 66, r);
  movement(35, 71, r);
  movement(17, 35, l);
  movement(15, 112, l);
  movement(35, 50, l);
}

void right_eye() {
  movement(25, 8, r);
  movement(27, 17, l);
  movement(30, 157, l);
  movement(30, 38, l);
  movement(54, 80, l);
  movement(10, 63, r);
  movement(24, 45, r);
  movement(10, 54, r);
}

void lipNhair() {
  movement(15, 60, r);
  movement(20, 16, r);
  movement(17, 30, r);
  movement(54, 147, r);
  movement(27, 159, l);
  movement(27, 130, r);
  movement(40, 123, l);
  movement(46, 55, l);
  movement(37, 25, l);
  movement(74, 25, l);
  movement(68, 30, l);
  movement(88, 44, l);
  movement(80, 54, l);
  movement(50, 48, l);
  movement(70, 24, l);
  movement(45, 104, r);
  movement(57, 84, r);
  movement(95, 16, r);
  movement(107, 40, r);
  movement(103, 59, r);
  movement(90, 40, r);
}

void drawFace() {
  unsigned long track;
  unsigned long track_until;

  left_eye();
  
  penup();

  //movement(46, 5, r);
  movement(46, 10, r);
  pendown();

  right_eye();
  penup();

  // change from 40, 48
  movement(40, 50, r, false);

  pendown();

  lipNhair();
  penup();

  toOrigin();
  pendown();

}

//---------TEST FUNCTIONS---------------

// test basic shape (square)
void test1() {
  for(int x=0; x<12; x++){
    forward(50);
    left(90);
  }
}

// test basic letter (M)
void test2() {
  movement(80, 150, true);
  delay(250);
  movement(40, 120, false);
  delay(250);
  movement(40, 150, true);
  delay(250);
  movement(80, 0, true);
}

// test position update (result sould be (10, -10 sqrt 3))
void test3() {
  movement(20, 60, l);
  movement(20, 120, r);
  movement(20, 120, l, false);
  Serial.println("Is this correct");
  Serial.println(p.x);
  Serial.println(p.y);
}