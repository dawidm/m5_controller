
const int BUTTON_PIN = 2;

const int button_sampling_ms = 10;
const int long_press_num_samples = 75;

long last_button_read_millis = 0;
int button_pressed_samples = 0;
boolean last_long_pressed = false;

int num_presets = 4;
int num_banks = 2;

int current_preset = -1; // -1 is initial state
int current_bank = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("elo");
  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  
  if (millis() - last_button_read_millis < 0) // overflow
    last_button_read_millis = 0;
    
  if (millis() - last_button_read_millis > button_sampling_ms) {
        
    boolean button_state = (digitalRead(BUTTON_PIN) == HIGH)?true:false;
    
    if (button_state)
      button_pressed_samples++;
   
    if (button_pressed_samples >= long_press_num_samples) {
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
    Serial.println("bank switch");
    load_preset();
  }
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

void load_preset() {
  Serial.print("loading preset ");
  Serial.print(current_preset);
  Serial.print(" bank ");
  Serial.print(current_bank);
  int m5_preset = current_bank * num_presets + current_preset + 1;
  Serial.print("; m5 preset no ");
  Serial.println(m5_preset);
}
