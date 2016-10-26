#define _DST_offset 3600

#define AntiBounceTime 100     // Anti bounce time for buttons in millisecs
#define MinMaxResetTime 15     // Timeout for Min / Max Display

#define PulsesPerSec 20        // total pulses send to the display every seccond, used for blinking and screensaver
#define PulseInterval 1000/PulsesPerSec 

#define MaxSensors 2

#define EEpromDatePerMin     0
#define EEpromDateLength     1
#define EEpromWeatherLength  2
#define EEpromScrLength      3
#define EEpromScrPerHour     4
#define EEpromBackLightDay   5
#define EEpromBackLightNight 6
#define EEpromDayStartHour   7
#define EEpromDayStartMin    8
#define EEpromDayEndHour     9
#define EEpromDayEndMin      10
#define EEpromWakeup         11
#define EEpromTimeZone       12

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

