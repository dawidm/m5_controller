
#include <EEPROM.h>

const byte BUTTON_PIN = 2;

const byte BUTTON_SAMPLING_MS = 10;
const byte LONG_PRESS_NUM_SAMPLES = 75;

const byte LED_1_PIN_R = 3; // red
const byte LED_1_PIN_G = 4; // green
const byte LED_1_PIN_B = 5; // blue
const byte LED_2_PIN_R = 6; // red
const byte LED_2_PIN_G = 7; // green
const byte LED_2_PIN_B = 8; // blue

const byte LED_COLOR_RED = 1;
const byte LED_COLOR_GREEN = 2;
const byte LED_COLOR_BLUE = 3;
const byte LED_COLOR_YELLOW = 4; // red + green
const byte LED_COLOR_CYAN = 5; // green + blue
const byte LED_COLOR_MAGENTA = 6; // red + blue

const byte MAX_BANKS = 4;
const byte MAX_PRESETS = 6;
const byte MIN_BANKS = 2;
const byte MIN_PRESETS = 2;
const byte DEF_N_BANKS = 3;
const byte DEF_N_PRESETS = 3;

const byte EEPROM_SETTINGS_STORED = 0;
const byte EEPROM_SETTINGS_STORED_VAL = 0;
const byte EEPROM_N_BANKS_ADDR = 1;
const byte EEPROM_N_PRESETS_ADDR = 2;

long last_button_read_millis = 0;
int button_pressed_samples = 0; // number of consecutive samples with button pressed
boolean last_long_pressed = false; // flag set after long press to avoid triggering short press on button release

byte num_banks = DEF_N_BANKS;
byte num_presets = DEF_N_PRESETS;

int current_preset = -1; // -1 is initial state
int current_bank = 0;

boolean setup_mode = false;
boolean setup_mode_waiting_release = true; // set false after releasing button when started in setup mode
byte setup_step = 0; // 0 - setting number of banks, 1 - setting number of presets
byte setup_n_banks = 0;
byte setup_n_presets = 0;

void setup() {

  Serial.begin(9600);
  Serial.println("init");

  //Serial.begin(31250); // init for midi

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_1_PIN_R, OUTPUT);
  pinMode(LED_1_PIN_G, OUTPUT);
  pinMode(LED_1_PIN_B, OUTPUT);
  pinMode(LED_2_PIN_R, OUTPUT);
  pinMode(LED_2_PIN_G, OUTPUT);
  pinMode(LED_2_PIN_B, OUTPUT);
  led1_off();
  led2_off();

  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("setup mode");
    setup_mode = true;
    led1_color(LED_COLOR_RED);
  } else {
    Serial.println("normal mode");
    if (EEPROM.read(EEPROM_SETTINGS_STORED) == EEPROM_SETTINGS_STORED_VAL) {
      num_banks = check_n_banks(EEPROM.read(EEPROM_N_BANKS_ADDR));
      num_presets = check_n_presets(EEPROM.read(EEPROM_N_PRESETS_ADDR));
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

    boolean button_state = (digitalRead(BUTTON_PIN) == LOW) ? true : false;

    if (button_state)
      button_pressed_samples++;

    if (button_pressed_samples >= LONG_PRESS_NUM_SAMPLES) {
      button_pressed_samples = 0;
      last_long_pressed = true;
      long_press();
    }

    if (!button_state) {
      if (button_pressed_samples > 0 & !last_long_pressed & !setup_mode_waiting_release) {
        button_pressed_samples = 0;
        short_press();
      }
      last_long_pressed = false;
      setup_mode_waiting_release = false;
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
      led1_off();
      led2_color(LED_COLOR_RED);
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
      EEPROM.write(EEPROM_SETTINGS_STORED, EEPROM_SETTINGS_STORED_VAL);
      led2_off();
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
      led1_color(LED_COLOR_RED);
      break;
    case 1:
      led1_color(LED_COLOR_GREEN);
      break;
    case 2:
      led1_color(LED_COLOR_BLUE);
      break;
    case 3:
      led1_color(LED_COLOR_CYAN);
      break;
    case 4:
      led1_color(LED_COLOR_MAGENTA);
      break;
    case 5:
      led1_color(LED_COLOR_YELLOW);
      break;
    default:
      led1_off();
  }

}

void load_preset() {

  switch (current_preset) {
    case 0:
      led2_color(LED_COLOR_RED);
      break;
    case 1:
      led2_color(LED_COLOR_GREEN);
      break;
    case 2:
      led2_color(LED_COLOR_BLUE);
      break;
    case 3:
      led2_color(LED_COLOR_CYAN);
      break;
    case 4:
      led2_color(LED_COLOR_MAGENTA);
      break;
    case 5:
      led2_color(LED_COLOR_YELLOW);
      break;
    default:
      led2_off();
  }

  Serial.print("loading bank ");
  Serial.print(current_bank);
  Serial.print(" preset ");
  Serial.println(current_preset);
  byte m5_preset = current_bank * num_presets + current_preset;
  Serial.write(192); // midi program change
  Serial.write(m5_preset); // program number
}

void led1_off() {
  digitalWrite(LED_1_PIN_R, LOW);
  digitalWrite(LED_1_PIN_G, LOW);
  digitalWrite(LED_1_PIN_B, LOW);
}

void led2_off() {
  digitalWrite(LED_2_PIN_R, LOW);
  digitalWrite(LED_2_PIN_G, LOW);
  digitalWrite(LED_2_PIN_B, LOW);
}

void led1_color(byte color) {
  led_color(LED_1_PIN_R, LED_1_PIN_G, LED_1_PIN_B, color);
}

void led2_color(byte color) {
  led_color(LED_2_PIN_R, LED_2_PIN_G, LED_2_PIN_B, color);
}

void led_color(byte red_pin, byte green_pin, byte blue_pin, byte color) {

  digitalWrite(red_pin, LOW);
  digitalWrite(green_pin, LOW);
  digitalWrite(blue_pin, LOW);

  switch (color) {
    case LED_COLOR_RED:
      digitalWrite(red_pin, HIGH);
      break;
    case LED_COLOR_GREEN:
      digitalWrite(green_pin, HIGH);
      break;
    case LED_COLOR_BLUE:
      digitalWrite(blue_pin, HIGH);
      break;
    case LED_COLOR_YELLOW: // red + green
      digitalWrite(red_pin, HIGH);
      digitalWrite(green_pin, HIGH);
      break;
    case LED_COLOR_CYAN: // green + blue
      digitalWrite(green_pin, HIGH);
      digitalWrite(blue_pin, HIGH);
      break;
    case LED_COLOR_MAGENTA: // red + blue
      digitalWrite(red_pin, HIGH);
      digitalWrite(blue_pin, HIGH);
      break;
  }
}

byte check_n_banks(byte n_banks) {
  return max(min(n_banks, MAX_BANKS), MIN_BANKS);
}

byte check_n_presets(byte n_presets) {
  return max(min(n_presets, MAX_PRESETS), MIN_PRESETS);
}
