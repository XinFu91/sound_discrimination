// this code is used to train the mice to first do either left or right licks to collect milk reward contingent on the frequency of the sound 
// Mice initiate a trial by poking the central port, which triggers the speaker to play a sound. After sound presentation, mice need to lick the correct spout to collect the reward
// otherwise the mice will get a timeout of 10 seconds for no rewards.

byte pokeSensor_pin = 6; 
byte sound_pin = 3; // to left and right DAQ
byte licksensorPin_left = 4;
byte licksensorPin_right = 5;
byte pumpTrigger_left = 8; //to DAQ
byte trialTrigger_left = 11;
byte trialTrigger_right = 12;
byte pumpTrigger_right = 10; //to DAQ
byte pokeOnsetTrigger = 7; //to DAQ
byte right_LED = 36;
byte left_LED = 34;
byte laser_on_trigger = 2;

bool pokeSensorState = 0;
bool pokeState = 0;
bool lickSensorState_left = 0;
bool lickSensorState_right = 0;
bool left_lick_state = 0;
bool right_lick_state = 0;
bool left_training = 1; 
bool left_trial_state = 0;
bool right_trial_state = 0;


int trialCounter = 0;
int min_poke_gap = 3000; // sound will only play once  even if more than one pokes are detected in this time window
int lick_time_window = 3*1000; // mice need to lick inside this time window to activate the pump
int lickNum = 1; // number of licks needed to activate the pump
int lickCounter = 0; 
int minLickDuration = 1; // minimum duration to be considered as a lick
int minPokeDuration = 100; // minimum duration for a successful poke
int sound_delay = 300; //300ms delay to nose poke before the sound is turned on

const int refresherNum = 0; // number of refresher trials before real experiments 
const int blockNumber = 15;
const int trialInBlock = 20; // number of trial in a block
const int extraTraining = 10;
const int trialNumber=refresherNum+blockNumber*trialInBlock+extraTraining;

int trial_array[trialNumber];
int laser_trial[trialNumber];
//int interval_array[trialNumber];
int refresher_array[refresherNum];
int laser_refresher_array[refresherNum];
// int block_array[trialInBlock]={1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1}; // 0 indicates left lick trials, 1 indicates right lick trials.
// int block_array[trialInBlock]={1,0,1,0,1,1,1,0,1,1,0,0,1,0,0,1,0,0,0,1};
int laser_block_array[trialInBlock]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
// int block_array[trialInBlock]={1,1,0,0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0};
int block_array[trialInBlock]={1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0};
// int interval_block_array[trialInBlock]={25,18,20,24,18,16,25,20,17,22};
//int interval_block_array[trialInBlock]={5,5,5,5,5,5,5,5,5,5};

unsigned long current_time = 0;
unsigned long previous_time = 0;
unsigned long poke_off_time = 0;
unsigned long poke_onset_time = 0;
unsigned long poke_on_time = 0;
unsigned long poke_offset_time = 0;
unsigned long poke_duration = 0;
unsigned long left_trial_time_on = 0;
unsigned long right_trial_time_on = 0;
unsigned long current_left_trial_time = 0;
unsigned long current_right_trial_time = 0;
unsigned long left_trial_on_time = 0;
unsigned long right_trial_on_time = 0;

unsigned long left_lick_off_time = 0;
unsigned long left_lick_onset_time = 0;
unsigned long left_lick_on_time = 0;
unsigned long left_lick_offset_time = 0;
unsigned long left_lick_duration = 0;

unsigned long right_lick_off_time = 0;
unsigned long right_lick_onset_time = 0;
unsigned long right_lick_on_time = 0;
unsigned long right_lick_offset_time = 0;
unsigned long right_lick_duration = 0;

