#include <avr/pgmspace.h>
#include "Nixie.h"
#include "PinDef.h"
#include "Wire.h"
#include "Time.h"
#include "DS3231RTC.h"
#include "Button.h"

#include "Nixie_Sensor.h"

#include "Globals.h"
#include "Config.h"
#include "rf.h"
#include "bmp085.h"
#include "Functions.h"


int i,temp;

void setup() {
  InitIO();      // Initialize IO pins
  Nixie.Leds_On(); // Turn all LED's on for test
  Wire.begin();  // Starting I2C bus
  init_rf();     // Setup interupts for RF receiver
  
  digitalWrite(HV_Pin, HIGH);                   // Turn HV on
  Nixie.DimIntensity = BackLight_Bright;
  Nixie.Disp_Test(); 
  
  setSyncProvider(RTC.get); //Init RTC 
  
  //init all time registers:
  CheckDST(now());
  breakTime ((now()+TZ_offset+DST_offset),tm);
  
  CurrentMin=tm.Minute;
  CurrentHour=tm.Hour;
  CurrentDay=tm.Day;
  CurrentMonth=tm.Month;
  CurrentYear=tm.Year;

  bmp085Calibration(); //Init barometer
  Baro.SetTemp(bmp085GetTemperature(bmp085ReadUT()));
  Baro.SetPressure(bmp085GetPressure(bmp085ReadUP()),(CurrentTime + TZ_offset + DST_offset));
  

  Nixie.Leds_Off(); // Turn off all LED's to show that init phase is done.
  //NightMode_on(tm.Hour, tm.Minute);
}

