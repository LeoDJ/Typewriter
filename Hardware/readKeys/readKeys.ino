unsigned long lastMicros=0, isrMicros;
byte keys[8];
bool readyToProcess = false;

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
  isrMicros = micros();
  byte lower = (PIND & B11100000) >> 5;
  byte upper = (PINB & B00011111) << 3;
  byte res = ~(upper | lower);
  //Serial.write(res);
  //falling edge && prevent double execution if 2 interrupts happen at the same time on different pins
  if(res > 0 && isrMicros - lastMicros > 40) 
  {
    lastMicros = isrMicros;
    PORTD |= (1 << 4); //takes 0.125us
    byte bitPos = __builtin_ctz(res); //takes 1.5us
    byte val = PINC | ((PIND & B00001100) << 4);
    keys[bitPos] = ~val;
    if(bitPos == 7)
      readyToProcess = true;
    PORTD &= ~(1 << 4); //takes 0.125us
    //Serial.write(val);
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
  Serial.begin(115200);   
  
}

byte lastVal = 9;
byte lastI = 9;

void loop() {
  // put your main code here, to run repeatedly:
  if(readyToProcess)
  {
    for(byte i = 0; i < 8; ++i)
    {
      byte val = keys[i];
      if (val != 0 && (val != lastVal || i != lastI))
      {
        lastVal = val;
        lastI = i;
        Serial.print(i);
        Serial.print("/");
        Serial.println(__builtin_ctz(val));
      }
    }
  }
}
