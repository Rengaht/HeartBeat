
//  Decides How To OutPut BPM and IBI Data
void serialOutputWhenBeatHappens(){    
    Serial.print("BPM: ");
    Serial.println(BPM);  
//    Serial.print("   ");
//    Serial.println(fadeRate);
}

void printStart(MODE mode_){
  Serial.print("---------------Start ");
  switch(mode_){
    case WAIT:
      Serial.print("WAIT");
      break;
    case TOUCH:
      Serial.print("TOUCH");
      break;
    case REAL:
      Serial.print("REAL");
      break;
    case FAKE:
      Serial.print("FAKE");
      break;
    case SUCCESS:
      Serial.print("SUCCESS");
      break;
  }
  Serial.println("--------------------");
}

