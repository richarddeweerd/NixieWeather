#define BackLight_Bright 250   // Brightness for backlight
#define SecsPerStep 2          // Seconds per step
#define AntiBounceTime 100     // Anti bounce time for buttons in millisecs
#define TZ_offset 3600         // Timezone offset in seconds
#define MinMaxResetTime 15     // Timeout for Min / Max Display
#define ScrSaver_Duration 30   // Screensaver duration in seconds
#define ScrSaver_Interval 15   // Screensaver interval in minutes must be a devider of 60 eg: 1,2,3,4,5,6,10,12,15,20,30

#define PulsesPerSec 20        // total pulses send to the display every seccond, used for blinking and screensaver
#define PulseInterval 1000/PulsesPerSec 

#define On_hr 7                // 
#define On_min 30               // 
#define Off_hr 0               // 
#define Off_min 30             // 
 
#define On_time (On_hr * 60) + On_min // 
#define Off_time (Off_hr * 60) + Off_min // 

#define _DST_offset 3600

#define WakeUp_time 10

#define MaxSensors 2

const byte DST_dates [16] [5] = {
  {45, 3, 29, 10, 25},
  {46, 3, 27, 10, 30},
  {47, 3, 26, 10, 29},
  {48, 3, 25, 10, 28},
  {49, 3, 31, 10, 27},
  {50, 3, 29, 10, 25},
  {51, 3, 28, 10, 31},
  {52, 3, 27, 10, 30},
  {53, 3, 26, 10, 29},
  {54, 3, 26, 10, 29},
  {55, 3, 31, 10, 27},
  {56, 3, 29, 10, 25},
  {57, 3, 28, 10, 31},
  {58, 3, 27, 10, 30},
  {59, 3, 26, 10, 29},
  {60, 3, 26, 10, 29},
};

