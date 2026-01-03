#include <EEPROM.h>
#define FAN_PIN 11
#define WATER_PUMP 3
#define SOLENOID 4
#define COMPRESSOR 5
#define motorPinA 6
#define motorPinB 7
#define BOT_LIMIT 8 // goes low when tray is ready to be unloaded
#define TOP_LIMIT 9 // goes low when tray is ready to be loaded
#define POWER_LED 10
#define PUSH_BUTTON 2
#define ICE_FULL_SENSOR A1
#define WATER_FLOW_SENSOR A0

// === Adjustables ===
#define time_to_ice 600000    
#define time_to_release 6000
#define time_to_fill_tray 10000 
const unsigned long MOTOR_TIMEOUT = 10000;
const bool int_enable = true; 
const unsigned long SESSION_MS = 3600000UL;

// === LOGIC ===
bool motorDirectionCW = true; //CW dumps ice and CCW loads water
bool ICE_FULL = false;
bool NO_WATER = false;
bool system_on = true;
bool led_state = false;
bool cooldownRequired = false;
bool compressorRunning = false;

const int EE_COOLDOWN_FLAG_ADDR = 0;  
const uint8_t EE_MAGIC = 0xA5;
const int EE_MAGIC_ADDR = 1;          

unsigned long compressorOnAt = 0;
unsigned long compressorOffAt = 0;
const unsigned long COOLDOWN_MS = 10UL * 60UL * 1000UL;

unsigned long last_blink_time = 0;
unsigned long wentToSleepAt_time = 0;
unsigned long startupTime = 0;

volatile bool interruptFlag = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 1000;

void setup() {
  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(PUSH_BUTTON), toggleSystemState, FALLING);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(WATER_PUMP, OUTPUT);
  pinMode(SOLENOID, OUTPUT);
  pinMode(COMPRESSOR, OUTPUT);
  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);
  pinMode(BOT_LIMIT, INPUT_PULLUP); // tray unload position
  pinMode(TOP_LIMIT, INPUT_PULLUP);   // tray load position
  pinMode(POWER_LED, OUTPUT);
  pinMode(PUSH_BUTTON, INPUT_PULLUP);
  digitalWrite(POWER_LED, HIGH);

  // Turn off all devices initially
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(WATER_PUMP, LOW);
  digitalWrite(SOLENOID, LOW);
  digitalWrite(COMPRESSOR, LOW);
  digitalWrite(motorPinA, LOW); 
  digitalWrite(motorPinB, LOW); 
  startupTime = millis();

  //init eeprom for compressor manditory cooldown
  if (EEPROM.read(EE_MAGIC_ADDR) != EE_MAGIC) {
      EEPROM.update(EE_MAGIC_ADDR, EE_MAGIC);
      EEPROM.update(EE_COOLDOWN_FLAG_ADDR, 0);  // assume cold
    }

    uint8_t flag = EEPROM.read(EE_COOLDOWN_FLAG_ADDR);

  // EEPROM healthy?
  if (flag != 0 && flag != 1) {
    flag = 0; // assume cold 
    EEPROM.update(EE_COOLDOWN_FLAG_ADDR, flag);
  }

  cooldownRequired = (flag == 1);
  if (cooldownRequired) {
    compressorOffAt = millis();
  }

  }

void toggleSystemState() {
  unsigned long currentTime = millis();
  if (int_enable && digitalRead(PUSH_BUTTON) == LOW && ((currentTime - lastInterruptTime) > debounceDelay)) {
    interruptFlag = true;
    lastInterruptTime = currentTime;

  }
}

// === MOTOR LOGIC ===
//returns true if motor reaches unload position in time
bool moveToload() 
{
  motorDirectionCW = false;
  if(digitalRead(TOP_LIMIT) == LOW) 
  {
    return true; //already at top
  }
  motorCCW();

  unsigned long startTime = millis();
  while (millis() - startTime < MOTOR_TIMEOUT) 
  {
    if(digitalRead(TOP_LIMIT) == LOW) 
    {
      stopMotor();        
      Serial.println("@top");
      return true; //already at top
    }
    
    //check if motor is moving towards the wrong limit switch but give it time to move initially
    if(millis() - startTime > 300 && digitalRead(BOT_LIMIT) == LOW)
    {
       while(digitalRead(BOT_LIMIT) == LOW)
        {
          reverseMotor();
          delay(300);
        }
      
      //restart the clock to give time for the motor to move
      startTime = millis();
    }
  }
    
  stopMotor();
  if(digitalRead(TOP_LIMIT) == HIGH) return false; //motor did not reach proper positioning in time
}


