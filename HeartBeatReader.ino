#include <CapacitiveSensor.h>

const bool SERIAL_DEBUG=false;
const int TOUCH_THRES=500;

enum MODE{WAIT, TOUCH, SUCCESS};
MODE _mode;
int _timer_index;

//BPM
const int LOWBPM=50;
const int HIGHBPM=100;

const int RPin=5;
const int GPin=6;
const int BPin=9;
 
const int UpdateInterval=20;
//Sleep
const int SleepInterval=2000/UpdateInterval;
//Touch
const int TouchInterval=1000/UpdateInterval;
const int TouchAmp=100;
const int TouchCount=4;

const int FakeRate=72;

//Record
const int RecordLength=5;


//  Variables
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

int _record[RecordLength];
int _tmp_record[RecordLength];

int _touch_index;
int hueRate=0;

int _record_index;

int _play_index;
int _sleep_index;

bool _touched;


//touch
CapacitiveSensor   cs_4_2 = CapacitiveSensor(4,2);  

int Bpm2Time(int bpm){    
  int time_=(int)(60000.0/(float)bpm/(float)UpdateInterval); 
  return time_;
}

void setup(){

 // cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);  
     
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  pinMode(RPin,OUTPUT);          // pin that will fade to your heartbeat!
  pinMode(GPin,OUTPUT);          // pin that will fade to your heartbeat!
  pinMode(BPin,OUTPUT);          // pin that will fade to your heartbeat!
  
  Serial.begin(9600);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 

  for(int i=0;i<RecordLength;++i){
    _record[i]=-1;
    _tmp_record[i]=-1;
  }
  
  startMode(MODE::WAIT);  
  _touched=false;
}


//  Where the Magic Happens
void loop(){

  checkBeat();
  checkTouch();
  
  bool timeup=(_timer_index<=0);
  switch(_mode){
    case WAIT:
        /* play record */
        if(timeup){
          _play_index=(_play_index+1)%RecordLength;
          
          _timer_index=_record[_play_index];
          fadeRate=255;          
        }
      break;
    case TOUCH:
        if(timeup){
          fadeRate=TouchAmp;
          _timer_index=TouchInterval;
          _touch_index++;

          if(SERIAL_DEBUG){
            Serial.print("Touch ");
            Serial.println(_touch_index);
          }
          
          if(_touch_index>=TouchCount) startMode(MODE::WAIT);  
        
        }
        
      break;   
    
  }
  
  if(_timer_index>0) _timer_index--;
  if(_sleep_index>0) _sleep_index--;

  
  handleLed();
  delay(UpdateInterval);
}
void checkTouch(){

  if(_sleep_index>0) return;
  
  int touch=cs_4_2.capacitiveSensor(5);
//  Serial.println(touch);

  if(touch>TOUCH_THRES){
      if(_mode==MODE::WAIT && !_touched) startMode(MODE::TOUCH); 
      _touched=true; 
  }else{
    _touched=false;
    if(_mode!=MODE::WAIT) startMode(MODE::WAIT);
  }
}
void checkBeat(){
  if(QS==true){
      if(_mode==MODE::TOUCH){
        if(validBPM(BPM)){
          
          if(SERIAL_DEBUG){
            Serial.print("Valid beat ");
            Serial.println(_record_index);
          }
          _tmp_record[_record_index]=Bpm2Time(BPM);
          _record_index=(_record_index+1)%RecordLength;
        }
      }
      if(SERIAL_DEBUG) serialOutputWhenBeatHappens();
      QS = false;  
  }
     
}
bool validBPM(int bpm_){
  return (bpm_>=LOWBPM && bpm_<=HIGHBPM);
}
void checkRecord(){

  if(SERIAL_DEBUG) Serial.print("Record: ");
  
  int sample_=(_tmp_record[0]<0)?Bpm2Time(FakeRate):_tmp_record[0];  
  if(SERIAL_DEBUG) Serial.print(sample_);
  
  for(int i=0;i<RecordLength;++i){
    if(_tmp_record[i]<0) _record[i]=sample_+random(-2,2);
    else _record[i]=_tmp_record[i];

    if(SERIAL_DEBUG){
      Serial.print(" ");
      Serial.print(_record[i]);
    }
  }
 
  
  if(SERIAL_DEBUG) Serial.println();
}

void startMode(MODE mode_){
  if(SERIAL_DEBUG){
    printStart(mode_);
  }
  _mode=mode_;
  switch(_mode){
      case WAIT:          
          
          _sleep_index=SleepInterval;
          _timer_index=_record[0];
          _play_index=0;
          fadeRate=255;          
          checkRecord();
          break;
      case TOUCH:
          _timer_index=TouchInterval;
          _touch_index=0;
          hueRate=0;
          
          // reset record
          _record_index=0;
          for(int i=0;i<RecordLength;++i) _tmp_record[i]=-1;
          
          break;
  }
  resetLed();
}
void resetLed(){
  analogWrite(RPin,255-0);
  analogWrite(GPin,255-0);
  analogWrite(BPin,255-0);
       
}




void handleLed(){
   
 
    switch(_mode){
      case WAIT:
          analogWrite(RPin,255-fadeRate); 
          fadeRate-=15;
          
          break;
      case TOUCH:
         
          //Serial.println(fadeRate);
          int rgb[3];
          
          HSV2RGB(hueRate,255,TouchAmp-fadeRate,rgb);
          
          analogWrite(RPin,255-rgb[0]);
          analogWrite(GPin,255-rgb[1]);
          analogWrite(BPin,255-rgb[2]);          
          
          fadeRate-=5;          
          hueRate=(hueRate+5)%255;
          break;         
    }
    fadeRate=constrain(fadeRate,0,255);
    
}





