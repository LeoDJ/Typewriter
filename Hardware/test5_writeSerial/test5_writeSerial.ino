unsigned long lastMicros=0, isrMicros, nextMicros;
byte keys[8];
bool readyToProcess = false;
byte sendX, sendY, sendYHigh, sendYLow, sendModifier, sendCounter = 0;
bool armInt = true;
//bool didRun = false;
char toPrintBuf[128];
byte toPrintIndex=0, curPrintIndex=0;
unsigned long lastSendKeyMillis=0, curMillis;

// 7     6     5     4     3     2     1     0
// 0     1     2     3     4     5     6     7
char keysSmall[8][8] = {
{  0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 }, //0 | 7
{  0 , '\'', '\t',  ' ', '\n',   0 ,   0 , '\b'}, //1 | 6
{  0 ,  '´',  'ß',   0 ,  'ä',   0 ,  'ü',  '+'}, //2 | 5
{ '-',  '0',  '9',  '.',  'ö',  'p',  'l',  'o'}, //3 | 4
{ 'y',  'b',  'n',  'x',  'c',  'v',  ',',  'm'}, //4 | 3
{ 'a',  'g',  'h',  's',  'd',  'f',  'k',  'j'}, //5 | 2
{ 'q',  't',  'z',  'w',  'e',  'r',  'i',  'u'}, //6 | 1
{ '1',  '5',  '6',  '2',  '3',  '4',  '8',  '7'}  //7 | 0
};
// 7     6     5     4     3     2     1     0
// 0     1     2     3     4     5     6     7
char keysShift[8][8] = {
{  0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 }, //0 | 7
{  0 , '\'', '\t',  ' ', '\n',   0 ,   0 , '\b'}, //1 | 6
{  0 ,  '`',  '?',   0 ,  'Ä',   0 ,  'Ü',  '*'}, //2 | 5
{ '_',  '=',  ')',  ':',  'Ö',  'P',  'L',  'O'}, //3 | 4
{ 'Y',  'B',  'N',  'X',  'C',  'V',  ';',  'M'}, //4 | 3
{ 'A',  'G',  'H',  'S',  'D',  'F',  'K',  'J'}, //5 | 2
{ 'Q',  'T',  'Z',  'W',  'E',  'R',  'I',  'U'}, //6 | 1
{ '!',  '%',  '&', '\"',  '§',  '$',  '(',  '/'}  //7 | 0
};

void isr()
{
  /*PORTB |= (1 << 5); //takes 0.125us
  PORTB &= ~(1 << 5); //takes 0.125us*/
  byte lower = (PIND & B11100000) >> 5; //takes
  byte upper = (PINB & B00011111) << 3; //0.75
  byte res = ~(upper | lower);          //us
  //Serial.write(res&sendX);
  //Serial.write((res & sendX) != 0 /*&& armInt*/);
  //falling edge && prevent double execution if 2 interrupts happen at the same time on different pins
  if((res & B00000001) != 0 && armInt && sendModifier == 1)
  {
    DDRC |= B00000001; //send shift key
    armInt = false;
  }
  else if((res & sendX) != 0 && armInt)
  {
    DDRC |= sendYLow;
    DDRD |= sendYHigh;

    sendCounter++;
    armInt = false;
  }
  else if (res == 0)
  {
    armInt = true;
    DDRC &= B11000000; //set pins A0-A5 as input
    DDRD &= B11110011; //set pin 2, 3 as input
  }
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


void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void setup() {
  // put your setup code here, to run once:
  pinMode(4, OUTPUT);
  for (byte i=5; i<=12; i++)
  {
    pinMode(i, INPUT_PULLUP);
    pciSetup(i);
  }
  for(byte i = 0; i < sizeof(toPrintBuf); i++) //clear buffer array
  {
    toPrintBuf[i] = 0;
  }
  Serial.begin(115200);

}


bool didRun = false;
void loop() {
  curMillis = millis();

  if(sendCounter >=4)
  {
    sendCounter = 0;
    sendKeyRaw(8,8); //turn off
    didRun = false;
  }

  if(curMillis - lastSendKeyMillis > 50)
  {
    lastSendKeyMillis = curMillis;
    if(toPrintIndex > 0)
    {
      char c = toPrintBuf[curPrintIndex];
      if (c != 0)
      {
        bool found = false;
        for(byte x = 0; x < 8; x++)
        {
          for(byte y = 0; y < 8; y++)
          {
            if(keysSmall[x][y] == c)
            {
              noInterrupts();
              found = true;
              Serial.println(String(x) + String(y));
              sendKeyRaw(x, y);
              interrupts();
            }
            else if(keysShift[x][y] == c)
            {
              noInterrupts();
              found = true;
              sendModifier = 1; //shift keys
              sendKeyRaw(x, y);
              interrupts();
            }
          }
          if(found)
            break;
        }
        if(!found)
        {
          Serial.println("Unknown character \'" + String((unsigned char)c) + "\' ");
        }
        curPrintIndex++;
      }
      else //when reached end of array
      {
        for(byte i = 0; i < sizeof(toPrintBuf); i++) //clear buffer array
        {
          toPrintBuf[i] = 0;
        }
        toPrintIndex = 0;
        curPrintIndex = 0;
      }
    }
  }
/*
  if(millis() % 500 == 0 && !didRun)
  {
    sendModifier = 0; //shift
    //sendKeyRaw(1, 4); //space
    sendKeyRaw(5, 4); //d
    //sendKeyRaw()
    didRun = true;
  }
*/

  if(Serial.available())
  {
    char c = Serial.read();
    toPrintBuf[toPrintIndex] = c;
    toPrintIndex++;
  }
}

void sendKeyRaw(byte x, byte y)
{
  if(x < 8 && y < 8)
  {
    sendX = 1 << x;
    sendYLow = (1 << y) & B00111111; //set A0-A5 (y0-y5)
    sendYHigh = ((1 << y) >> 4) & B00001100; //set 2,3 (y6,y7)
  }
  else
  {
    sendX = 255;
    sendYLow = 0;
    sendYHigh = 0;
    sendModifier = 0;
  }
}
