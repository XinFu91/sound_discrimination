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
int lick_time_window = 10*1000; // mice need to lick inside this time window to activate the pump
int lickNum = 1; // number of licks needed to activate the pump
int lickCounter = 0; 
int minLickDuration = 1; // minimum duration to be considered as a lick
int minPokeDuration = 100; // minimum duration for a successful poke

const int refresherNum = 0; // number of refresher trials before real experiments 
const int blockNumber = 15;
const int trialInBlock = 20; // number of trial in a block
const int extraTraining = 10;
const int trialNumber=refresherNum+blockNumber*trialInBlock+extraTraining;

int trial_array[trialNumber];
//int interval_array[trialNumber];
int refresher_array[refresherNum];
// int block_array[trialInBlock]={1,0,1,1,0,1,0,0,1,0,1,1,1,0,0,1,1,0,0,0}; // 0 indicates left lick trials, 1 indicates right lick trials.
int block_array[trialInBlock]={0,0,0,0,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1};
// int block_array[trialInBlock]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
// int block_array[trialInBlock]={0,0,1,1,1,1,1,1,0,0};
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
        //interval_array[refresherNum+i*trialInBlock+j]=interval_block_array[j];
      }   
    }
    for (i=0;i<refresherNum;i++){
      trial_array[i]=1;
      //interval_array[i]=5;
    }
    for (i=0;i<extraTraining;i++){
      trial_array[refresherNum+blockNumber*trialInBlock+i]=0;
     // interval_array[refresherNum+blockNumber*trialInBlock+i]=20;
    }
    for (i=0;i<trialNumber;i++){
      Serial.print(i);
      Serial.print(" trial is : ");
      Serial.println(trial_array[i]);
      // Serial.print(" with interval: ");
      // Serial.println(interval_array[i]);
    }
}

void loop() {
  if(trialCounter<trialNumber){
    current_time = millis();
    pokeSensorState = digitalRead(pokeSensor_pin);
    nose_poke_detection();// detect a successful nose poke and play a sound, set a time window for lick detection and milk delivery 

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
          tone (sound_pin, 3000,1500);
          digitalWrite(left_LED, HIGH);
          // left_trial_state =1;
          // left_trial_time_on = current_time;
          
          //delay(500);
          left_milkDelivery();
          previous_time = current_time;
        }
        if (trial_array[trialCounter] == 1){
          tone (sound_pin, 15000,1500);
          // right_trial_state =1;
          // right_trial_time_on = current_time;
         digitalWrite(right_LED, HIGH);
         //delay(500);
         right_milkDelivery();
         previous_time = current_time;
        }
        trialCounter++;
        Serial.print("current trial is: ");
        Serial.println(trialCounter);
      }
   }
}


void left_milkDelivery(){
  
  digitalWrite (pumpTrigger_left, HIGH);
  delay(100);
  digitalWrite(pumpTrigger_left, LOW);
  delay(1500);
  digitalWrite(left_LED, LOW);
}
void right_milkDelivery(){
  
  digitalWrite (pumpTrigger_right, HIGH);
  delay(100);
  digitalWrite(pumpTrigger_right, LOW);
  delay(1500);
  digitalWrite(right_LED, LOW);
}