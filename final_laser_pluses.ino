// convert square wave to pulses

byte sound_detection = 4; 
byte laser_trig = 9; 

int laser_on_time = 5; 
int laser_cycle = 50;

bool sound_trig_value = 0 ;
bool sound_state = 0;

unsigned long previous_time = 0;
unsigned long current_time = 0;

void setup() {
  pinMode(sound_detection, INPUT);
  pinMode(laser_trig, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  sound_trig_value = digitalRead(sound_detection);
  current_time = millis();
  if (sound_trig_value == 0 && sound_state == 0){
    sound_state = 0;
  }
  if (sound_trig_value == 1 && sound_state == 0){
    sound_state = 1;
    previous_time = current_time;
  }
  if (sound_trig_value == 1 && sound_state == 1){
    if (current_time - previous_time < laser_on_time){
      digitalWrite(laser_trig, HIGH);
    }
    if (current_time - previous_time > laser_on_time){
      digitalWrite(laser_trig, LOW);
      if(current_time - previous_time > laser_cycle){
        previous_time = current_time;
      }
    }
  }
  if (sound_trig_value == 0 && sound_state == 1){
    sound_state = 0;
    digitalWrite(laser_trig, LOW);
  }
}