void setup() {
  pinMode(pokeSensor_pin, INPUT);
  digitalWrite(pokeSensor_pin, HIGH); //pull up the pin FOR IR sensor
  pinMode(licksensorPin_left, INPUT);
  pinMode(licksensorPin_right, INPUT);
  pinMode(pumpTrigger_left, OUTPUT);
  pinMode(pumpTrigger_right, OUTPUT);
  pinMode(sound_pin, OUTPUT);
  pinMode(laser_on_trigger, OUTPUT);
  pinMode(pokeOnsetTrigger,OUTPUT);
  pinMode(trialTrigger_left, OUTPUT);
  pinMode(trialTrigger_right, OUTPUT);
  pinMode(left_LED, OUTPUT);
  pinMode(right_LED, OUTPUT);
  Serial.begin(9600);
  Serial.println("training begin!");

  int i = 0;
  int j = 0;
    for(i=0;i<blockNumber;i++){
      for(j=0;j<trialInBlock;j++){
        trial_array[refresherNum+i*trialInBlock+j]=block_array[j];
        laser_trial[refresherNum+i*trialInBlock+j]=laser_block_array[j];
        //interval_array[refresherNum+i*trialInBlock+j]=interval_block_array[j];
      }   
    }
    for (i=0;i<refresherNum;i++){
      trial_array[i]=1;
      laser_trial[i]=0;
      //interval_array[i]=5;
    }
    for (i=0;i<extraTraining;i++){
      trial_array[refresherNum+blockNumber*trialInBlock+i]=0;
      laser_trial[refresherNum+blockNumber*trialInBlock+i]=0;
     // interval_array[refresherNum+blockNumber*trialInBlock+i]=20;
    }
    for (i=0;i<trialNumber;i++){
      Serial.print(i);
      Serial.print(" trial is : ");
      Serial.print(trial_array[i]);
      Serial.print(" laser is : ");
      Serial.println(laser_trial[i]);
      // Serial.print(" with interval: ");
      // Serial.println(interval_array[i]);
    }
}

void loop() {
  if(trialCounter<trialNumber){
    current_time = millis();
    pokeSensorState = digitalRead(pokeSensor_pin);
    lickSensorState_left = digitalRead(licksensorPin_left);
    lickSensorState_right = digitalRead(licksensorPin_right);
    nose_poke_detection();// detect a successful nose poke and play a sound, set a time window for lick detection and milk delivery 
    if (left_trial_state ==1){
      current_left_trial_time = millis();
      if (current_left_trial_time - left_trial_time_on < lick_time_window){
        left_lick_detection();
      }
      if (current_left_trial_time - left_trial_time_on >= lick_time_window){
        noTone (sound_pin);
        digitalWrite(laser_on_trigger, LOW);
        left_trial_state = 0; // stop left lick detection after the time window
        digitalWrite(left_LED, LOW);
        digitalWrite(trialTrigger_left, LOW);
      }
    }
    if (right_trial_state ==1){
      current_right_trial_time = millis();
      if (current_right_trial_time - right_trial_time_on < lick_time_window){
        right_lick_detection();
      }
      if (current_right_trial_time - right_trial_time_on >= lick_time_window){
        noTone (sound_pin);
        digitalWrite(laser_on_trigger, LOW);
        right_trial_state = 0; // stop left lick detection after the time window
        digitalWrite(right_LED, LOW);
        digitalWrite(trialTrigger_right, LOW);
      }
    }

  }
}

