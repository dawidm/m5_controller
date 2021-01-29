
#include <EEPROM.h>

//#define DEBUG

#ifdef DEBUG
#define D(x) x
#define ND(x) do{}while(0);
#else
#define D(x) do{}while(0);
#define ND(x) x
#endif


const byte BUTTON_1_PIN = 2;
const byte BUTTON_2_PIN = 3;

const byte BUTTON_SAMPLING_MS = 10;
const byte LONG_PRESS_NUM_SAMPLES = 50; // long press time = BUTTON_SAMPLING_MS * LONG_PRESS_NUM_SAMPLES

const byte LED_1_PIN_R = 7; // red
const byte LED_1_PIN_G = 8; // green
const byte LED_1_PIN_B = 9; // blue
const byte LED_2_PIN_R = 4; // red
const byte LED_2_PIN_G = 5; // green
const byte LED_2_PIN_B = 6; // blue

const byte LED_COLOR_RED = 1;
const byte LED_COLOR_GREEN = 2;
const byte LED_COLOR_BLUE = 3;
const byte LED_COLOR_YELLOW = 4; // red + green
const byte LED_COLOR_CYAN = 5; // green + blue
const byte LED_COLOR_MAGENTA = 6; // red + blue

const byte MAX_BANKS = 6;
const byte MAX_PRESETS = 4;
const byte MIN_BANKS = 2;
const byte MIN_PRESETS = 2;
const byte DEF_N_BANKS = 3;
const byte DEF_N_PRESETS = 3;

const byte EEPROM_SETTINGS_STORED = 0;
const byte EEPROM_SETTINGS_STORED_VAL = 0;
const byte EEPROM_N_BANKS_ADDR = 1;
const byte EEPROM_N_PRESETS_ADDR = 2;
const byte EEPROM_BANK_ADDR = 3;
const byte EEPROM_PRESET_ADDR = 4;

long last_button1_read_millis = 0;
int button1_pressed_samples = 0; // number of consecutive samples with button pressed
boolean last_button1_long_pressed = false; // flag set after long press to avoid triggering short press on button release

long last_button2_read_millis = 0;
int button2_pressed_samples = 0;
boolean last_button2_long_pressed = false;

byte num_banks = DEF_N_BANKS;
byte num_presets = DEF_N_PRESETS;

int current_preset = 0;
int current_bank = 0;

boolean setup_mode = false;
boolean setup_mode_waiting_release = true; // set false after releasing button when started in setup mode
byte setup_step = 0; // 0 - setting number of banks, 1 - setting number of presets
byte setup_n_banks = 0;
byte setup_n_presets = 0;

boolean no_bank_mode = false;

boolean two_preset_mode = false;
boolean after_bank_switch = false; // the state after bank switch, waiting for choosing preset 1/2, two leds are on

void setup() {

#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("init");
#else
  Serial.begin(31250); // init for midi
#endif

  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(LED_1_PIN_R, OUTPUT);
  pinMode(LED_1_PIN_G, OUTPUT);
  pinMode(LED_1_PIN_B, OUTPUT);
  pinMode(LED_2_PIN_R, OUTPUT);
  pinMode(LED_2_PIN_G, OUTPUT);
  pinMode(LED_2_PIN_B, OUTPUT);
  led1_off();
  led2_off();

  if (digitalRead(BUTTON_1_PIN) == LOW) {
    
    D(Serial.println("setup mode step 1");)
    setup_mode = true;
    led1_color(LED_COLOR_RED);

  } else {

    D(Serial.println("normal mode");)

    if (EEPROM.read(EEPROM_SETTINGS_STORED) == EEPROM_SETTINGS_STORED_VAL) {
      byte nbanks = EEPROM.read(EEPROM_N_BANKS_ADDR);
      if (nbanks == 0) {
        no_bank_mode = true;
        num_banks = 0;
        num_presets = 4;
        D(Serial.println("no banks mode");)
      } else {
        byte npresets = EEPROM.read(EEPROM_N_PRESETS_ADDR);
        if (npresets == 0) {
          two_preset_mode = true;
          num_presets = 2;
          D(Serial.println("two preset mode");)
        } else {
          num_banks = check_n_banks(nbanks);
          num_presets = check_n_presets(npresets);
          D(Serial.println("multiple bank-preset mode");)
        }
      }
    }

    current_bank = EEPROM.read(EEPROM_BANK_ADDR);
    current_preset = EEPROM.read(EEPROM_PRESET_ADDR);
    if (current_bank >= num_banks)
      current_bank = 0;
    if (current_preset >= num_presets)
      current_preset = 0;

    D(Serial.print("banks ");)
    D(Serial.print(num_banks);)
    D(Serial.print(" presets ");)
    D(Serial.println(num_presets);)
    if (two_preset_mode)

    load_preset();

  }

}

