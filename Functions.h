void InitIO(){
  pinMode(Receiver_Pin, INPUT);
}

void CheckDST(unsigned long cur_time){
  DST_offset=0;

  int i;
  time_t DST_start, DST_end;
  byte dst_on = 0;
  
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
  }

  if ((Nixie.NightMode == 0) && (stat == 1)){
    Nixie.NightmodeStart();  } 
}
