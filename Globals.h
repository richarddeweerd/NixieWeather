Nixie_Display Nixie(Clock_Latch_Pin, Clock_Clock_Pin, Clock_Data_Pin, Weather_Latch_Pin, Weather_Clock_Pin, Weather_Data_Pin, Clock_High_Dot_Pin, Clock_Low_Dot_Pin, Weather_Dot_Pin, Weather_Min_Dot_Pin, Weather_Max_Dot_Pin, BackLight_Pin, HV_Pin);

Baro_Sensor Baro;
RF_Sensor RFSensor[10];

Button But_Front(Button_Front_Pin, 1, 20);
Button But_Up(Button_Up_Pin, 1, 20);
Button But_Down(Button_Down_Pin, 1, 20);

byte SensorCount=0;

TimeElements tm, te;
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