void loop() {

  if (millis() - last_button1_read_millis < 0) // overflow
    last_button1_read_millis = 0;

  if (millis() - last_button1_read_millis > BUTTON_SAMPLING_MS) {

    boolean button_state = (digitalRead(BUTTON_1_PIN) == LOW) ? true : false;

    if (button_state)
      button1_pressed_samples++;

    if (button1_pressed_samples >= LONG_PRESS_NUM_SAMPLES) {
      button1_pressed_samples = 0;
      last_button1_long_pressed = true;
      long_b1_press();
    }

    if (!button_state) {
      if (button1_pressed_samples > 0 & !last_button1_long_pressed & !setup_mode_waiting_release) {
        button1_pressed_samples = 0;
        short_b1_press();
      }
      last_button1_long_pressed = false;
      setup_mode_waiting_release = false;
      button1_pressed_samples = 0;
    }

    last_button1_read_millis = millis();
  }

  if (millis() - last_button2_read_millis < 0) // overflow
    last_button2_read_millis = 0;

  if (millis() - last_button2_read_millis > BUTTON_SAMPLING_MS) {

    boolean button_state = (digitalRead(BUTTON_2_PIN) == LOW) ? true : false;

    if (button_state)
      button2_pressed_samples++;

    if (button2_pressed_samples >= LONG_PRESS_NUM_SAMPLES) {
      button2_pressed_samples = 0;
      last_button2_long_pressed = true;
      long_b2_press();
    }

    if (!button_state) {
      if (button2_pressed_samples > 0 & !last_button2_long_pressed) {
        button2_pressed_samples = 0;
        short_b2_press();
      }
      last_button2_long_pressed = false;
      button2_pressed_samples = 0;
    }

    last_button2_read_millis = millis();
  }

}

void short_b1_press() {

  if (setup_mode) {

    if (setup_step == 0)
      setup_n_banks++;
    if (setup_step == 1)
      setup_n_presets++;

  } else if (no_bank_mode) {

    current_preset = 0;
    load_preset();

  } else if (two_preset_mode) {

    after_bank_switch = false;
    if (current_preset == 0) {
      // off
      current_preset = -1;
      load_preset();
    } else {
      current_preset = 0;
      load_preset();
    }

  } else {

    // change bank
    if (current_preset == -1) {
      current_preset = 0;
    } else {
      current_bank++;
      if (current_bank == num_banks)
        current_bank = 0;
      current_preset = 0;
    }
    load_preset();

  }
}

void long_b1_press() {

  if (setup_mode) {

    if (setup_mode_waiting_release)
      return;
    setup_step++;
    if (setup_step == 1) {
      if (setup_n_banks == 0) { // no banks mode
        D(Serial.println("no banks mode");)
        setup_mode = false;
        no_bank_mode = true;
        EEPROM.write(EEPROM_N_BANKS_ADDR, 0);
        EEPROM.write(EEPROM_SETTINGS_STORED, EEPROM_SETTINGS_STORED_VAL);
        led1_off();
      } else {
        led1_off();
        led2_color(LED_COLOR_RED);
        D(Serial.println("setup mode step 2");)
      }
    }
    if (setup_step == 2) {
      setup_mode = false;
      num_banks = check_n_banks(setup_n_banks);
      EEPROM.write(EEPROM_N_BANKS_ADDR, num_banks);

      if (setup_n_presets == 0) { // two preset mode
        D(Serial.println("two presets mode ");)
        D(Serial.print("banks ");)
        D(Serial.println(num_banks);)
        num_presets = 2;
        two_preset_mode = true;
        EEPROM.write(EEPROM_N_PRESETS_ADDR, 0);
      } else { // mulit bank-preset mode
        num_presets = check_n_presets(setup_n_presets);
        D(Serial.println("multiple bank-preset mode");)
        D(Serial.print("banks ");)
        D(Serial.print(num_banks);)
        D(Serial.print(" presets ");)
        D(Serial.println(num_presets);)
        EEPROM.write(EEPROM_N_PRESETS_ADDR, num_presets);
        led2_off();
      }
      EEPROM.write(EEPROM_SETTINGS_STORED, EEPROM_SETTINGS_STORED_VAL);
      store_bank_preset(true);
      current_bank = 0;
      current_preset = 0;
      load_preset();
    }

  } else if (no_bank_mode) {

    current_preset = 2;
    load_preset();

  } else if (two_preset_mode) {

    current_bank--;
    if (current_bank < 0)
      current_bank = num_banks - 1;
    current_preset = -1;
    after_bank_switch = true;
    load_preset();

  } else {

    if (current_bank != 0 || current_preset != 0) {
      current_bank = 0;
      current_preset = 0;
      load_preset();
    }

  }
}