//returns true if motor reaches load position in time
bool moveToUnload() 
{
  motorDirectionCW = true;
  if(digitalRead(BOT_LIMIT) == LOW) 
  {
    return true; //already at bottom
  }
  motorCW();
  
  
  unsigned long startTime = millis();
  while (millis() - startTime < MOTOR_TIMEOUT) 
  {
    if(digitalRead(BOT_LIMIT) == LOW) 
    {
      stopMotor();
      Serial.println("@bot");
      return true; //already at bottom
    }

    //check if motor is moving towards the wrong limit switch but give it time to move initially
    if(millis() - startTime > 300 && digitalRead(TOP_LIMIT) == LOW)
    {
       while(digitalRead(TOP_LIMIT) == LOW)
        {
          reverseMotor();
          delay(300);
        }
      
      //restart the clock to give time for the motor to move
      startTime = millis();
    }
  }
  stopMotor();
  if(digitalRead(BOT_LIMIT) == HIGH) return false; //motor did not reach proper positioning in time
}

void stopMotor() {
  digitalWrite(motorPinA, LOW);
  digitalWrite(motorPinB, LOW);
  delay(50);
}

void motorCW()
{
  motorDirectionCW = true;
  digitalWrite(motorPinA, HIGH);
  delay(50);
  digitalWrite(motorPinB, LOW);
  delay(50); 
}

void motorCCW()
{
  motorDirectionCW = false;
  digitalWrite(motorPinA, LOW);
  delay(50);
  digitalWrite(motorPinB, HIGH);
  delay(50);
}

void reverseMotor()
{
  if(digitalRead(TOP_LIMIT) == LOW && digitalRead(BOT_LIMIT) == LOW)
  {
    Serial.println("ERROR: Critical - Both switches active");
    stopMotor();
    return;
  }
  stopMotor();
  if(motorDirectionCW)
  {
    motorCCW();
  }
  else
  {
    motorCW();
  }
  motorDirectionCW=!motorDirectionCW;
}

void unloadTray()
{
  detachInterrupt(digitalPinToInterrupt(PUSH_BUTTON));
  while(!moveToUnload()) {delay(10);}
   Serial.println("NOW UNLOADED");
  attachInterrupt(digitalPinToInterrupt(PUSH_BUTTON), toggleSystemState, FALLING);
}

void loadTray()
{
  detachInterrupt(digitalPinToInterrupt(PUSH_BUTTON));
  while(!moveToload()) 
  {
     delay(10);
    }
  Serial.println("NOW LOADED");
  attachInterrupt(digitalPinToInterrupt(PUSH_BUTTON), toggleSystemState, FALLING);
}

// === System Controls ===
void pump_water() {
  digitalWrite(WATER_PUMP, HIGH);
  delay(1000);

  int water_flow = analogRead(WATER_FLOW_SENSOR);
  float voltage = water_flow*5.0/1023;
  Serial.print("water flow sensor: ");
  Serial.println(voltage);
  NO_WATER = (voltage > 4.9); //5v normally and drops to 4.68-4.83 when water present
  delay(time_to_fill_tray-1000);
  digitalWrite(WATER_PUMP, LOW);
}

bool check_ice_full() {
  ICE_FULL = false;
  int infra = analogRead(ICE_FULL_SENSOR);
  float volt = infra*(5.0)/(1023); //dark room: 3.83 to 4.85 bright room:  0.9-3.025 
  Serial.print("ice sensor: ");
  Serial.println(volt);
  ICE_FULL = (volt>=4.5); //i chose values for dark room. Covering the machine's clear top will improve accuracy. Rewireing the led to create two samples (one with led and one without) will improve detection veracity during multiple light level scenarios
  return ICE_FULL;
}

