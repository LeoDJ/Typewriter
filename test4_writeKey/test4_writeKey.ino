unsigned long lastMicros=0, isrMicros, nextMicros;
byte keys[8];
bool readyToProcess = false;
byte sendX, sendY, sendYHigh, sendYLow, sendCounter = 0;
bool armInt = true;

//!!!!!!!!!!!!!!!!!!!!!!!!!!!
//nächster Code in PlatformIO
//!!!!!!!!!!!!!!!!!!!!!!!!!!!

void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

ISR (PCINT2_vect) // D0 to D7 (PORTD)
{
  if(PIND & B11100000) //if valid pin got triggered
  {
    isr();
  }
}  

ISR (PCINT0_vect) // D8 to D13 (PORTB)
{    
  if(PINB & B00011111) //if valid pin got triggered
  {
    isr();
  }
}

void isr()
{
  //PORTD |= (1 << 4); //takes 0.125us
  byte lower = (PIND & B11100000) >> 5;
  byte upper = (PINB & B00011111) << 3;
  byte res = ~(upper | lower);
  //PORTD &= ~(1 << 4); //takes 0.125us
  //Serial.write(res);
  //falling edge && prevent double execution if 2 interrupts happen at the same time on different pins
  if(res > 0 && armInt/*&& micros() > nextMicros /*isrMicros - lastMicros > 40*/) 
  {
    armInt = false;
    /*PORTD |= (1 << 4); //takse 0.125us
    PORTD &= ~(1 << 4); //takes 0.125us*/
    
    if(res == sendX)
    {
      /*DDRC |= (B00111111 & sendY); //set A0-A5 (y0-y5)
      DDRD |= (sendY >> 4) & B00001100; //set 2,3 (y6,y7)*/
      DDRC |= sendYLow;
      DDRD |= sendYHigh;
      sendCounter++;
    }
    //lastMicros = isrMicros;
    //PORTD |= (1 << 4); //takes 0.125us
    /*byte bitPos = __builtin_ctz(res); //takes 1.5us
    byte val = PINC | ((PIND & B00001100) << 4);
    keys[bitPos] = ~val;
    if(bitPos == 7)
      readyToProcess = true;*/
    //PORTD &= ~(1 << 4); //takes 0.125us
    /*Serial.write(bitPos);
    Serial.write(~val);*/
    //nextMicros = micros() + 40; //move debounce code here to increase speed of if
  }
  else if (res == 0)
  {
    //PORTD &= ~(1 << 4); //takes 0.125us
    armInt = true;
    DDRC &= B11000000; //set pins A0-A5 as input
    DDRD &= B11110011; //set pin 2, 3 as input
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(4, OUTPUT);
  for (byte i=5; i<=12; i++)
  {
    pinMode(i, INPUT_PULLUP);
    pciSetup(i);
  }
  Serial.begin(2000000);   
  
}

bool didRun = false;

void loop() {
  // put your main code here, to run repeatedly:
  if(millis() % 1000 == 0 && !didRun)
  {
    /*sendX = 1 << 5;
    sendY = 1 << 6;*/
    sendKey(5, 6);
    didRun = true;
  }
  
  if(sendCounter >=8)
  {
    sendCounter = 0;
    /*sendX = 0;
    sendY = 0;*/
    sendKey(8,8); //turn off
    didRun = false;
  }
}

void sendKey(byte x, byte y)
{
  if(x < 8 && y < 8)
  {
    sendX = 1 << x;
    sendYLow = (1 << y) & B00111111; //set A0-A5 (y0-y5)
    sendYHigh = ((1 << y) >> 4) & B00001100; //set 2,3 (y6,y7)
  }
  else
  {
    sendX = 0;
    sendYLow = 0;
    sendYHigh = 0;
  }
  
}

