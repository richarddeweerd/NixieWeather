#include <avr/pgmspace.h>
#include "Nixie.h"
#include "PinDef.h"
#include "Wire.h"
#include "TimeLib.h"
#include "DS3231RTC.h"
#include "Button.h"
#include <EEPROM.h>

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
      InitEEprom();
    }
  }
  //InitEEprom(); //Needed during development!!!!!!!!!!!!!!!!!
  ReadConfig();
  
  Nixie.DimIntensity = Nixie.BackLightDay;  
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
    
    if (((CurrentMin % (60/ScrSaver_Interval)) == 0)&&((60 - ScrSaver_Duration) == Nixie.Time.Second )){
       //Nixie.ScreenSaverStart(ScrSaver_Duration,tm.Hour,tm.Minute, tm.Second, Baro.Pressure);
    }
    
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
      //Nixie.Baro.SetPressure(bmp085GetPressure(bmp085ReadUP()),(CurrentTime + TZ_offset + DST_offset));
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
      Setup_Item = 1;
    }    
  }
  
  switch (Run_Mode){
    case 0:
      //Normal Operation
      
      //Clock part:
      //First check if time or date should be shown
      if (Nixie.Time.Second % Date_Interval >= Date_Interval - Date_Lenght){
          Nixie.ShowDate();          
      } else {        
         Nixie.ShowTime();
      }
  
      //Weather info
      if (StepCounter == 1){
        //Show Pressure
        Nixie.ShowPressure();
      } else {
        //Temp or humidity
        if (StepCounter % 2 == 0){
          //Temp 
          Nixie.ShowTemp(StepCounter/ 2);
        } else {
          //Hum
          Nixie.ShowHum(StepCounter/ 2);
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
      //Setup mode
      if (But_Front.hold(2000)){
        //Exit setup mode
        Run_Mode = 0;
        Setup_Item = 0;
        Nixie.ExitSetup();
      }
      if (But_Front.status(3000)){
        //Next menu item
        if (Run_Mode == 99){
          if (Setup_Item <6){
            Setup_Item++;
          } else {
            Setup_Item=1;
          }
        }
      }
      
      byte ValChanged = 0;
      
      switch (Setup_Item){
        case 1:
          //set hour
          if (But_Up.status(1000,250)){
            ValChanged = 1;
            SetupVal = incMax(Nixie.Time.Hour, 0, 23);
            SaveTime(SetupVal,1);
          }
          if (But_Down.status(1000,250)){
            ValChanged = 1;            
            SetupVal = decMax(Nixie.Time.Hour, 0, 23);
            SaveTime(SetupVal,1);
          }
          Nixie.SetupClock(1,ValChanged);
          break;
        case 2:
          //set minute
          if (But_Up.status(1000,250)){
            ValChanged = 1;
            SetupVal = incMax(Nixie.Time.Minute, 0, 59);
            SaveTime(SetupVal,2);
          }
          if (But_Down.status(1000,250)){
            ValChanged = 1;            
            SetupVal = decMax(Nixie.Time.Minute, 0, 59);
            SaveTime(SetupVal,2);
          }          
          Nixie.SetupClock(2,ValChanged);
          break;
        case 3:
          //set seccond
          if (But_Up.status(1000,250)||But_Down.status(1000,250)){
            ValChanged = 1;            
            SaveTime(0,3);
          }          
          Nixie.SetupClock(3,ValChanged);
          break;
        case 4:
          //set year
           if (But_Up.status(1000,250)){
            ValChanged = 1;
            SetupVal = incMax(Nixie.Time.Year, 0, 99);
            SaveTime(SetupVal,6);
          }
          if (But_Down.status(1000,250)){
            ValChanged = 1;            
            SetupVal = decMax(Nixie.Time.Year, 0, 99);
            SaveTime(SetupVal,6);
          }
          Nixie.SetupClock(6,ValChanged);
          break;            
        case 5:
          //set month
           if (But_Up.status(1000,250)){
            ValChanged = 1;
            SetupVal = incMax(Nixie.Time.Month, 1, 12);
            SaveTime(SetupVal,5);
          }
          if (But_Down.status(1000,250)){
            ValChanged = 1;            
            SetupVal = decMax(Nixie.Time.Month, 1, 12);
            SaveTime(SetupVal,5);
          }
          Nixie.SetupClock(5,ValChanged);
          break;
        case 6:
          //set day
           if (But_Up.status(1000,250)){
            ValChanged = 1;
            SetupVal = incMax(Nixie.Time.Day, 1, 31);
            SaveTime(SetupVal,4);
          }
          if (But_Down.status(1000,250)){
            ValChanged = 1;            
            SetupVal = decMax(Nixie.Time.Day, 1, 31);
            SaveTime(SetupVal,4);
          }
          Nixie.SetupClock(4,ValChanged);
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
