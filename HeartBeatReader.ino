#include <CapacitiveSensor.h>

const bool SERIAL_DEBUG=true;
const int TOUCH_THRES=300;

enum MODE{WAIT, TOUCH, REAL, FAKE, SUCCESS};
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
const int TouchInterval=5000/UpdateInterval;
const int TouchFadeInterval=TouchInterval/4;
const int TouchAmp=100;

//Success
const int SuccessInterval=1000/UpdateInterval;
//Fake
const int FakeRate=60000/60/UpdateInterval;
//Record
const int BeatThres=2;
const int BeatLength=3;
const int BeatDropInterval=60000/LOWBPM/UpdateInterval;
const int RecordLength=60000/LOWBPM*BeatLength/UpdateInterval;


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

int _record_beat;

int _record[RecordLength];
int _tmp_record[RecordLength];
int _record_index;
int _play_index;
int _stop_index;

//fake
int _fake_rate;


bool _touched;


//touch
CapacitiveSensor   cs_4_2 = CapacitiveSensor(4,2);  

void setup(){

 // cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);  
     
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  pinMode(RPin,OUTPUT);          // pin that will fade to your heartbeat!
  pinMode(GPin,OUTPUT);          // pin that will fade to your heartbeat!
  pinMode(BPin,OUTPUT);          // pin that will fade to your heartbeat!
  
  Serial.begin(9600);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 

  //setup default record
  int br=60000/60/UpdateInterval;
  int beat=0;
  for(int i=0;i<RecordLength;++i){
     beat-=15;
     _record[i]=constrain(beat,0,255);
     if(i%br==0) beat=255;     
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
//      if(timeup) startMode(MODE::TOUCH);
      break;
    case TOUCH:
      if(!timeup){
        if(_record_beat>BeatThres) startMode(MODE::REAL);
        if(_timer_index%TouchFadeInterval==1) fadeRate=TouchAmp;
      }else{
        startMode(MODE::FAKE);
      }
      break;
    case REAL: 
      if(!timeup){
        if(_record_beat>BeatLength) startMode(MODE::SUCCESS);
      }else{
        startMode(MODE::FAKE);
      }
      break;
    case FAKE:
      if(_record_beat<BeatLength){
        if(timeup){
          Serial.print("FAKE RECORD ");
          Serial.println(_record_beat);
           _fake_rate=random(FakeRate-5,FakeRate+5);
          _timer_index=_fake_rate;
          _record_beat++;
          fadeRate=255;
        }
      }else{
        startMode(MODE::SUCCESS);
      }
      break;
    case SUCCESS:
      if(timeup) startMode(MODE::WAIT);
      break;
  }
  
  if(_timer_index>0) _timer_index--;
  
  handleLed();
  delay(UpdateInterval);
}
void checkTouch(){
  int touch=cs_4_2.capacitiveSensor(30);
  //Serial.println(touch);
  if(touch>TOUCH_THRES){
      if(_mode==MODE::WAIT && _timer_index<=0 && !_touched) startMode(MODE::TOUCH); 
      _touched=true; 
  }else{
    _touched=false;
    if(_mode!=MODE::SUCCESS && _mode!=MODE::WAIT) startMode(MODE::WAIT);
  }
}
void checkBeat(){
  if(QS==true){
      if(_mode==MODE::REAL || _mode==MODE::TOUCH){
        if(validBPM(BPM)){
          _record_beat++;       
          if(_mode==MODE::REAL){
            if(SERIAL_DEBUG){
              Serial.print("BEAT ");
              Serial.println(_record_beat);
            }
            _timer_index=BeatDropInterval;
            fadeRate=255;
          } 
        }else{
          _record_beat--;    
        }
      }
      if(SERIAL_DEBUG) serialOutputWhenBeatHappens();
      QS = false;  
  }
     
}
bool validBPM(int bpm_){
  return (bpm_>=LOWBPM && bpm_<=HIGHBPM);
}

void startMode(MODE mode_){
  if(SERIAL_DEBUG){
    printStart(mode_);
  }
  _mode=mode_;
  switch(_mode){
      case WAIT:
          _timer_index=SleepInterval;
          break;
      case TOUCH:
          _timer_index=TouchInterval;
          _record_beat=0;
          break;      
      case FAKE:
          _fake_rate=random(FakeRate-5,FakeRate+5);
          _timer_index=_fake_rate;
          fadeRate=255;
      case REAL:
          _record_index=0;
          _record_beat=0;
          break;
      case SUCCESS:
          //copy record data
          for(int i=0;i<RecordLength;++i){
            if(i<_record_index) _record[i]=_tmp_record[i];                  
            else _record[i]=0;
          }
          _stop_index=_record_index;
          _play_index=0;

          _timer_index=SuccessInterval;
//          startMode(MODE::WAIT);
          
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
   
 
    int inv=TouchAmp-fadeRate;
    inv=constrain(inv,0,255);
    
    switch(_mode){
      case WAIT:
          analogWrite(RPin,255-_record[_play_index]); 
          _play_index=(_play_index+1)%_stop_index;
          break;
      case TOUCH:
         
          //Serial.println(fadeRate);
//          int rgb[3];
//          HSV2RGB(inv,255,inv,rgb);
//          analogWrite(RPin,255-rgb[0]);
//          analogWrite(GPin,255-rgb[1]);
//          analogWrite(BPin,255-rgb[2]);
          analogWrite(RPin,255-inv);
          analogWrite(GPin,255-inv);
          analogWrite(BPin,255-inv);
          
          fadeRate-=5;
          fadeRate=constrain(fadeRate,0,255);
          
          break;  
       case REAL:
       case FAKE:
          analogWrite(BPin,255-fadeRate);

          _tmp_record[_record_index]=fadeRate;
          _record_index=(_record_index+1)%RecordLength;

          fadeRate-=15;
          fadeRate=constrain(fadeRate,0,255);
          //Serial.println(fadeRate);
          break;
       case SUCCESS:
          analogWrite(GPin,255-255);
          break;
    }
    
}




