byte right_licksensorPin = 4;
byte right_lickOnset = 11;
byte left_licksensorPin = 3;
byte left_lickOnset = 10;
//byte lickOffset = 12;

bool right_lickSensorState = 0;
bool right_burstinBout = 0;
bool left_lickSensorState = 0;
bool left_burstinBout = 0;

unsigned long right_timebursthigh = 0;
unsigned long right_timeburstlow = 0;
unsigned long left_timebursthigh = 0;
unsigned long left_timeburstlow = 0;

//unsigned long mininterburstinterval = 500; // minimum interval between burst during one lick (micros)
//unsigned long minburstduration = 8; //minimum duration for burst (micros)
unsigned long minlickgap = 0.2 * 1000;  // minimum gap between two licks (10ms, 10*1000 micros)

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(right_licksensorPin, INPUT);
  pinMode(right_lickOnset, OUTPUT);
  pinMode(left_licksensorPin, INPUT);
  pinMode(left_lickOnset, OUTPUT);
  //pinMode (lickOffset, OUTPUT);
  digitalWrite (right_licksensorPin, 0);//activate put up resistor
}

void loop() {

  right_lickSensorState = digitalRead(right_licksensorPin);
  left_lickSensorState= digitalRead(left_licksensorPin);
  left_lick_detection();
  right_lick_detection();
}

void left_lick_detection(){
if (left_lickSensorState == 0 && left_burstinBout == 0) {
    left_timeburstlow = micros();
  }
  if (left_lickSensorState == 1 && left_burstinBout == 0) {
    left_timebursthigh = micros();
    left_burstinBout = 1;
        //      Serial.println("lick detected");
    digitalWrite(left_lickOnset, 1);
    //      Serial.print("lick number is:");
    //      Serial.println(lickcounter);
    //delay(1);digitalWrite(lickOnset,0);
  }
  if (left_lickSensorState == 1 && left_burstinBout == 1) {
    left_timebursthigh = micros();
  }
  if (left_lickSensorState == 0 && left_burstinBout == 1) {
    left_timeburstlow = micros();
    if (left_timeburstlow - left_timebursthigh > minlickgap) {
      digitalWrite(left_lickOnset, 0);
      left_burstinBout = 0;
    }
  }
}

void right_lick_detection(){

  if (right_lickSensorState == 0 && right_burstinBout == 0) {
    right_timeburstlow = micros();
  }
  if (right_lickSensorState == 1 && right_burstinBout == 0) {
    right_timebursthigh = micros();
    right_burstinBout = 1;
        //      Serial.println("lick detected");
    digitalWrite(right_lickOnset, 1);
    //      Serial.print("lick number is:");
    //      Serial.println(lickcounter);
    //delay(1);digitalWrite(lickOnset,0);
  }
  if (right_lickSensorState == 1 && right_burstinBout == 1) {
    right_timebursthigh = micros();
  }
  if (right_lickSensorState == 0 && right_burstinBout == 1) {
    right_timeburstlow = micros();
    if (right_timeburstlow - right_timebursthigh > minlickgap) {
      digitalWrite(right_lickOnset, 0);
      right_burstinBout = 0;
    }
  }
}