void short_b2_press() {

  if (setup_mode) {

  } else if (no_bank_mode) {

    current_preset = 1;
    load_preset();

  } else if (two_preset_mode) {

    after_bank_switch = false;
    if (current_preset == 1) {
      // off
      current_preset = -1;
      load_preset();
    } else {
      current_preset = 1;
      load_preset();
    }

  } else {

    // change preset
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

void long_b2_press() {
  if (setup_mode) {

  } else if (no_bank_mode) {

    current_preset = 3;
    load_preset();

  } else if (two_preset_mode) {

    current_bank++;
    if (current_bank >= num_banks)
      current_bank = 0;
    current_preset = -1;
    after_bank_switch = true;
    load_preset();

  } else {

    if (current_bank != 0 || current_preset != 1) {
      current_bank = 0;
      current_preset = 1;
      load_preset();
    }

  }
}

void load_preset() {

  D(Serial.print("loading bank ");)
  D(Serial.print(current_bank);)
  D(Serial.print(" preset ");)
  D(Serial.println(current_preset);)

  update_preset_diodes();

  if (current_preset == -1) {
    m5_bypass(true);
  } else {
    byte m5_preset = current_bank * num_presets + current_preset;
    m5_preset_change(m5_preset);
    if (two_preset_mode)
      m5_bypass(false);
    store_bank_preset(false);
  }
}

void update_preset_diodes() {

  if (no_bank_mode) {

    switch (current_preset) {
      case 0:
        led1_color(LED_COLOR_RED);
        led2_off();
        break;
      case 1:
        led2_color(LED_COLOR_RED);
        led1_off();
        break;
      case 2:
        led1_color(LED_COLOR_GREEN);
        led2_off();
        break;
      case 3:
        led2_color(LED_COLOR_GREEN);
        led1_off();
        break;
    }

  } else if (two_preset_mode) {

    if (current_preset == -1) {

      if (after_bank_switch) {

        led1_bank_color();
        led2_bank_color();

      } else {

        led1_off();
        led2_off();

      }

    } else if (current_preset == 0) {

      led2_off();
      led1_bank_color();

    } else {

      led1_off();
      led2_bank_color();

    }

  } else {

    led1_bank_color();
    led2_preset_color();
  }

}

void led1_bank_color() {
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
      led1_color(LED_COLOR_MAGENTA);
      break;
    case 4:
      led1_color(LED_COLOR_CYAN);
      break;
    case 5:
      led1_color(LED_COLOR_YELLOW);
      break;
    default:
      led1_off();
  }
}

void led2_bank_color() {
  switch (current_bank) {
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
      led2_color(LED_COLOR_MAGENTA);
      break;
    case 4:
      led2_color(LED_COLOR_CYAN);
      break;
    case 5:
      led2_color(LED_COLOR_YELLOW);
      break;
    default:
      led1_off();
  }
}

void led1_preset_color() {
  switch (current_preset) {
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
      led1_color(LED_COLOR_MAGENTA);
      break;
    case 4:
      led1_color(LED_COLOR_CYAN);
      break;
    case 5:
      led1_color(LED_COLOR_YELLOW);
      break;
    default:
      led1_off();
  }
}

void led2_preset_color() {
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
      led2_color(LED_COLOR_MAGENTA);
      break;
    case 4:
      led2_color(LED_COLOR_CYAN);
      break;
    case 5:
      led2_color(LED_COLOR_YELLOW);
      break;
    default:
      led2_off();
  }
}

void store_bank_preset(boolean initial_values) {
  if (initial_values) {
    EEPROM.write(EEPROM_BANK_ADDR, 0);
    EEPROM.write(EEPROM_PRESET_ADDR, 0);
  } else {
    EEPROM.write(EEPROM_BANK_ADDR, current_bank);
    EEPROM.write(EEPROM_PRESET_ADDR, current_preset);
  }
}

void m5_bypass(boolean bypass) {

  D(Serial.print("bypass=");)
  D(Serial.println(bypass);)

  ND(Serial.write(176);) // control change
  ND(Serial.write(11);) // command number
  if (bypass)
    ND(Serial.write(0);)
  else
    ND(Serial.write(127);)

}

void m5_preset_change(byte preset) {

  D(Serial.print("m5preset=");)
  D(Serial.println(preset);)

  ND(Serial.write(192);) // midi program change
  ND(Serial.write(preset);) // program number

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
