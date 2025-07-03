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
#define wake_time 3600000 //1hour
#define time_to_ice 700000    
#define time_to_release 10000 
#define time_to_fill_tray 13000 
const unsigned long MOTOR_TIMEOUT = 6000;

// === LOGIC ===
bool motorDirectionCW = true; //CW dumps ice and CCW loads water
bool ICE_FULL = false;
bool NO_WATER = false;
bool system_on = true;\
unsigned long last_blink_time = 0;
unsigned long wentToSleepAt_time = 0;
bool led_state = false;
volatile bool interruptFlag = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 500; // milliseconds

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
}

void toggleSystemState() {
  delayMicroseconds(200);
  unsigned long currentTime = millis();

  // Check if enough time has passed since the last interrupt
  if (digitalRead(PUSH_BUTTON) == LOW && currentTime - lastInterruptTime > debounceDelay) {
    interruptFlag = true;
    lastInterruptTime = currentTime;
    Serial.println("INTERRUPT TRIGGERED");

  }
}

// === MOTOR LOGIC ===
//returns true if motor reaches unload position in time
bool moveToload() 
{
  motorDirectionCW = false;
  bool attemptOne = false;
  bool attemptTwo = false;
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
    if(millis() - startTime > 400 && digitalRead(BOT_LIMIT) == LOW && attemptOne == false && digitalRead(TOP_LIMIT) == HIGH)
    {
      attemptOne = true;
        if(digitalRead(TOP_LIMIT) == LOW) 
      {
        stopMotor();
        Serial.println("@top");
        return true; //already at top
      }
        reverseMotor();
    }
     if(millis() - startTime > 800 && digitalRead(BOT_LIMIT) == LOW && attemptTwo == false && digitalRead(TOP_LIMIT) == HIGH)
    {
      attemptTwo = true;
        if(digitalRead(TOP_LIMIT) == LOW) 
      {
        stopMotor();
        Serial.println("@top");
        return true; //already at top
      }
        reverseMotor();
    }
    
  }
  stopMotor();
  if(digitalRead(TOP_LIMIT) == HIGH) return false; //motor did not reach proper positioning in time
}

//returns true if motor reaches load position in time
bool moveToUnload() 
{
  motorDirectionCW = true;
  bool attemptOne = false;
  bool attemptTwo = false;
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
    if(millis() - startTime > 400 && digitalRead(TOP_LIMIT) == LOW && attemptOne == false && digitalRead(BOT_LIMIT) == HIGH)
    {
      attemptOne = true;
        reverseMotor();
    }
      if(digitalRead(BOT_LIMIT) == LOW) 
      {
        stopMotor();
        Serial.println("@bot");
        return true; //already at bottom
      }
     if(millis() - startTime > 800 && digitalRead(TOP_LIMIT) == LOW && attemptTwo == false && digitalRead(BOT_LIMIT) == HIGH)
    {
      attemptTwo = true;
        if(digitalRead(BOT_LIMIT) == LOW) 
      {
        stopMotor();
        Serial.println("@bot");
        return true; //already at bottom
      }
        reverseMotor();
    }
  }
  stopMotor();
  if(digitalRead(BOT_LIMIT) == HIGH) return false; //motor did not reach proper positioning in time
}

void stopMotor() {
  digitalWrite(motorPinA, LOW);
  digitalWrite(motorPinB, LOW);
  delay(10);
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
  if( digitalRead(TOP_LIMIT) == HIGH && digitalRead(BOT_LIMIT) == HIGH)
  {
    Serial.println("ERROR: Trying to reverse in transit");
    stopMotor();
    return;
  }
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
  while(!moveToUnload()) {}
  Serial.println("NOW UNLOADED");
}

void loadTray()
{
  while(!moveToload()) {}
  Serial.println("NOW LOADED");
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
  float volt = infra*(5.0)/(1023); //4.03V IR light det and 4.82v with hand blocking sensor
  Serial.print("ice sensor: ");
  Serial.println(volt);
  ICE_FULL = (volt>=4.5);
  return ICE_FULL;
}

void state_1() {
  if (interruptFlag == true) return;
  Serial.println("State 1");
  unloadTray();

  // Wait until ice is NOT full
  if (check_ice_full()) {
    Serial.println("Ice bin full. Retrying in 60 sec...");
    delay(60000);
    if (check_ice_full()) {
      sleep();
    }
  }
}

void state_2() {
  if (interruptFlag == true) return;
  Serial.println("State 2");
  loadTray();
  delay(1000);
  Serial.println("Water is being loaded");
  pump_water();
  if (NO_WATER) {
    Serial.println("No water detected. Retrying in 60 sec...");
    delay(60000);
    pump_water();
    if (NO_WATER){
      sleep();
    }
  }
}

void state_3() {
  Serial.println("State 3");
  digitalWrite(COMPRESSOR, HIGH);
  digitalWrite(FAN_PIN, HIGH);
  unsigned long startTime = millis();
  while (millis() - startTime < time_to_ice) {
    if (interruptFlag == true) {
      Serial.println("Button pressed during ice cycle — aborting early");
      break;
      }
    

    delay(10);
  }
}

void state_4() {
  if (interruptFlag == true) return;
  Serial.println("State 4");
  digitalWrite(COMPRESSOR, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(SOLENOID, HIGH);
  delay(time_to_release);
  digitalWrite(SOLENOID, LOW);
}

void sleep()
{
    wentToSleepAt_time = millis();
    Serial.println("Sleeping");
    system_on = false;
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(WATER_PUMP, LOW);
    digitalWrite(SOLENOID, LOW);
    digitalWrite(COMPRESSOR, LOW);
    digitalWrite(motorPinA, LOW);
    digitalWrite(motorPinB, LOW);
}

void loop() {
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(WATER_PUMP, LOW);
  digitalWrite(SOLENOID, LOW);
  digitalWrite(COMPRESSOR, LOW);
  digitalWrite(motorPinA, LOW);
  digitalWrite(motorPinB, LOW);
    
  unsigned long current_time = millis();
 if (interruptFlag) {
    interruptFlag = false;  // Clear flag
    if(!ICE_FULL && !NO_WATER)
    {
    system_on = !system_on;
    Serial.println(system_on);
    if(!system_on) sleep();
    }
  }

  if (system_on) {
    digitalWrite(POWER_LED, HIGH);
    
    if (system_on) state_1();
    if (system_on) state_2();
    if (system_on) state_3();
    if (system_on) state_4();
    
  } else {
    if (current_time - last_blink_time >= 1000) {
      led_state = !led_state;
      digitalWrite(POWER_LED, led_state ? HIGH : LOW);
      last_blink_time = current_time;
    }

    
  }

  }
}
