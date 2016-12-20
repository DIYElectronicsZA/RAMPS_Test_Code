#include "Arduino.h"
#include <TimerOne.h>
#include "thermistortables.h"

#define VERSION_STRING    "V0.1"

#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
#define X_MIN_PIN           3
#define X_MAX_PIN           2

#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56
#define Y_MIN_PIN          14
#define Y_MAX_PIN          15

#define Z_STEP_PIN         46
#define Z_DIR_PIN          48
#define Z_ENABLE_PIN       62
#define Z_MIN_PIN          18
#define Z_MAX_PIN          19

#define E_STEP_PIN         26
#define E_DIR_PIN          28
#define E_ENABLE_PIN       24

#define Q_STEP_PIN         36
#define Q_DIR_PIN          34
#define Q_ENABLE_PIN       30

#define SDPOWER            -1

#define EXTRUDERS 3

#define TEMP_SENSOR_AD595_OFFSET 0.0
#define TEMP_SENSOR_AD595_GAIN   1.0

#define THERMISTORHEATER_0 1
#define THERMISTORHEATER_1 1
#define THERMISTORHEATER_2 1

#define HEATER_0_USES_THERMISTOR 1
#define HEATER_1_USES_THERMISTOR 1
#define HEATER_2_USES_THERMISTOR 1

  static void *heater_ttbl_map[EXTRUDERS] = { (void *)heater_0_temptable
#if EXTRUDERS > 1
                                            , (void *)heater_1_temptable
#endif
#if EXTRUDERS > 2
                                            , (void *)heater_2_temptable
#endif
#if EXTRUDERS > 3
  #error Unsupported number of extruders
#endif
  };

    static int heater_ttbllen_map[EXTRUDERS] = { heater_0_temptable_len
#if EXTRUDERS > 1
                                             , heater_1_temptable_len
#endif
#if EXTRUDERS > 2
                                             , heater_2_temptable_len
#endif
#if EXTRUDERS > 3
  #error Unsupported number of extruders
#endif
  };

  #define PGM_RD_W(x)   (short)pgm_read_word(&x)

#define SDSS               53
#define LED_PIN            13

#define FAN_PIN            9




#define PS_ON_PIN          12
#define KILL_PIN           -1

#define HEATER_0_PIN       10

#define HEATER_1_PIN       8
#define TEMP_0_PIN         15   // ANALOG NUMBERING
#define TEMP_1_PIN         14   // ANALOG NUMBERING
#define TEMP_2_PIN         13   // ANALOG NUMBERING

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200

// Since microstepping is set externally, make sure this matches the selected mode
// 1=full step, 2=half step etc.
#define MICROSTEPS 16

// steps / rev
#define STEPS_REV MOTOR_STEPS * MICROSTEPS


#define MAX_SPEED  8

//interrupt period in uS
#define INT_PERIOD  100

bool dirFlag;
bool running;
unsigned int speed_ticks;
unsigned int tick_count;
unsigned long tmr1_clock;

#define DIR_DELAY 10000

// function prototypes
void timerIsr() ;

void setup() {

  // Temp Inputs
  pinMode(TEMP_0_PIN  , INPUT);
  pinMode(TEMP_1_PIN  , INPUT);
  pinMode(TEMP_2_PIN  , INPUT);

  //Endstops
  pinMode(X_MIN_PIN   , INPUT_PULLUP);
  pinMode(X_MAX_PIN   , INPUT_PULLUP);
  pinMode(Y_MIN_PIN   , INPUT_PULLUP);
  pinMode(Y_MAX_PIN   , INPUT_PULLUP);
  pinMode(Z_MIN_PIN   , INPUT_PULLUP);
  pinMode(Z_MAX_PIN   , INPUT_PULLUP);

  // Output pins
  pinMode(FAN_PIN       , OUTPUT);
  pinMode(HEATER_0_PIN  , OUTPUT);
  pinMode(HEATER_1_PIN  , OUTPUT);
  pinMode(LED_PIN       , OUTPUT);

  pinMode(X_STEP_PIN    , OUTPUT);
  pinMode(X_DIR_PIN     , OUTPUT);
  pinMode(X_ENABLE_PIN  , OUTPUT);

  pinMode(Y_STEP_PIN    , OUTPUT);
  pinMode(Y_DIR_PIN     , OUTPUT);
  pinMode(Y_ENABLE_PIN  , OUTPUT);

  pinMode(Z_STEP_PIN    , OUTPUT);
  pinMode(Z_DIR_PIN     , OUTPUT);
  pinMode(Z_ENABLE_PIN  , OUTPUT);

  pinMode(E_STEP_PIN    , OUTPUT);
  pinMode(E_DIR_PIN     , OUTPUT);
  pinMode(E_ENABLE_PIN  , OUTPUT);

  pinMode(Q_STEP_PIN    , OUTPUT);
  pinMode(Q_DIR_PIN     , OUTPUT);
  pinMode(Q_ENABLE_PIN  , OUTPUT);

  // make sure all motors off
  digitalWrite(X_ENABLE_PIN    , HIGH);
  digitalWrite(Y_ENABLE_PIN    , HIGH);
  digitalWrite(Z_ENABLE_PIN    , HIGH);
  digitalWrite(E_ENABLE_PIN    , HIGH);
  digitalWrite(Q_ENABLE_PIN    , HIGH);

  // Timer stuffs http://www.lucadentella.it/en/2013/05/30/allegro-a4988-e-arduino-3/
  Timer1.initialize(INT_PERIOD); // setup for 10uS interrupts
  Timer1.attachInterrupt(timerIsr); // attach isr function

   // initial values
  dirFlag = true;
  running = true;
  tick_count = 0;
  speed_ticks = MAX_SPEED;

  Serial.begin(115200);
  Serial.println("Hello! RAMPS test code here :D ");
  Serial.println(VERSION_STRING);
  // Serial.println("Enter to continue... ");
  // Serial.println(" ");
  // while(Serial.available() == 0) { }
  // Serial.read();
}



