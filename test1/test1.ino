/*#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
*/
unsigned long lastMicros=0, isrMicros;
byte keys[8];
byte idx=0;
bool readFinish = false;

void isr()
{ //[3us]
  isrMicros = micros(); //takes 3.25us
  if(isrMicros - lastMicros > 4000) //takes 1.5us
  { //[7.75us]
    lastMicros = isrMicros; //takes 1us
    /*PORTD |= (1 << 4); //takes 0.125us
    PORTD &= ~(1 << 4); //takes 0.125us
    keys[idx] = PINB; //takes 0.5us
    idx++; //takes 0.315us
    if(idx>7) //takes 0.315us (without execution) / 0.25us with exec
    {
      idx=0; //takes 0.125us
      readFinish=true;
    }
    PORTD |= (1 << 4); //takes 0.125us
    PORTD &= ~(1 << 4); //takes 0.125us*/
    for(byte i = 0; i < 8; i++)
    {
      PORTD |= (1 << 4); //takes 0.125us
      delayMicroseconds(12);
      byte lower = (PIND & B11100000) >> 5;
      byte upper = (PINB & B00011111) << 3;
      keys[i] = upper | lower;
      PORTD &= ~(1 << 4); //takes 0.125us
      delayMicroseconds(160);
    }
    readFinish = true;
  }
  /*PORTB |= (1 << 0);
  delayMicroseconds(10);
  PORTB &= ~(1 << 0);*/
  //isrMicros = micros();
  //Serial.println(micros());
}

void setup() {
  // put your setup code here, to run once:
  noInterrupts();
  pinMode(4, OUTPUT);
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), isr, FALLING);
  Serial.begin(115200);
  //u8g2.begin();
  interrupts();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(readFinish)
  {
    readFinish = false;
    for(byte i = 0; i < 8; i++)
    {
      if(keys[i] != 255)
      {
        String m = "";
        m += String(i);
        m += ": ";
        m += String(keys[i], BIN);
        Serial.println(m);
        /*Serial.print(i);
        Serial.print(" ");
        Serial.println(keys[i], BIN);*/
        /*
        u8g2.firstPage();
        do {
          u8g2.setFont(u8g2_font_crox3h_tr);
          u8g2.drawStr(0,13,m.c_str());
        } while ( u8g2.nextPage() );*/
      }
    }
  }
 /* int t = isrMicros-lastMicros;
  if(t!=0)
    Serial.println(t);
  lastMicros = isrMicros;*/
}