void compressorOn() {
  if (cooldownRequired && (millis() - compressorOffAt < COOLDOWN_MS)) return;

  EEPROM.update(EE_COOLDOWN_FLAG_ADDR, 1);

  cooldownRequired = false;

  detachInterrupt(digitalPinToInterrupt(PUSH_BUTTON));
  delay(100);
  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(COMPRESSOR, HIGH);
  delay(100);
  attachInterrupt(digitalPinToInterrupt(PUSH_BUTTON), toggleSystemState, FALLING);

  if (!compressorRunning) {
    compressorRunning = true;
    compressorOnAt = millis();
  }
}


void compressorOff() {
  detachInterrupt(digitalPinToInterrupt(PUSH_BUTTON));
  delay(10);
  digitalWrite(COMPRESSOR, LOW);
  digitalWrite(FAN_PIN, LOW);
  delay(100);
  attachInterrupt(digitalPinToInterrupt(PUSH_BUTTON), toggleSystemState, FALLING);

  compressorRunning = false;
  compressorOffAt = millis();
  cooldownRequired = true;

}



void state_1() {
  Serial.println("State 1");
  unloadTray();

  // Wait until ice is NOT full
  if (check_ice_full()) {
    Serial.println("Ice bin full.");
    sleep();
  }
}

void state_2() {
  Serial.println("State 2");
  loadTray();
  delay(1000);
  Serial.println("Water is being loaded");
  pump_water();
  if (NO_WATER) {
    Serial.println("No water detected.");
    sleep();
    }
  }


void state_3() {
  Serial.println("State 3");
  
  compressorOn();
  if (!compressorRunning) {
    Serial.println("Cooldown active, waiting...");
    delay(1000);
    return;
  }
  
  //cooling period is time_to_ice
  unsigned long startTime = millis();
  while (millis() - startTime < time_to_ice) {
    if (interruptFlag == true) {
      Serial.println("Button pressed — aborting early");
      break;
      }
     // End session after 1 hour adjustable
    if (SESSION_MS != 0 && (millis() - compressorOnAt >= SESSION_MS)) {
      Serial.println("Session limit reached.");
      compressorOff();
      return;
    }
    delay(10);
  }
}

void state_4() {
  if (interruptFlag == true || cooldownRequired != 0) return;
  Serial.println("State 4");
  
  digitalWrite(WATER_PUMP, LOW);
  detachInterrupt(digitalPinToInterrupt(PUSH_BUTTON)); 
  delay(10);
  digitalWrite(SOLENOID, HIGH);
  delay(time_to_release);
  digitalWrite(SOLENOID, LOW);
  delay(100);
  attachInterrupt(digitalPinToInterrupt(PUSH_BUTTON), toggleSystemState, FALLING);
  delay(1000);
}

void sleep()
{
    wentToSleepAt_time = millis();
    Serial.println("Sleeping");
    system_on = false;
    compressorOff();
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(WATER_PUMP, LOW);
    
    detachInterrupt(digitalPinToInterrupt(PUSH_BUTTON)); 
    delay(10);
    digitalWrite(SOLENOID, LOW);
    digitalWrite(motorPinA, LOW);
    digitalWrite(motorPinB, LOW);
    delay(100);
    attachInterrupt(digitalPinToInterrupt(PUSH_BUTTON), toggleSystemState, FALLING);
}

void loop() {
    
  unsigned long current_time = millis();
  
  //check if power button was pushed
 if (interruptFlag && int_enable) {
    interruptFlag = false; 
    delay(300);
    if(digitalRead(PUSH_BUTTON) != LOW) return;
    system_on = !system_on;
    Serial.print("System: ");
    Serial.println(system_on);
    if(!system_on)
    {
      sleep();
      return;
    }
  }

  if (cooldownRequired && (millis() - compressorOffAt >= COOLDOWN_MS)) {
  cooldownRequired = false;
  EEPROM.update(EE_COOLDOWN_FLAG_ADDR, 0);  
}

  if(cooldownRequired) {
    delay(10);
    return;
  }
  if (system_on) {
    digitalWrite(POWER_LED, HIGH);
    state_1();
    state_2();
    state_3();
    state_4();
    
  } else {
    delay(10);
    if (current_time - last_blink_time >= 1000) {
      led_state = !led_state;
      digitalWrite(POWER_LED, led_state ? HIGH : LOW);
      last_blink_time = current_time;
    }
  }
}
