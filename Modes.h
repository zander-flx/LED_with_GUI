// variables
const uint8_t REPEAT = 3;

// Updates LCD line 1 with cycle progress
void showCycle(uint8_t c){
  lcd.setCursor(0, 1);
  lcd.print("Cycle: ");
  lcd.print(c);
  lcd.print("/3   ");
}

////////// Mode 0: OFF
void led_off(){
  for(int i = 0; i < NUM_LEDS; i++) digitalWrite(LED[i], 0);
}

////////// Mode 1: Blink
void led_blink(){
  for(int i = 0; i < REPEAT; i++){
    //------------------------------------------------------------
    // leds ON
    showCycle(i + 1);
    
    for(int i = 0; i < NUM_LEDS; i++) digitalWrite(LED[i], 1);
    wait(1000);
    // leds OFF
    for(int i = 0; i < NUM_LEDS; i++) digitalWrite(LED[i], 0);
    wait(1000);
    //------------------------------------------------------------
    
  }
}

////////// Mode 2: Alternate
void led_alt(){
  for(int i = 0; i < REPEAT; i++){
    //------------------------------------------------------------
    // toggle first half of leds
    showCycle(i + 1);

    for(int i = 0; i < NUM_LEDS/2; i++) digitalWrite(LED[i], 1);
    wait(1000);
    for(int i = 0; i < NUM_LEDS/2; i++) digitalWrite(LED[i], 0);
    wait(1000);
    // toggle second half of leds
    for(int i = NUM_LEDS-1; i > (NUM_LEDS/2)-1; i--) digitalWrite(LED[i], 1);
    wait(1000);
    // turn OFF second half of leds
    for(int i = NUM_LEDS-1; i > (NUM_LEDS/2)-1; i--) digitalWrite(LED[i], 0);
    wait(1000);
    //------------------------------------------------------------
    
  }
}

////////// Mode 3: Running Lights
void led_run(){
  const uint8_t DELAY = 250;
  for(int i = 0; i < REPEAT; i++){
    //------------------------------------------------------------
    // forward run
    showCycle(i + 1);

    for(int i = 0; i < NUM_LEDS; i++){
      digitalWrite(LED[i], 1);
      wait(DELAY);
      digitalWrite(LED[i], 0);
    }
    // reverse run
    for(int i = NUM_LEDS-1; i >= 0; i--){
      digitalWrite(LED[i], 1);
      wait(DELAY);
      digitalWrite(LED[i], 0);
    }
    //------------------------------------------------------------
    
  }
}