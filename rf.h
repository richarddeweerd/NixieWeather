//Timer 1 (rf reciever ISR)

// 9 msec low is sync bit
// 4 msec low is high bit (1)
// 2 msex low is low bit (0)

// Prescaler is set to 64; = 4 uSec per timer increase

#define MIN_SYNC 2200
#define MAX_SYNC 2300
#define MIN_ONE 900
#define MAX_ONE 1100
#define MIN_ZERO 400
#define MAX_ZERO 600

#define MIN_WAIT 100		// minimum interval since end of last bit 225
#define MAX_WAIT 150		// maximum interval since end of last bit 275

#define INPUT_CAPTURE_IS_RISING_EDGE()    ((TCCR1B & _BV(ICES1)) != 0)
#define INPUT_CAPTURE_IS_FALLING_EDGE()   ((TCCR1B & _BV(ICES1)) == 0)
#define SET_INPUT_CAPTURE_RISING_EDGE()   (TCCR1B |=  _BV(ICES1))
#define SET_INPUT_CAPTURE_FALLING_EDGE()  (TCCR1B &= ~_BV(ICES1))

#define PACKET_SIZE 9   /* number of nibbles in packet (after inital byte) */

bool pulseStart = false;

unsigned int CapturedTime;
unsigned int PreviousCapturedTime;
unsigned int CapturedPeriod;
unsigned int PreviousCapturedPeriod;


byte PreviousCapturedPeriodWasHigh;
byte CapturedPeriodWasHigh;
byte DataPacket[PACKET_SIZE];	  /* actively loading packet */
byte FinishedPacket[PACKET_SIZE]; /* fully read packet */
byte WorkPacket[PACKET_SIZE];
byte PacketBitCounter;


boolean PacketDone;


void init_rf(){
  //Setup for all counters
  cli();  
  // Set up timer1 for RF signal detection
  TCCR1A = B00000000;   //Normal mode of operation, TOP = 0xFFFF, TOV1 Flag Set on MAX
  TCCR1B = ( _BV(ICNC1) | _BV(CS11) | _BV(CS10) );
  SET_INPUT_CAPTURE_RISING_EDGE();
  //Timer1 Input Capture Interrupt Enable, Overflow Interrupt Enable  
  TIMSK1 = ( _BV(ICIE1) | _BV(TOIE1) ); 
  sei();
}


ISR(TIMER1_OVF_vect ){
}


ISR( TIMER1_CAPT_vect )
{

  byte mask;        /* temporary mask byte */

  // Immediately grab the current capture time in case it triggers again and
  // overwrites ICR1 with an unexpected new value
  CapturedTime = ICR1;

  if( INPUT_CAPTURE_IS_RISING_EDGE() )
  {
    //upslope
    SET_INPUT_CAPTURE_FALLING_EDGE();       //previous period was low and just transitioned high
    CapturedPeriodWasHigh = false;          //uiICP_CapturedPeriod about to be stored will be a low period
  } else {
    //downslope
    SET_INPUT_CAPTURE_RISING_EDGE();        //previous period was high and transitioned low
    CapturedPeriodWasHigh = true;           //uiICP_CapturedPeriod about to be stored will be a high period
  }

  CapturedPeriod = (CapturedTime - PreviousCapturedTime);


  if (CapturedPeriodWasHigh == true) { 
    //downslope
    //check pulselength
    if ((CapturedPeriod > MIN_WAIT) && (CapturedPeriod < MAX_WAIT)) { 
      //pulse is right size
      pulseStart = true;
    } else {
      pulseStart = false;
    }
  }

  if ((pulseStart)&&(CapturedPeriodWasHigh == false)){
    pulseStart=false;
    //upslope after valid startpulse
    if ((CapturedPeriod > MIN_SYNC) && (CapturedPeriod < MAX_SYNC))  { 
      //syncbit
      if (PacketBitCounter == 36 ){
        //packet complete
        memcpy(&FinishedPacket,&DataPacket,PACKET_SIZE);
        PacketDone = 1;
      } else {
        //packet error
        PacketBitCounter = 0;
      }
    } else if ((CapturedPeriod > MIN_ONE) && (CapturedPeriod < MAX_ONE)){
      // recieved 1
      mask = (1 << (PacketBitCounter & 0x03));
      DataPacket[(PacketBitCounter >> 2)] |= mask;
      PacketBitCounter++;      
    } else if ((CapturedPeriod > MIN_ZERO) && (CapturedPeriod < MAX_ZERO)){
      //recieved 0
      mask = (1 << (PacketBitCounter & 0x03));
      DataPacket[(PacketBitCounter >> 2)] &= ~mask;
      PacketBitCounter++;
    } else {
      //error bit reset packet
      PacketBitCounter = 0;      
    }
  }

  //save the current capture data as previous so it can be used for period calculation again next time around
  PreviousCapturedTime            = CapturedTime;
  PreviousCapturedPeriod          = CapturedPeriod;
  PreviousCapturedPeriodWasHigh   = CapturedPeriodWasHigh;
}

byte getSensorNum(byte n){
  byte rval=0;
  n = n & 0x03;
  switch (n){
    case 1:
      rval=2;
      break;
    case 2:
      rval=1;
      break;
    case 3:
      rval=3;
      break;    
  }
  return rval;
}

void ParsePacket(byte *Packet) {
  // parse a raw data string
  byte chksum = 0x0F;
  byte SensorId = 0;
  byte SensorAddress = 0;
  int temp;
  byte hum;
  bool batwarn=false;
  
  for (byte j=0; j<(PACKET_SIZE-1); j++) {
    Packet[j] = (Packet[j] & 0x0F);
    chksum -= Packet[j];
  }

  // dbg = (Packet[0]*100) + Packet[1]; works!
  // dbg = Packet[1];
  if ((chksum & 0x0F) == Packet[PACKET_SIZE-1]) { 
    /* checksum pass proces packet*/
    SensorId = getSensorNum(Packet[1]);
    SensorAddress = (Packet[0]<<4) + Packet[1];

    temp = Packet[5];
    temp = temp << 4;
    temp += Packet[4];
    temp = temp << 4;
    temp += Packet[3];
    temp = temp << 4;
    temp = temp /16;

    hum = (Packet[7]*10) + Packet[6];
    
    if (Packet[2]%2 ==1){
      batwarn=true;
    }
    if ((SensorAddress > 0)&&(SensorId > 0)){
      SensorId--;
      if (Nixie.RFSensor[SensorId].Address == 0){
        Nixie.RFSensor[SensorId].Init(SensorAddress, temp, hum, batwarn,(CurrentTime + TZ_offset + DST_offset));
        SensorCount = 0;
        byte i = 0;
        if (Nixie.RFSensor[0].Address != 0){
          SensorCount++;
          SensorArr[i]=0;
          i++;
        }
        if (Nixie.RFSensor[1].Address != 0){
          SensorCount++;
          SensorArr[i]=1;
          i++;
        }
        if (Nixie.RFSensor[2].Address != 0){
          SensorCount++;          
          SensorArr[i]=2;
        }
        WeatherSteps = 1 + (SensorCount * 2);
      } else {
        if (Nixie.RFSensor[SensorId].Address == SensorAddress){
          if (SensorId == 2){
            batwarn = true;
          }
          Nixie.RFSensor[SensorId].SetValues(temp, hum, batwarn,(CurrentTime + TZ_offset + DST_offset));  
        }
      }
    }
  }
}

