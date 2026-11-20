/*========================================================================
  LEDs Lightning Modes (LCD + Switches + Serial GUI)
  Description:
    6 LEDs, 16x2 I2C LCD, 2 mode switches and a Python serial GUI.
    Switch code: 00=OFF, 01=BLINK, 10=ALTERNATE, 11=RUN
    The GUI's "Next Mode" reflects switch input in real time, but the
    new mode is applied only AFTER the current mode finishes 3 cycles.
  
  Engineer:
  Felixander B. Doloso
==========================================================================*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// GPIOs
const uint8_t LED[] = {19, 18, 5, 25, 26, 27};
const uint8_t NUM_LEDS = sizeof(LED) / sizeof(LED[0]);
const uint8_t SW0 = 32;
const uint8_t SW1 = 33;

// LCD: I2C address 0x27, 16 chars, 2 lines
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables
uint8_t mode        = 0;    // active mode
int8_t  pendingMode = -1;   // requested next mode (GUI or switches)
uint8_t lastSw      = 0;    // last sampled switch state

uint8_t readSwitches(){
  return (digitalRead(SW1) << 1) | digitalRead(SW0);
}

const char* modeName(uint8_t m){
  switch(m){
    case 0:  return "OFF";
    case 1:  return "BLINK";
    case 2:  return "ALTERNATE";
    case 3:  return "RUN";
  }
  return "?";
}

void showMode(uint8_t m){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MODE: ");
  lcd.print(modeName(m));
  Serial.printf("MODE:%d\n", m); //This talks to the GUI through serial
}

// Polls the GUI (serial) and the switches. Called continuously,
// even while a mode is running its 3 cycles.
void checkInputs(){
  // ---- Serial (Python GUI): single digit 0-3, last command wins ----
  while(Serial.available()){
    String s = Serial.readStringUntil('\n');
    s.trim();
    if(s.length() == 1 && isDigit(s[0])){
      int next_mode = s.toInt();
      if(next_mode >= 0 && next_mode <= 3){
        pendingMode = next_mode;
        Serial.printf("PENDING:%d:GUI\n", next_mode);
      }
    }
  }

  // ---- Switches: notify the GUI as soon as they change ----
  uint8_t sw = readSwitches();
  if(sw != lastSw){
    lastSw      = sw;
    pendingMode = sw;
    Serial.printf("PENDING:%d:SW\n", sw);
  }
}

// Replacement for delay(): waits while keeping inputs polled,
// so the GUI sees switch changes in real time.
void wait(uint32_t ms){
  uint32_t start = millis();
  while(millis() - start < ms){
    checkInputs();
    delay(5);
  }
}

void setup(){
  Serial.begin(115200);

  // Inputs
  pinMode(SW0, INPUT);
  pinMode(SW1, INPUT);

  // Outputs
  for(int i = 0; i < NUM_LEDS; i++){
    pinMode(LED[i], OUTPUT);
    digitalWrite(LED[i], 0);
  }

  // LCD
  lcd.init();
  lcd.backlight();

  // Start with whatever the switches say
  mode   = readSwitches();
  lastSw = mode;
  showMode(mode);
}

#include "Modes.h"

void loop(){
  checkInputs();

  // Reached only after the previous mode finished its 3 cycles
  if(pendingMode >= 0){
    mode        = pendingMode;
    pendingMode = -1;
    showMode(mode);          // updates LCD and tells the GUI
  }

  // LEDs mode
  switch(mode){
    case 0:  led_off();  delay(50);  break;  // OFF: idle, keep polling inputs
    case 1:  led_blink();             break;  // each blocks for 3 full cycles
    case 2:  led_alt();               break;
    case 3:  led_run();               break;
  }
}