float analog2temp(int raw, uint8_t e) {

  #ifdef HEATER_0_USES_MAX6675
    if (e == 0)
    {
      return 0.25 * raw;
    }
  #endif

  if(heater_ttbl_map[e] != 0)
  {
    float celsius = 0;
    byte i;
    short (*tt)[][2] = (short (*)[][2])(heater_ttbl_map[e]);

    raw = (1023 * OVERSAMPLENR) - raw;
    for (i=1; i<heater_ttbllen_map[e]; i++)
    {
      if ((PGM_RD_W((*tt)[i][0]) > raw) && ((float)(PGM_RD_W((*tt)[i][0]) - PGM_RD_W((*tt)[i-1][0])) >0))
      {
        celsius = PGM_RD_W((*tt)[i-1][1]) +
          (raw - PGM_RD_W((*tt)[i-1][0])) *
          (float)(PGM_RD_W((*tt)[i][1]) - PGM_RD_W((*tt)[i-1][1])) /
          (float)(PGM_RD_W((*tt)[i][0]) - PGM_RD_W((*tt)[i-1][0]));
        break;
      }
    }

    // Overflow: Set to last value in the table
    if (i == heater_ttbllen_map[e]) celsius = PGM_RD_W((*tt)[i-1][1]);

    return celsius;
  }
  return ((raw * ((5.0 * 100.0) / 1024.0) / OVERSAMPLENR) * TEMP_SENSOR_AD595_GAIN) + TEMP_SENSOR_AD595_OFFSET;
}

unsigned long prevMillis;

  char serData = 0;


