
const int BUTTON_PIN = 2;
const int LED_PIN_1 = 3;
const int LED_PIN_2 = 4;

const int BUTTON_SAMPLING_MS = 10;
const int LONG_PRESS_NUM_SAMPLES = 75;

long last_button_read_millis = 0;
int button_pressed_samples = 0;
boolean last_long_pressed = false;

int num_presets = 4;
int num_banks = 3;

int current_preset = -1; // -1 is initial state
int current_bank = 0;


void setup() {
  
  // Serial.begin(9600);
  // Serial.println("init");
  
  Serial.begin(31250); // init for midi
 
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  led_off();
  
}

void loop() {
  
  if (millis() - last_button_read_millis < 0) // overflow
    last_button_read_millis = 0;
    
  if (millis() - last_button_read_millis > BUTTON_SAMPLING_MS) {
        
    boolean button_state = (digitalRead(BUTTON_PIN) == HIGH)?true:false;
    
    if (button_state)
      button_pressed_samples++;
   
    if (button_pressed_samples >= LONG_PRESS_NUM_SAMPLES) {
      button_pressed_samples = 0;
      last_long_pressed = true;
      long_press();
    }
    
    if (!button_state) {
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

void short_press() {
  if (current_preset == -1) {
     current_preset = 0;
  } else {
    current_preset++;
    if (current_preset == num_presets)
      current_preset = 0;
  }
  load_preset();
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
