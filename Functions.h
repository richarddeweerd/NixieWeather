void InitIO(){
  pinMode(Receiver_Pin, INPUT);
}

void InitEEprom(){
  EEPROM.write(EEpromDatePerMin,      3);
  EEPROM.write(EEpromDateLength,      2);
  EEPROM.write(EEpromWeatherLength,   2);
  EEPROM.write(EEpromScrLength,       20);
  EEPROM.write(EEpromScrPerHour,      6);
  EEPROM.write(EEpromBackLightDay,    250);
  EEPROM.write(EEpromBackLightNight,  50);
  EEPROM.write(EEpromDayStartHour,    7);
  EEPROM.write(EEpromDayStartMin,     30);
  EEPROM.write(EEpromDayEndHour,      0);
  EEPROM.write(EEpromDayEndMin,       30);
  EEPROM.write(EEpromWakeup,          20);
  EEPROM.write(EEpromTimeZone,        25);
}

void ReadConfig(){
  Date_Interval = EEPROM.read(EEpromDatePerMin);
  Date_Interval = (int) 60 / Date_Interval;
  Date_Lenght = EEPROM.read(EEpromDateLength);
  SecsPerStep = EEPROM.read(EEpromWeatherLength);
  ScrSaver_Duration = EEPROM.read(EEpromScrLength);
  ScrSaver_Interval = EEPROM.read(EEpromScrPerHour);
  Nixie.BackLightDay = EEPROM.read(EEpromBackLightDay);
  Nixie.BackLightNight = EEPROM.read(EEpromBackLightNight);
  //EEPROM.read(EEpromDayStartHour);
  //EEPROM.read(EEpromDayStartMin);
  //EEPROM.read(EEpromDayEndHour);
  //EEPROM.read(EEpromDayEndMin);
  WakeUp_time = EEPROM.read(EEpromWakeup);
  int t = EEPROM.read(EEpromTimeZone);  
  TZ_offset = (t-24) * 3600;
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
  byte result = val;
  result++;
  if (val >= maximum){
    result = minimum;
  }
  return result;
}

byte decMax(byte val, byte minimum, byte maximum){
  byte result = val;
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
