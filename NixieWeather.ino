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

    PulseCount=0;
    CurrentTime = now();
    breakTime ((CurrentTime + TZ_offset + DST_offset),tm);

    if (SecCounter == SecsPerStep){
      SecCounter = 0;
      StepCounter++;
      if (StepCounter > Steps)
      {
        StepCounter = 1;
      }
    }

    //Clock part:
    if ((tm.Second % 20 == 18)||(tm.Second % 20 == 19)){
        Nixie.ShowDate(tm.Day, tm.Month, tm.Year);
        
    } else {
        Nixie.ShowTime(tm.Hour, tm.Minute, tm.Second);
    }

    //Weather info
    if (StepCounter == 1){
      //Show Pressure
      //Nixie.ShowPressure(Baro.Pressure, Baro.MinMaxLed);
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
        
    if (tm.Second % 6 == 1){
      // read barometer every 10 seconds
      Baro.SetTemp(bmp085GetTemperature(bmp085ReadUT()));
      Baro.SetPressure(bmp085GetPressure(bmp085ReadUP()),(CurrentTime + TZ_offset + DST_offset));
    }
    
    if (tm.Minute != CurrentMin){
      //New Minute!
      CurrentMin=tm.Minute;
      // Check if nighmode is enabled
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

  long curmil = millis();
  if (curmil < Sec_Start){
    // millis rolover
    Sec_Start = curmil;
  }
  curmil = curmil - Sec_Start;
  
  if ((curmil >= (PulseCount + 1)*PulseInterval)&&(curmil<1000)){
    PulseCount++;
    Nixie.Pulse(PulseCount);    
  }
  
  if (PacketDone) {
    // Received a RF packet
    memcpy(&WorkPacket,&FinishedPacket,PACKET_SIZE);
    ParsePacket(WorkPacket);
    PacketDone=0;
    Steps = 1 + (SensorCount * 2);    
  }
}