void nose_poke_detection(){
  if (pokeState == 0 && pokeSensorState == 1){
      poke_off_time = millis();
    }
    else if (pokeState == 0 && pokeSensorState == 0){
      poke_onset_time = poke_off_time;
      poke_on_time = millis();
      digitalWrite(pokeOnsetTrigger, HIGH);
      pokeState = 1;
   }
   else if (pokeState == 1 && pokeSensorState ==0){
     poke_on_time = millis();
   }
   else {
      digitalWrite(pokeOnsetTrigger, LOW);
      
      pokeState = 0;
      poke_offset_time = poke_on_time;
      poke_duration = poke_offset_time-poke_onset_time;
      if (poke_duration >= minPokeDuration && poke_offset_time - previous_time > min_poke_gap){
        if(trial_array[trialCounter] == 0){
          if(laser_trial[trialCounter] == 1){
            digitalWrite(laser_on_trigger, HIGH);
          }
          delay(sound_delay);
          tone (sound_pin, 3000);
          // tone (sound_pin, 15000);
          // digitalWrite(left_LED, HIGH);
          left_trial_state =1;
          left_trial_time_on = current_time;
          digitalWrite(trialTrigger_left, HIGH);
          
          
        }
        if (trial_array[trialCounter] == 1){
          if(laser_trial[trialCounter] == 1){
            digitalWrite(laser_on_trigger, HIGH);
          }
          delay(sound_delay);
          tone (sound_pin, 15000);
          // tone (sound_pin, 3000);
          // digitalWrite(right_LED, HIGH);
          right_trial_state =1;
          right_trial_time_on = current_time;
          digitalWrite(trialTrigger_right, HIGH);
          
          
        }
        previous_time = current_time;
      }
   }


}
void left_lick_detection(){
  if (lickCounter<lickNum){
      if (left_lick_state == 0 && lickSensorState_left == 0){
        left_lick_off_time = millis();
      }
      if (left_lick_state == 0 && lickSensorState_left == 1){
        left_lick_onset_time = left_lick_off_time;
        left_lick_on_time = millis();
        if (left_lick_on_time-left_lick_off_time >=minLickDuration){
          left_lick_state = 1;
          // digitalWrite(lickTrigger_left, HIGH);
        }
     }
     if (left_lick_state == 1 && lickSensorState_left ==1){
        left_lick_on_time = millis();

     }
     if (left_lick_state == 1 && lickSensorState_left ==0) {
        left_lick_offset_time = left_lick_on_time;
        left_lick_duration = left_lick_offset_time - left_lick_onset_time;
        if (left_lick_duration >= minLickDuration){
          lickCounter++;
          // digitalWrite(lickTrigger_left, LOW);
        }
        left_lick_state = 0;
     }
  }
  if (lickCounter==lickNum){
    trialCounter++;
    lickCounter = 0;
    left_trial_state = 0;
    digitalWrite(left_LED, LOW);
    noTone (sound_pin);
    digitalWrite(laser_on_trigger, LOW);
    digitalWrite(trialTrigger_left, LOW);
    left_milkDelivery();
    Serial.print("milk delivered, current left trial is: ");
    Serial.println(trialCounter);
    
  }

}
void right_lick_detection(){
  if (lickCounter<lickNum){
      if (right_lick_state == 0 && lickSensorState_right == 0){
        right_lick_off_time = millis();
      }
      if (right_lick_state == 0 && lickSensorState_right == 1){
        right_lick_onset_time = right_lick_off_time;
        right_lick_on_time = millis();
        if (right_lick_on_time-right_lick_off_time >=minLickDuration){
          right_lick_state = 1;
          // digitalWrite(lickTrigger_right, HIGH);
        }
     }
     if (right_lick_state == 1 && lickSensorState_right ==1){
        right_lick_on_time = millis();

     }
     if (right_lick_state == 1 && lickSensorState_right ==0) {
        right_lick_offset_time = right_lick_on_time;
        right_lick_duration = right_lick_offset_time - right_lick_onset_time;
        if (right_lick_duration >= minLickDuration){
          lickCounter++;
          // digitalWrite(lickTrigger_right, LOW);
        }
        right_lick_state = 0;
     }
  }
  if (lickCounter==lickNum){
    trialCounter++;
    lickCounter = 0;
    right_trial_state = 0;
    digitalWrite(right_LED, LOW);
    noTone (sound_pin);
    digitalWrite(laser_on_trigger, LOW);
    digitalWrite(trialTrigger_right, LOW);
    right_milkDelivery();
    Serial.print("milk delivered, current right trial is: ");
    Serial.println(trialCounter);
    
  }
}

void left_milkDelivery(){
  
  digitalWrite (pumpTrigger_left, HIGH);
  delay(100);
  digitalWrite(pumpTrigger_left, LOW);
  // delay(1000);
  // digitalWrite (pumpTrigger_left, HIGH);
  // delay(100);
  // digitalWrite(pumpTrigger_left, LOW);

}
void right_milkDelivery(){
  
  digitalWrite (pumpTrigger_right, HIGH);
  delay(100);
  digitalWrite(pumpTrigger_right, LOW);
  // delay(1000);
  // digitalWrite (pumpTrigger_right, HIGH);
  // delay(100);
  // digitalWrite(pumpTrigger_right, LOW);
}