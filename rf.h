//Timer 1 (rf reciever ISR)
// 0.5 ms high is a one
#define MIN_ONE 110		// minimum length of '1'110
#define MAX_ONE 170		// maximum length of '1'155 170
// 1.3 ms high is a zero
#define MIN_ZERO 310		// minimum length of '0'310
#define MAX_ZERO 370		// maximum length of '0'370
// 1 ms between bits
#define MIN_WAIT 225		// minimum interval since end of last bit 225
#define MAX_WAIT 275		// maximum interval since end of last bit 275

#define INPUT_CAPTURE_IS_RISING_EDGE()    ((TCCR1B & _BV(ICES1)) != 0)
#define INPUT_CAPTURE_IS_FALLING_EDGE()   ((TCCR1B & _BV(ICES1)) == 0)
#define SET_INPUT_CAPTURE_RISING_EDGE()   (TCCR1B |=  _BV(ICES1))
#define SET_INPUT_CAPTURE_FALLING_EDGE()  (TCCR1B &= ~_BV(ICES1))

#define PACKET_SIZE 9   /* number of nibbles in packet (after inital byte) */
#define PACKET_START 0x0A	/* byte to match for start of packet */

unsigned int CapturedTime;
unsigned int PreviousCapturedTime;
unsigned int CapturedPeriod;
unsigned int PreviousCapturedPeriod;
unsigned int SinceLastBit;
unsigned int LastBitTime;
unsigned int BitCount;

byte PreviousCapturedPeriodWasHigh;
byte mask;		    /* temporary mask byte */
byte CompByte;		    /* byte containing the last 8 bits read */
byte CapturedPeriodWasHigh;
byte DataPacket[PACKET_SIZE];	  /* actively loading packet */
byte FinishedPacket[PACKET_SIZE]; /* fully read packet */
byte WorkPacket[PACKET_SIZE];
byte PacketBitCounter;
byte j;
byte sensor;
byte PacketType;
byte SensorNumber;
byte hum;

boolean ReadingPacket;
boolean PacketDone;

float tempC;

void init_rf(){
  //Setup for all counters

  cli();
  
  // Set up timer1 for RF signal detection
  TCCR1A = B00000000;   //Normal mode of operation, TOP = 0xFFFF, TOV1 Flag Set on MAX
  //TCCR1A = 0x00;
  //TCCR1A |= (1<<COM1A1);
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
  // Immediately grab the current capture time in case it triggers again and
  // overwrites ICR1 with an unexpected new value
  CapturedTime = ICR1;

  if( INPUT_CAPTURE_IS_RISING_EDGE() )
  {
    SET_INPUT_CAPTURE_FALLING_EDGE();      //previous period was low and just transitioned high
    CapturedPeriodWasHigh = false;    //uiICP_CapturedPeriod about to be stored will be a low period
  } else {
    SET_INPUT_CAPTURE_RISING_EDGE();       //previous period was high and transitioned low
    CapturedPeriodWasHigh = true;     //uiICP_CapturedPeriod about to be stored will be a high period
  }

  CapturedPeriod = (CapturedTime - PreviousCapturedTime);

  if ((CapturedPeriod > MIN_ONE) && (CapturedPeriodWasHigh == true)) { // possible bit
    /* time from end of last bit to beginning of this one */
    SinceLastBit = (PreviousCapturedTime - LastBitTime);
    
    if ((CapturedPeriod < MAX_ONE) && (SinceLastBit > MIN_WAIT)) {
      if (SinceLastBit > MAX_WAIT) { // too long since last bit read
	if ((SinceLastBit > (2*MIN_WAIT+MIN_ONE)) && (SinceLastBit < (2*MAX_WAIT+MAX_ONE))) { /* missed a one */

	} else {
	  if ((SinceLastBit > (2*MIN_WAIT+MIN_ZERO)) && (SinceLastBit < (2*MAX_WAIT+MAX_ZERO))) { /* missed a zero */

	  }
	}

	if (ReadingPacket) {

	  ReadingPacket=0;
	  PacketBitCounter=0;
	}
	CompByte=0xFF;			  /* reset comparison byte */
      } else { /* call it a one */
	if (ReadingPacket) {	/* record the bit as a one */
	  //	  Serial.print("1");
	  mask = (1 << (3 - (PacketBitCounter & 0x03)));
	  DataPacket[(PacketBitCounter >> 2)] |= mask;
	  PacketBitCounter++;
	} else {		  /* still looking for valid packet data */
	  if (CompByte != 0xFF) {	/* don't bother recording if no zeros recently */
	    CompByte = ((CompByte << 1) | 0x01); /* push one on the end */
	  }
	}
	LastBitTime = CapturedTime;
      }
    } else {			/* Check whether it's a zero */
      if ((CapturedPeriod > MIN_ZERO) && (CapturedPeriod < MAX_ZERO)) {
	if (ReadingPacket) {	/* record the bit as a zero */
	  //	  Serial.print("0");
	  mask = (1 << (3 - (PacketBitCounter & 0x03)));
	  DataPacket[(PacketBitCounter >> 2)] &= ~mask;
	  PacketBitCounter++;
	} else {		      /* still looking for valid packet data */
	  CompByte = (CompByte << 1); /* push zero on the end */

	}
	LastBitTime = CapturedTime;
      }
    }
  }

  if (ReadingPacket) {
    if (PacketBitCounter == (4*PACKET_SIZE)) { /* done reading packet */
      memcpy(&FinishedPacket,&DataPacket,PACKET_SIZE);

      PacketDone = 1;
      ReadingPacket = 0;
      PacketBitCounter = 0;
    }
  } else {
    /* Check whether we have the start of a data packet */
    if (CompByte == PACKET_START) {
      //      Serial.println("Got packet start!");
      CompByte=0xFF;		/* reset comparison byte */

      /* set a flag and start recording data */
      ReadingPacket = 1;
    }
  }

  //save the current capture data as previous so it can be used for period calculation again next time around
  PreviousCapturedTime           = CapturedTime;
  PreviousCapturedPeriod         = CapturedPeriod;
  PreviousCapturedPeriodWasHigh   = CapturedPeriodWasHigh;
}

