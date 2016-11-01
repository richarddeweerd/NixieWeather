#define _DST_offset 3600

#define AntiBounceTime 100     // Anti bounce time for buttons in millisecs
#define MinMaxResetTime 15     // Timeout for Min / Max Display

#define PulsesPerSec 20        // total pulses send to the display every seccond, used for blinking and screensaver
#define PulseInterval 1000/PulsesPerSec 

#define EEpromTimeZone       0
#define EEpromDatePerMin     1
#define EEpromDateLength     2
#define EEpromWeatherLength  3
#define EEpromScrPerHour     4
#define EEpromScrLength      5
#define EEpromBackLightDay   6
#define EEpromBackLightNight 7
#define EEpromDayStartHour   8
#define EEpromDayStartMin    9
#define EEpromDayEndHour     10
#define EEpromDayEndMin      11
#define EEpromWakeup         12

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

byte minuteArr[] = {1,2,3,4,5,6,10,12,15,20,30,60};
byte menuArr[] = {0,1,1,1,2,2,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
