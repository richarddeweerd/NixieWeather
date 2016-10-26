Nixie_Display Nixie(Clock_Latch_Pin, Clock_Clock_Pin, Clock_Data_Pin, Weather_Latch_Pin, Weather_Clock_Pin, Weather_Data_Pin, Clock_High_Dot_Pin, Clock_Low_Dot_Pin, Weather_Dot_Pin, Weather_Min_Dot_Pin, Weather_Max_Dot_Pin, BackLight_Pin, HV_Pin);

Button But_Front(Button_Front_Pin, 1, 20);
Button But_Up(Button_Up_Pin, 1, 20);
Button But_Down(Button_Down_Pin, 1, 20);

int TZ_offset;                  // Timezone offset in seconds
byte  WakeUp_time;

byte SecsPerStep;           // Seconds per step

int Date_Interval;
byte Date_Lenght;

byte ScrSaver_Duration;   // Screensaver duration in seconds
byte ScrSaver_Interval;   // Screensaver interval in minutes must be a devider of 60 eg: 1,2,3,4,5,6,10,12,15,20,30
#define On_hr 7                // 
#define On_min 30               // 
#define Off_hr 0               // 
#define Off_min 30             // 

#define On_time (On_hr * 60) + On_min // 
#define Off_time (Off_hr * 60) + Off_min // 


byte SensorCount=0;

TimeElements temp_tm;

byte Run_Mode = 0;
byte Setup_Item = 0;
byte Setup_Subitem = 0;

time_t CurrentTime = 0;

unsigned long DST_offset;

byte MinMaxStep =0;

byte MinMaxResetCounter;

byte Sec_Step = 0;
unsigned long Sec_Start;

byte CurrentMin;
byte CurrentHour;
byte CurrentDay;
byte CurrentMonth;
byte CurrentYear;

byte ScreenSaver = 0;

byte SecCounter = 0;
byte StepCounter = 1;
byte Steps = 0;

boolean B_Min_Max;
boolean MinMaxReset;
unsigned long B_Min_Max_Start;
unsigned long B_Min_Max_Last;

int SetupVal;
bool SetupStart = true;

byte WakeUp = 0;
