
#include <EEPROM.h>

const int BUTTON_PIN = 2;
const int LED_PIN_1 = 3;
const int LED_PIN_2 = 4;

const int BUTTON_SAMPLING_MS = 10;
const int LONG_PRESS_NUM_SAMPLES = 75;

const int MAX_BANKS = 4;
const int MAX_PRESETS = 6;
const int MIN_BANKS = 2;
const int MIN_PRESETS = 2;
const int DEF_N_BANKS = 3;
const int DEF_N_PRESETS = 3;

const int EEPROM_SETTINGS_STORED = 0;
const int EEPROM_IS_STORED_VAL = 0;
const int EEPROM_N_BANKS_ADDR = 1;
const int EEPROM_N_PRESETS_ADDR = 2;

long last_button_read_millis = 0;
int button_pressed_samples = 0; // number of consecutive samples with button pressed
boolean last_long_pressed = false; // flag set after long press to avoid triggering short press on button release

byte num_presets = 4;
byte num_banks = 3;

int current_preset = -1; // -1 is initial state
int current_bank = 0;

boolean setup_mode = false;
boolean setup_mode_waiting_release = true; // set false after releasing button when started in setup mode
int setup_step = 0; // 0 - setting number of banks, 1 - setting number of presets
byte setup_n_banks = 0;
byte setup_n_presets = 0;

void setup() {

  Serial.begin(9600);
  Serial.println("init");

  //Serial.begin(31250); // init for midi

  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  led_off();

  if (digitalRead(BUTTON_PIN) == HIGH) {
    Serial.println("setup mode");
    setup_mode = true;
    led_col1();
  } else {
    Serial.println("normal mode");
    if (EEPROM.read(EEPROM_SETTINGS_STORED) == EEPROM_IS_STORED_VAL) {
      num_banks = check_n_banks(EEPROM.read(EEPROM_N_BANKS_ADDR));
      num_presets = check_n_presets(EEPROM.read(EEPROM_N_PRESETS_ADDR));
    } else {
      num_banks = DEF_N_BANKS;
      num_presets = DEF_N_PRESETS;
    }
    Serial.print("banks ");
    Serial.print(num_banks);
    Serial.print(" presets ");
    Serial.println(num_presets);
  }

}

void loop() {

  if (millis() - last_button_read_millis < 0) // overflow
    last_button_read_millis = 0;

  if (millis() - last_button_read_millis > BUTTON_SAMPLING_MS) {

    boolean button_state = (digitalRead(BUTTON_PIN) == HIGH) ? true : false;

    if (button_state)
      button_pressed_samples++;

    if (button_pressed_samples >= LONG_PRESS_NUM_SAMPLES) {
      button_pressed_samples = 0;
      last_long_pressed = true;
      long_press();
    }

    if (!button_state) {
      if (setup_mode_waiting_release)
        setup_mode_waiting_release = false;
      if (button_pressed_samples > 0 & last_long_pressed == false) {
        button_pressed_samples = 0;
        short_press();
      }
      last_long_pressed = false;
      button_pressed_samples = 0;
    }

    last_button_read_millis = millis();
  }

}

void long_press() {

  if (setup_mode) {
    
    if (setup_mode_waiting_release)
      return;
    setup_step++;
    if (setup_step == 1) {
      led_col2();
      Serial.println("setup step 2");
    }
    if (setup_step == 2) {
      setup_mode = false;
      num_banks = check_n_banks(setup_n_banks);
      num_presets = check_n_presets(setup_n_presets);
      Serial.print("banks ");
      Serial.print(num_banks);
      Serial.print(" presets ");
      Serial.println(num_presets);
      EEPROM.write(EEPROM_N_BANKS_ADDR, num_banks);
      EEPROM.write(EEPROM_N_PRESETS_ADDR, num_presets);
      EEPROM.write(EEPROM_SETTINGS_STORED, EEPROM_IS_STORED_VAL);
      led_off();
    }
    
  } else {

    if (current_preset == -1) {
      current_preset = 0;
    } else {
      current_bank++;
      if (current_bank == num_banks)
        current_bank = 0;
      current_preset = 0;
    }
    load_bank();
    load_preset();

  }
}

void short_press() {

  if (setup_mode) {

    if (setup_mode_waiting_release)
      return;

    if (setup_step == 0)
      setup_n_banks++;
    if (setup_step == 1)
      setup_n_presets++;

  } else {

    if (current_preset == -1) {
      current_preset = 0;
    } else {
      current_preset++;
      if (current_preset == num_presets)
        current_preset = 0;
    }
    load_preset();
  }

}

void load_bank() {
  switch (current_bank) {
    case 0:
      led_col1();
      break;
    case 1:
      led_col2();
      break;
    case 2:
      led_col3();
      break;
    default:
      led_off();
  }

}

void load_preset() {
  Serial.print("loading bank ");
  Serial.print(current_bank);
  Serial.print(" preset ");
  Serial.println(current_preset);
  int m5_preset = current_bank * num_presets + current_preset;
  Serial.write(192); // midi program change
  Serial.write(m5_preset); // program number
}

void led_off() {
  digitalWrite(LED_PIN_1, LOW);
  digitalWrite(LED_PIN_2, LOW);
}

void led_col1() {
  led_off();
  digitalWrite(LED_PIN_1, HIGH);
}

void led_col2() {
  led_off();
  digitalWrite(LED_PIN_2, HIGH);
}

void led_col3() {
  led_off();
  digitalWrite(LED_PIN_1, HIGH);
  digitalWrite(LED_PIN_2, HIGH);
}

byte check_n_banks(byte n_banks) {
  return max(min(n_banks, MAX_BANKS), MIN_BANKS);
}

byte check_n_presets(byte n_presets) {
  return max(min(n_presets, MAX_PRESETS), MIN_PRESETS);
}