// Main program loop entry point
void loop () {

  // Testing Motors:
  Serial.print("Testing X Motor: ");
  digitalWrite(X_ENABLE_PIN    , LOW); // turn X on
  Serial.println("Working? (y/n)");
  while(serData != 'y' && serData != 'n' && serData != 'Y' && serData != 'N')
  {if (Serial.available()) {serData = (char)Serial.read();}}
  serData = 0;
  digitalWrite(X_ENABLE_PIN    , HIGH); // turn X off

  Serial.print("Testing Y Motor: ");
  digitalWrite(Y_ENABLE_PIN    , LOW); // turn Y on
  Serial.println("Working? (y/n)");
  while(serData != 'y' && serData != 'n' && serData != 'Y' && serData != 'N')
  {if (Serial.available()) {serData = (char)Serial.read();}}
  serData = 0;
  digitalWrite(Y_ENABLE_PIN    , HIGH); // turn Y off

  Serial.print("Testing Z Motor: ");
  digitalWrite(Z_ENABLE_PIN    , LOW); // turn Z on
  Serial.println("Working? (y/n)");
  while(serData != 'y' && serData != 'n' && serData != 'Y' && serData != 'N')
  {if (Serial.available()) {serData = (char)Serial.read();}}
  serData = 0;
  digitalWrite(Z_ENABLE_PIN    , HIGH); // turn Z off

  Serial.print("Testing E Motor: ");
  digitalWrite(E_ENABLE_PIN    , LOW); // turn E on
  Serial.println("Working? (y/n)");
  while(serData != 'y' && serData != 'n' && serData != 'Y' && serData != 'N')
  {if (Serial.available())serData = (char)Serial.read();}
  serData = 0;
  digitalWrite(E_ENABLE_PIN    , HIGH); // turn E off

  //Test Heaters
  Serial.print("Testing Hotend: ");
  digitalWrite(HEATER_0_PIN, HIGH); // turn Hotend on
  Serial.println("Working? (y/n)");
  while(serData != 'y' && serData != 'n' && serData != 'Y' && serData != 'N'){
    if (Serial.available())serData = (char)Serial.read();
    if (millis() -prevMillis >500){
      prevMillis=millis();
      int t = analogRead( TEMP_0_PIN);
      Serial.print("THot ");
      Serial.print(t);
      Serial.print("/");
      Serial.println(analog2temp(1024 - t,0),0);
    }
  }
  serData = 0;
  digitalWrite(HEATER_0_PIN, LOW); // turn Hotend off

  Serial.print("Testing Bed: ");
  digitalWrite(HEATER_1_PIN, HIGH); // turn Bed on
  Serial.println("Working? (y/n)");
  while(serData != 'y' && serData != 'n' && serData != 'Y' && serData != 'N'){
    if (Serial.available())serData = (char)Serial.read();
    if (millis() -prevMillis >500){
      prevMillis=millis();
      int t = analogRead( TEMP_1_PIN);
      Serial.print("TBed ");
      Serial.print(t);
      Serial.print("/");
      Serial.println(analog2temp(1024 - t,0),0);
    }
  }
  serData = 0;
  digitalWrite(HEATER_1_PIN, LOW); // turn Bed off

  Serial.print("Testing Fan: ");
  digitalWrite(FAN_PIN, HIGH); // turn Fan on
  Serial.println("Working? (y/n)");
  while(serData != 'y' && serData != 'n' && serData != 'Y' && serData != 'N'){
    if (Serial.available())serData = (char)Serial.read();
    if (millis() -prevMillis >500){
      prevMillis=millis();
      int t = analogRead( TEMP_2_PIN);
      Serial.print("T2 ");
      Serial.print(t);
      Serial.print("/");
      Serial.println(analog2temp(1024 - t,0),0);
    }
  }
  serData = 0;
  digitalWrite(FAN_PIN, LOW); // turn Fan off

  // Testing Endstops

  if(digitalRead(X_MIN_PIN)){
    Serial.println("Error X Endstop activated!");
  }else{
    Serial.print("Testing X Endstop: ");
    Serial.println("Please press X Endstop");
    while(!digitalRead(X_MIN_PIN)){ }
    Serial.println("Please relase X Endstop");
    while(digitalRead(X_MIN_PIN)){ }
    Serial.println("X Endstop Working");
  }

  if(digitalRead(Y_MIN_PIN)){
    Serial.println("Error Y Endstop activated!");
  }else{
    Serial.print("Testing Y Endstop: ");
    Serial.println("Please press Y Endstop");
    while(!digitalRead(Y_MIN_PIN)){ }
    Serial.println("Please relase Y Endstop");
    while(digitalRead(Y_MIN_PIN)){ }
    Serial.println("Y Endstop Working");
  }

  if(digitalRead(Z_MIN_PIN)){
    Serial.println("Error Z Endstop activated!");
  }else{
    Serial.print("Testing Z Endstop: ");
    Serial.println("Please press Z Endstop");
    while(!digitalRead(Z_MIN_PIN)){ }
    Serial.println("Please relase Z Endstop");
    while(digitalRead(Z_MIN_PIN)){ }
    Serial.println("Z Endstop Working");
  }

  
}

// ISR to do stepper moves
void timerIsr() {

  // inc the step counter
  tick_count++;
  tmr1_clock++;

  // if we have hit the limit and we should be running
  if((tick_count >= speed_ticks) && running) {

    // make a step
    digitalWrite(X_STEP_PIN    , HIGH);
    digitalWrite(Y_STEP_PIN    , HIGH);
    digitalWrite(Z_STEP_PIN    , HIGH);
    digitalWrite(E_STEP_PIN    , HIGH);
    digitalWrite(Q_STEP_PIN    , HIGH);

    digitalWrite(X_STEP_PIN    , LOW);
    digitalWrite(Y_STEP_PIN    , LOW);
    digitalWrite(Z_STEP_PIN    , LOW);
    digitalWrite(E_STEP_PIN    , LOW);
    digitalWrite(Q_STEP_PIN    , LOW);

    // reset tick counter
    tick_count = 0;
  }

  //flip direction
  if (tmr1_clock >= DIR_DELAY) {
    tmr1_clock = 0;
    if(dirFlag){
      dirFlag = false;
      digitalWrite(X_DIR_PIN    , HIGH);
      digitalWrite(Y_DIR_PIN    , HIGH);
      digitalWrite(Z_DIR_PIN    , HIGH);
      digitalWrite(E_DIR_PIN    , HIGH);
      digitalWrite(Q_DIR_PIN    , HIGH);
      //digitalWrite(LED_PIN      , HIGH);
    }else {
      dirFlag = true;
      digitalWrite(X_DIR_PIN    , LOW);
      digitalWrite(Y_DIR_PIN    , LOW);
      digitalWrite(Z_DIR_PIN    , LOW);
      digitalWrite(E_DIR_PIN    , LOW);
      digitalWrite(Q_DIR_PIN    , LOW);
      //digitalWrite(LED_PIN      , LOW);
    }
  }

}