void ParsePacket(byte *Packet) {
  // parse a raw data string
  byte chksum = 0x0A;
  byte ins_sens;
  for (j=0; j<(PACKET_SIZE-1); j++) {
    chksum += Packet[j];
  }
  PacketType=0;  
  if ((chksum & 0x0F) == Packet[PACKET_SIZE-1]) { /* checksum pass */
    /* check for bad digits and make sure that most significant digits repeat */
    if ((Packet[3]==Packet[6]) && (Packet[4]==Packet[7]) && (Packet[3]<10) && (Packet[4]<10) && (Packet[5]<10)) {
      sensor=(((Packet[1] << 4)|Packet[2]) & 254)>>1;
      if (Packet[0]==0) {		/* temperature packet */
        tempC=(Packet[3]*10-50 + Packet[4] + ( (float) Packet[5])/10);
        PacketType=1;
      } else {
	if (Packet[0]==0x0E) {		/* humidity packet */
	  hum=(Packet[3]*10 + Packet[4]);
          PacketType=2;
	} else  {
	  if (Packet[0]==0x0B) {		/* custom packet */
	    tempC=(Packet[3]*10-50 + Packet[4] + ( (float) Packet[5])/10);
            PacketType=1;
	  }
	}
      }
    }
  }
  if (PacketType > 0){
    SensorNumber = 0;
    // Check if sensoraddress is known
    for (j=1; j<=SensorCount; j++) {
      if (Nixie.RFSensor[j].Address == sensor){
        SensorNumber = j;
      }       
    }
    
//    if (SensorNumber == 0){
//      //new sensor
//      //Check if the sensor needs to be inserted before existing
//      for (j=1; j<=SensorCount; j++) {
//        if (RFSensor[j].Address < sensor){
//          SensorNumber = j;
//          RFSensor[SensorNumber].Init(sensor);
//          j=SensorCount+1;
//        }
//      }
//    }
    
    if (SensorNumber == 0){
      //new sensor
      if (SensorCount < MaxSensors){
        SensorCount++;
        SensorNumber = SensorCount;
        Nixie.RFSensor[SensorNumber].Init(sensor);        
      }
    }
    
    switch (PacketType){
      case 1:
        Nixie.RFSensor[SensorNumber].SetTemp(tempC, (CurrentTime + TZ_offset + DST_offset));
        break;
      case 2:
        Nixie.RFSensor[SensorNumber].SetHum(hum, (CurrentTime + TZ_offset + DST_offset));
        break; 
    } 
  }
}

