void InitIO(){
  pinMode(Receiver_Pin, INPUT);
}

void ReadTimeZone(){
  int t = EEPROM.read(EEpromTimeZone);  
  TZ_offset = (t-24) * 3600; 
}

void SetTimeZone(int _to){    
    _to = _to + 24;
    EEPROM.write(EEpromTimeZone,(byte)_to);
    ReadTimeZone();
}

void ReadDateInterval(){
  Date_Interval = EEPROM.read(EEpromDatePerMin);
  Date_Interval = (int) 60 / Date_Interval;  
}

void SetDateInterval(int _di){    
    EEPROM.write(EEpromDatePerMin,(byte)_di);
    ReadDateInterval();
}

void SetDateLength(int _dl){
    EEPROM.write(EEpromDateLength,(byte)_dl);
    Date_Lenght = _dl;
}

void SetWeatherLength(int _wl){
    EEPROM.write(EEpromWeatherLength,(byte)_wl);
    WeatherLength = _wl;
}

void SetScreenSaverInterval(int _si){
    EEPROM.write(EEpromScrPerHour,(byte)_si);
    ScrSaver_Interval = _si;
}

void SetScreenSaverLength(int _sl){
    EEPROM.write(EEpromScrLength,(byte)_sl);
    ScrSaver_Duration = _sl + 2;
}

void SetBacklightDay(int _bl){
    EEPROM.write(EEpromBackLightDay,(byte)_bl);
    Nixie.BackLightDay = _bl;
}

void SetBacklightNight(int _bl){
    EEPROM.write(EEpromBackLightNight,(byte)_bl);
    Nixie.BackLightNight = _bl;
}


void InitEEprom(){
  EEPROM.write(EEpromTimeZone,        25);
  EEPROM.write(EEpromDatePerMin,      3);
  EEPROM.write(EEpromDateLength,      2);
  EEPROM.write(EEpromWeatherLength,   2);
  EEPROM.write(EEpromScrLength,       18);
  EEPROM.write(EEpromScrPerHour,      60);
  EEPROM.write(EEpromBackLightDay,    250);
  EEPROM.write(EEpromBackLightNight,  50);
  EEPROM.write(EEpromDayStartHour,    7);
  EEPROM.write(EEpromDayStartMin,     30);
  EEPROM.write(EEpromDayEndHour,      0);
  EEPROM.write(EEpromDayEndMin,       30);
  EEPROM.write(EEpromWakeup,          20);
}


void ReadConfig(){
  ReadDateInterval();
  Date_Lenght = EEPROM.read(EEpromDateLength);
  WeatherLength = EEPROM.read(EEpromWeatherLength);
  ScrSaver_Duration = EEPROM.read(EEpromScrLength) + 2;
  ScrSaver_Interval = EEPROM.read(EEpromScrPerHour);
  Nixie.BackLightDay = EEPROM.read(EEpromBackLightDay);
  Nixie.BackLightNight = EEPROM.read(EEpromBackLightNight);
  //EEPROM.read(EEpromDayStartHour);
  //EEPROM.read(EEpromDayStartMin);
  //EEPROM.read(EEpromDayEndHour);
  //EEPROM.read(EEpromDayEndMin);
  WakeUp_time = EEPROM.read(EEpromWakeup);
  ReadTimeZone();
}

void SaveTime(byte _val, byte _pos){
  temp_tm.Hour = Nixie.Time.Hour;
  temp_tm.Minute = Nixie.Time.Minute;
  temp_tm.Second = Nixie.Time.Second;
  temp_tm.Day = Nixie.Time.Day;
  temp_tm.Month = Nixie.Time.Month;
  temp_tm.Year = Nixie.Time.Year;
  switch (_pos){
    case 1:
      temp_tm.Hour = _val;
      break;
    case 2:
      temp_tm.Minute = _val;
      break;
    case 3:
      temp_tm.Second = _val;
      break;
    case 4:
      temp_tm.Day = _val;
      break;
    case 5:
      temp_tm.Month = _val;
      break;
    case 6:
      temp_tm.Year = _val;
      break;
  }
  RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
  setSyncProvider(RTC.get);  
}



void CheckDST(unsigned long cur_time){
  DST_offset=0;

  int i;
  time_t DST_start, DST_end;
  byte dst_on = 0;
  TimeElements te;
  
  for (i = 0; i <= 15; i = i + 1) {
    if ((DST_dates[i][0]+1970) == (year())){
   
      te.Second = 0;
      te.Minute = 0;
      te.Hour = 1;
      te.Day = DST_dates[i][2];
      te.Month = DST_dates[i][1];
      te.Year = year()-1970;
      DST_start = makeTime(te);
      te.Day = DST_dates[i][4];
      te.Month = DST_dates[i][3];
      DST_end = makeTime(te);
      if ((cur_time>=DST_start) && (cur_time < DST_end)){
        dst_on = 1;
      }
    }
  }
  if (dst_on == 1){
    DST_offset = _DST_offset;
  } else {
    DST_offset = 0;
  }
}

byte incMax(byte val, byte minimum, byte maximum){
  //increse a byte by 1. 
  //if the maximum is passed then the value will be the minimum
  byte result = val;
  result++;
  if (val >= maximum){
    result = minimum;
  }
  return result;
}
byte incMax(byte val, byte minimum, byte maximum, byte stp){
  //increse a byte by stp. 
  //if the maximum is passed then the value will be the minimum
  byte result = val;
  result = result + stp;
  if (val >= maximum){
    result = minimum;
  }
  return result;
}

int incMaxint(int val, int minimum, int maximum){
  //increse an int by 1. 
  //if the maximum is passed then the value will be the minimum
  int result = val;
  result++;
  if (val >= maximum){
    result = minimum;
  }
  return result;
}

byte decMax(byte val, byte minimum, byte maximum){
  //decrese a byte by 1. 
  //if the minimum is passed then the value will be the maximum
  byte result = val;
  result--;
  if (val == minimum){
    result = maximum;
  }
  return result;
}
byte decMax(byte val, byte minimum, byte maximum, byte stp){
  //decrese a byte by stp. 
  //if the minimum is passed then the value will be the maximum
  byte result = val;
  result = result - stp;
  if (val == minimum){
    result = maximum;
  }
  return result;
}

int decMaxint(int val, int minimum, int maximum){
  //decrese an int by 1. 
  //if the minimum is passed then the value will be the maximum
  int result = val;
  result--;
  if (val == minimum){
    result = maximum;
  }
  return result;
}

void ResyncWeatherInfo(){
  SecCounter = 0;
  Sec_Step=0;
  StepCounter = 1;
}

void NightMode_on(byte h, byte m){
  int t = (h * 60) + m;
  byte stat = 0;
  if (Off_time < On_time ){
    if ((t < Off_time) || (t >= On_time) ){

    } else {
      stat = 1;
    }
  } else {
    if (Off_time > On_time ){
      if ((t < Off_time) && (t >= On_time) ){
        stat = 1;
      }
    } else {
      //on
      stat = 1;
    }
  }
  if ((Nixie.NightMode == 1) && (stat == 0)){
    Nixie.NightmodeEnd();
    Run_Mode = 0;
  }

  if ((Nixie.NightMode == 0) && (stat == 1)){
    Nixie.NightmodeStart();
    Run_Mode = 3;
  } 
}