void loop() {
  But_Front.check();
  But_Up.check();
  But_Down.check();

  if (now() != CurrentTime){
    //New Seccond
    Sec_Start=millis();
    Sec_Step=0;
    SecCounter++;       
    
    CurrentTime = now();
    breakTime ((CurrentTime + TZ_offset + DST_offset),tm);

    if (((CurrentMin % ScrSaver_Interval) == 0)&&((60 - ScrSaver_Duration) == tm.Second )){
       Nixie.ScreenSaverStart(ScrSaver_Duration,tm.Hour,tm.Minute, tm.Second, Baro.Pressure);
    }

    if (SecCounter == SecsPerStep){
      SecCounter = 0;
      StepCounter++;
      if (StepCounter > Steps)
      {
        StepCounter = 1;
      }
    }
 
    if (Nixie.ScreenSaverActive >= 1){
      //Screensaver active
      Nixie.ScreenSaverSecPulse(tm.Hour,tm.Minute, tm.Second, Baro.Pressure);
    }    
    
    if (Nixie.ScreenSaverFinished == 1){
      ResyncWeatherInfo();
      Nixie.ScreenSaverFinished = 0;
      Nixie.ScreenSaverActive = 0;
    }

    if (tm.Second % 10 == 6){
      Baro.SetTemp(bmp085GetTemperature(bmp085ReadUT()));
      Baro.SetPressure(bmp085GetPressure(bmp085ReadUP()),(CurrentTime + TZ_offset + DST_offset));
    }
    if (tm.Minute != CurrentMin){
      //New Minute!
      CurrentMin=tm.Minute;
      NightMode_on(tm.Hour, tm.Minute);
      if (tm.Hour != CurrentHour){
        //New Hour!
        CurrentHour=tm.Hour;
        //check if DST starts
        CheckDST(now());
        if (tm.Day != CurrentDay){
          //New Day!
          CurrentDay=tm.Day;
          if (tm.Month != CurrentMonth){
            //New Month!
            CurrentMonth=tm.Month;
            if (tm.Year != CurrentYear){
              //New Year!
              CurrentYear=tm.Year;
            }
          }
        }
      }
    }        
  }

  if (Run_Mode < 99 ){
    if (But_Up.hold(3000)){
      But_Up.block();
      
      Run_Mode = 99;
      Setup_Item = 0;
      Setup_Subitem = 1;
      SetupStart = true;
    }    
  }

  if ((Run_Mode == 0 )&&(Nixie.ScreenSaverActive >= 1)){
    Run_Mode = 2;
  }
   

  switch (Run_Mode){
    case 0:
      //Normal Operation
      //Clock part:
      if ((tm.Second % 20 == 18)||(tm.Second % 20 == 19)){
          Nixie.ShowDate(tm.Day, tm.Month, tm.Year);
      } else {
          Nixie.ShowClock(tm.Hour, tm.Minute, tm.Second);
      }
      //Weather info
      if (StepCounter == 1){
        //Show Pressure
        Nixie.ShowPressure(Baro.Pressure, Baro.MinMaxLed);
      } else {
        //Temp or humidity
        if (StepCounter % 2 == 0){
          //Temp 
          Nixie.ShowTemp(StepCounter/ 2, ((int) (RFSensor[StepCounter/2].Temp * 10)),RFSensor[StepCounter/2].TempMinMaxLed);
        } else {
          //Hum
          Nixie.ShowHum(StepCounter/ 2, RFSensor[StepCounter/2].Hum,RFSensor[StepCounter/2].HumMinMaxLed);
        }
      }      
      break;

    case 1:
      //Min - Max 
      break;
    
    case 2:
      //ScreenSaver
      Nixie.ScreenSaverPulse();
      if (Nixie.ScreenSaverActive == 0 ){
        Run_Mode = 0;
      }
      break;

    case 3:
      //sleep
      if (But_Front.status()){
        //wakeup
      }
      break;

    case 99:
      //setup
      if (But_Front.hold(2000)){
        //save
        Run_Mode = 0;
      }
      byte changed = 0;

      switch (Setup_Item){
        case 0:     
          //set time
          if (SetupStart){
            SetupVal = tm.Hour;
            SetupStart = false;
          }
          if (But_Front.status(3000)){            
            switch (Setup_Subitem){
              case 1:
                Setup_Subitem++;
                temp_tm.Hour = SetupVal;
                temp_tm.Minute = tm.Minute;
                temp_tm.Second = tm.Second;
                temp_tm.Day = tm.Day;
                temp_tm.Month = tm.Month;
                temp_tm.Year = tm.Year;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                setSyncProvider(RTC.get);
                SetupVal = tm.Minute;
                break;
              case 2:
                temp_tm.Hour = tm.Hour;
                temp_tm.Minute = SetupVal;
                temp_tm.Second = tm.Second;
                temp_tm.Day = tm.Day;
                temp_tm.Month = tm.Month;
                temp_tm.Year = tm.Year;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                setSyncProvider(RTC.get);
                
                Setup_Subitem++;
                break;
              case 3:
                Setup_Item = 1;
                Setup_Subitem = 1;
                SetupStart = true;
                SetupVal = tm.Day;

                break;
            }
          }

          switch (Setup_Subitem){
            case 1:
              //Hour
              if (But_Up.status(1000,250)){
                changed = 1;
                SetupVal = incMax(SetupVal, 0, 23);
                //SetupVal = 11;
              }
              if (But_Down.status(1000,250)){
                changed = 1;            
                SetupVal = decMax(SetupVal, 0, 23);
              }
              Nixie.SetupClock(SetupVal, tm.Minute, tm.Second, Setup_Subitem, changed);
              break;
            case 2:
              //Minute
              if (But_Up.status(1000,250)){
                changed = 1;
                SetupVal = incMax(SetupVal, 0, 59);
                //SetupVal = 11;
              }
              if (But_Down.status(1000,250)){
                changed = 1;            
                SetupVal = decMax(SetupVal, 0, 59);
              }
              Nixie.SetupClock(tm.Hour, SetupVal, tm.Second, Setup_Subitem, changed);
              break;
            case 3:
              //Sec
              if (But_Up.status(1000,250)||But_Down.status(1000,400)){
                temp_tm.Hour = tm.Hour;
                temp_tm.Minute = tm.Minute;
                temp_tm.Second = 0;
                temp_tm.Day = tm.Day;
                temp_tm.Month = tm.Month;
                temp_tm.Year = tm.Year;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                setSyncProvider(RTC.get);
              }
              Nixie.SetupClock(tm.Hour, tm.Minute, tm.Second, Setup_Subitem, changed);                
              break;
          }
          break;
  
        case 1:
          //set date

          if (But_Front.status(3000)){            
            switch (Setup_Subitem){
              case 1:
                Setup_Subitem++;
                temp_tm.Hour = tm.Hour;
                temp_tm.Minute = tm.Minute;
                temp_tm.Second = tm.Second;
                temp_tm.Day = SetupVal;
                temp_tm.Month = tm.Month;
                temp_tm.Year = tm.Year;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                setSyncProvider(RTC.get);
                SetupVal = tm.Month;
                break;
              case 2:
                temp_tm.Hour = tm.Hour;
                temp_tm.Minute = tm.Minute;
                temp_tm.Second = tm.Second;
                temp_tm.Day = tm.Day;
                temp_tm.Month = SetupVal;
                temp_tm.Year = tm.Year;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                
                setSyncProvider(RTC.get);      
                SetupVal = tm.Year;                
                Setup_Subitem++;
                break;
              case 3:
                temp_tm.Hour = tm.Hour;
                temp_tm.Minute = tm.Minute;
                temp_tm.Second = tm.Second;
                temp_tm.Day = tm.Day;
                temp_tm.Month = tm.Month;
                temp_tm.Year = SetupVal;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                setSyncProvider(RTC.get);
                Setup_Item = 0;
                Setup_Subitem = 1;
                SetupStart = true;
                break;
            }
          }

          switch (Setup_Subitem){
            case 1:
              //Day
              if (But_Up.status(1000,250)){
                changed = 1;
                SetupVal = incMax(SetupVal, 1, 31);
                //SetupVal = 11;
              }
              if (But_Down.status(1000,250)){
                changed = 1;            
                SetupVal = decMax(SetupVal, 1, 31);
              }
              Nixie.SetupDate(SetupVal, tm.Month, tm.Year, Setup_Subitem, changed);
              break;
            case 2:
              //Month
              if (But_Up.status(1000,2500)){
                changed = 1;
                SetupVal = incMax(SetupVal, 1, 12);
                //SetupVal = 11;
              }
              if (But_Down.status(1000,250)){
                changed = 1;            
                SetupVal = decMax(SetupVal, 1, 12);
              } 
              Nixie.SetupDate(tm.Day, SetupVal, tm.Year, Setup_Subitem, changed);
              break;
            case 3:
              //Year
              if (But_Up.status(1000,250)){
                changed = 1;
                SetupVal = incMax(SetupVal, 0, 99);
                //SetupVal = 11;
              }
              if (But_Down.status(1000,250)){
                changed = 1;            
                SetupVal = decMax(SetupVal, 0, 99);
              }
              Nixie.SetupDate(tm.Day, tm.Month, SetupVal, Setup_Subitem, changed);                
              break;
          }
          break;
          
          break;
  
          
      }
   break;    
  }

  switch (Sec_Step){
    case 0:
      Nixie.SecPulse(Sec_Step);
      Sec_Step = 1;      
      break;
    case 1:
      if ((millis() - Sec_Start)>=250){
        Nixie.SecPulse(Sec_Step);
        Sec_Step = 2;
      }
      break;
    case 2:
      if ((millis() - Sec_Start)>=500){
        Nixie.SecPulse(Sec_Step);
        Sec_Step = 3;
      }
      break; 
    case 3:
      if ((millis() - Sec_Start)>=750){
        Nixie.SecPulse(Sec_Step);
        Sec_Step = 4;
      }
      break;
  } 
  
  if (PacketDone) {
    // Received a RF packet
    memcpy(&WorkPacket,&FinishedPacket,PACKET_SIZE);
    ParsePacket(WorkPacket);
    PacketDone=0;
    Steps = 1 + (SensorCount * 2);    
  }
}
