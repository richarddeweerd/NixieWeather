#include <avr/pgmspace.h>
#include "Nixie.h"
#include "PinDef.h"
#include "Wire.h"
#include "TimeLib.h"
#include "DS3231RTC.h"
#include "Button.h"

#include "Nixie_Sensor.h"

#include "Globals.h"
#include "Config.h"
#include "rf.h"
#include "bmp085.h"
#include "Functions.h"


void setup() {
  InitIO();      // Initialize IO pins
  Nixie.Leds_On(); // Turn all LED's on for test
  Wire.begin();  // Starting I2C bus
  init_rf();     // Setup interupts for RF receiver

  setSyncProvider(RTC.get); //Init RTC 
  if (digitalRead(Button_Up_Pin) == LOW){
    delay(1000);
    if (digitalRead(Button_Up_Pin) == LOW){
      //reset date and time to 25-05-2016 00:00
      RTC.set(1464127200);
      setSyncProvider(RTC.get);
    }
  }
  
  
  digitalWrite(HV_Pin, HIGH);                   // Turn HV on
  Nixie.DimIntensity = BackLight_Bright;
  Nixie.Disp_Test(); 
  
  
  
  //init all time registers:
  CheckDST(now());
  breakTime ((now()+TZ_offset+DST_offset),Nixie.Time);
  
  CurrentMin=Nixie.Time.Minute;
  CurrentHour=Nixie.Time.Hour;
  CurrentDay=Nixie.Time.Day;
  CurrentMonth=Nixie.Time.Month;
  CurrentYear=Nixie.Time.Year;

  bmp085Calibration(); //Init barometer
  Nixie.Baro.SetTemp(bmp085GetTemperature(bmp085ReadUT()));
  Nixie.Baro.SetPressure(bmp085GetPressure(bmp085ReadUP()),(CurrentTime + TZ_offset + DST_offset));

  Nixie.Leds_Off(); // Turn off all LED's to show that init phase is done.
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

    Nixie.PulseCount=0;
    CurrentTime = now();
    breakTime ((CurrentTime + TZ_offset + DST_offset),Nixie.Time);

    if (SecCounter == SecsPerStep){
      SecCounter = 0;
      StepCounter++;
      if (StepCounter > Steps)
      {
        StepCounter = 1;
      }
    }

    if (WakeUp > 0){
      WakeUp--;
      if (WakeUp == 0){
        Nixie.NightmodeStart();
        Run_Mode = 3;
      }      
    }
       
    if (Nixie.Time.Second % 6 == 1){
      // read barometer every 10 seconds
      Nixie.Baro.SetTemp(bmp085GetTemperature(bmp085ReadUT()));
      Nixie.Baro.SetPressure(bmp085GetPressure(bmp085ReadUP()),(CurrentTime + TZ_offset + DST_offset));
    }
    
    if (Nixie.Time.Minute != CurrentMin){
      //New Minute!
      CurrentMin=Nixie.Time.Minute;
      // Check if nighmode is enabled
      if (WakeUp == 0){
        NightMode_on(Nixie.Time.Hour, Nixie.Time.Minute);
      }
      if (Nixie.Time.Hour != CurrentHour){
        //New Hour!
        CurrentHour=Nixie.Time.Hour;
        //check if DST starts
        CheckDST(now());
        if (Nixie.Time.Day != CurrentDay){
          //New Day!
          CurrentDay=Nixie.Time.Day;
          if (Nixie.Time.Month != CurrentMonth){
            //New Month!
            CurrentMonth=Nixie.Time.Month;
            if (Nixie.Time.Year != CurrentYear){
              //New Year!
              CurrentYear=Nixie.Time.Year;
            }
          }
        }
      }
    }        
  }

  long curmil = millis();
  if (curmil < Sec_Start){
    // millis rolover
    Sec_Start = curmil;
  }
  curmil = curmil - Sec_Start;
  
  if ((curmil >= (Nixie.PulseCount + 1)*PulseInterval)&&(curmil<1000)){
    Nixie.PulseCount++;
  }


  if ((Run_Mode == 0 )&&(Nixie.ScreenSaverActive >= 1)){
    Run_Mode = 2;
  }
   
  if (Run_Mode < 99 ){
    if (But_Up.hold(3000)){
      //Enter setup mode
      But_Up.block();
      
      Run_Mode = 99;
      Setup_Item = 0;
      Setup_Subitem = 1;
      SetupStart = true;
    }    
  }
  
  switch (Run_Mode){
    case 0:
      //Normal Operation
      
      //Clock part:
      if ((Nixie.Time.Second % 20 == 18)||(Nixie.Time.Second % 20 == 19)){
          Nixie.ShowDate(Nixie.Time.Day, Nixie.Time.Month, Nixie.Time.Year);
          
      } else {
          Nixie.ShowTime(Nixie.Time.Hour, Nixie.Time.Minute, Nixie.Time.Second);
      }
  
      //Weather info
      if (StepCounter == 1){
        //Show Pressure
        Nixie.ShowPressure(Nixie.Baro.Pressure, Nixie.Baro.MinMaxLed);
      } else {
        //Temp or humidity
        if (StepCounter % 2 == 0){
          //Temp 
          Nixie.ShowTemp(StepCounter/ 2, ((int) (Nixie.RFSensor[StepCounter/2].Temp * 10)),Nixie.RFSensor[StepCounter/2].TempMinMaxLed);
        } else {
          //Hum
          Nixie.ShowHum(StepCounter/ 2, Nixie.RFSensor[StepCounter/2].Hum,Nixie.RFSensor[StepCounter/2].HumMinMaxLed);
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
          Nixie.NightmodeEnd();
          WakeUp = WakeUp_time;
          Run_Mode = 0;
      }
      break;

    case 99:
      //setup
      if (But_Front.hold(2000)){
        //save / Exit setup mode
        Run_Mode = 0;
      }
      byte changed = 0;

      switch (Setup_Item){
        case 0:     
          //set time
          if (SetupStart){
            SetupVal = Nixie.Time.Hour;
            SetupStart = false;
          }
          if (But_Front.status(3000)){            
            switch (Setup_Subitem){
              case 1:
                Setup_Subitem++;
                temp_tm.Hour = SetupVal;
                temp_tm.Minute = Nixie.Time.Minute;
                temp_tm.Second = Nixie.Time.Second;
                temp_tm.Day = Nixie.Time.Day;
                temp_tm.Month = Nixie.Time.Month;
                temp_tm.Year = Nixie.Time.Year;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                setSyncProvider(RTC.get);
                SetupVal = Nixie.Time.Minute;
                break;
              case 2:
                temp_tm.Hour = Nixie.Time.Hour;
                temp_tm.Minute = SetupVal;
                temp_tm.Second = Nixie.Time.Second;
                temp_tm.Day = Nixie.Time.Day;
                temp_tm.Month = Nixie.Time.Month;
                temp_tm.Year = Nixie.Time.Year;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                setSyncProvider(RTC.get);
                
                Setup_Subitem++;
                break;
              case 3:
                Setup_Item = 1;
                Setup_Subitem = 1;
                SetupStart = true;
                SetupVal = Nixie.Time.Day;

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
              Nixie.SetupClock(SetupVal, Nixie.Time.Minute, Nixie.Time.Second, Setup_Subitem, changed);
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
              Nixie.SetupClock(Nixie.Time.Hour, SetupVal, Nixie.Time.Second, Setup_Subitem, changed);
              break;
            case 3:
              //Sec
              if (But_Up.status(1000,250)||But_Down.status(1000,400)){
                temp_tm.Hour = Nixie.Time.Hour;
                temp_tm.Minute = Nixie.Time.Minute;
                temp_tm.Second = 0;
                temp_tm.Day = Nixie.Time.Day;
                temp_tm.Month = Nixie.Time.Month;
                temp_tm.Year = Nixie.Time.Year;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                setSyncProvider(RTC.get);
              }
              Nixie.SetupClock(Nixie.Time.Hour, Nixie.Time.Minute, Nixie.Time.Second, Setup_Subitem, changed);                
              break;
          }
          break;
  
        case 1:
          //set date

          if (But_Front.status(3000)){            
            switch (Setup_Subitem){
              case 1:
                Setup_Subitem++;
                temp_tm.Hour = Nixie.Time.Hour;
                temp_tm.Minute = Nixie.Time.Minute;
                temp_tm.Second = Nixie.Time.Second;
                temp_tm.Day = SetupVal;
                temp_tm.Month = Nixie.Time.Month;
                temp_tm.Year = Nixie.Time.Year;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                setSyncProvider(RTC.get);
                SetupVal = Nixie.Time.Month;
                break;
              case 2:
                temp_tm.Hour = Nixie.Time.Hour;
                temp_tm.Minute = Nixie.Time.Minute;
                temp_tm.Second = Nixie.Time.Second;
                temp_tm.Day = Nixie.Time.Day;
                temp_tm.Month = SetupVal;
                temp_tm.Year = Nixie.Time.Year;
                RTC.set(makeTime(temp_tm) - (TZ_offset+DST_offset));
                
                setSyncProvider(RTC.get);      
                SetupVal = Nixie.Time.Year;                
                Setup_Subitem++;
                break;
              case 3:
                temp_tm.Hour = Nixie.Time.Hour;
                temp_tm.Minute = Nixie.Time.Minute;
                temp_tm.Second = Nixie.Time.Second;
                temp_tm.Day = Nixie.Time.Day;
                temp_tm.Month = Nixie.Time.Month;
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
              Nixie.SetupDate(SetupVal, Nixie.Time.Month, Nixie.Time.Year, Setup_Subitem, changed);
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
              Nixie.SetupDate(Nixie.Time.Day, SetupVal, Nixie.Time.Year, Setup_Subitem, changed);
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
              Nixie.SetupDate(Nixie.Time.Day, Nixie.Time.Month, SetupVal, Setup_Subitem, changed);                
              break;
          }
          break;
          
      }
   break;    
  }

  Nixie.Pulse();
  
  if (PacketDone) {
    // Received a RF packet
    memcpy(&WorkPacket,&FinishedPacket,PACKET_SIZE);
    ParsePacket(WorkPacket);
    PacketDone=0;
    Steps = 1 + (SensorCount * 2);    
  }
}
