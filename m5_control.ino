
const int BUTTON_PIN = 2;

const int button_sampling_ms = 10;
const int long_press_num_samples = 75;

int last_button_read_millis = 0;
int button_pressed_samples = 0;
boolean last_long_pressed = false;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("elo");
  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  if (millis() - last_button_read_millis > button_sampling_ms) {
    
    boolean button_state = (digitalRead(BUTTON_PIN) == HIGH)?true:false;
    
    if (button_state)
      button_pressed_samples++;
   
    if (button_pressed_samples >= long_press_num_samples) {
      button_pressed_samples = 0;
      last_long_pressed = true;
      longPress();
    }
    
    if (!button_state) {
      if (button_pressed_samples > 0 & last_long_pressed == false) {
        button_pressed_samples = 0;
        shortPress();
      }
      last_long_pressed = false;
      button_pressed_samples = 0;
    }
    
    last_button_read_millis = millis();
  }

}

void longPress() {
  Serial.println("long press");
}

void shortPress() {
  Serial.println("press");
}
