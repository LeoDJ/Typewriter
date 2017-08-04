#include <Arduino.h>

const byte inputPin[] =  { 5,  6,  7,  8,  9, 10, 11, 12};
const byte outputPin[] = {20, 19, 18, 17, 16, 15, 14, 13};
const byte inputPinLength = sizeof(inputPin) / sizeof(inputPin[0]);
const byte outputPinLength = sizeof(outputPin) / sizeof(outputPin[0]);
bool lastInputState[inputPinLength];


unsigned long lastMicros=0, isrMicros, nextMicros;
byte keys[8];
bool readyToProcess = false;
byte sendX, sendY, sendYIdx, sendYHigh, sendYLow;
/*volatile*/ byte sendModifier, sendCounter = 0;
/*volatile*/ bool armInt = true;
//bool didRun = false;
char toPrintBuf[128];
byte toPrintIndex=0, curPrintIndex=0;
unsigned long lastSendKeyMillis=0, curMillis;


/////// keymaps //////
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

/* Teensy 3.2 Port to PIND
Port - PinNumber
A12 - 3
A13 - 4

B00 - 16
B01 - 17
B02 - 19
B03 - 18

B16 - 0
B17 - 1

C00 - 15
C01 - 22
C02 - 23
C03 - 09
C04 - 10
C05 - 13
C06 - 11
C07 - 12

D00 - 2
D01 - 14
D02 - 7
D03 - 8
D04 - 6
D05 - 20
D06 - 21
D07 - 5
*/

inline FASTRUN void isr() {
  //do magic stuff
  pinMode(2, INPUT);
  byte res = 0;
  res |= digitalReadFast(inputPin[0]) << 0;
  res |= digitalReadFast(inputPin[1]) << 1;
  res |= digitalReadFast(inputPin[2]) << 2;
  res |= digitalReadFast(inputPin[3]) << 3;
  res |= digitalReadFast(inputPin[4]) << 4;
  res |= digitalReadFast(inputPin[5]) << 5;
  res |= digitalReadFast(inputPin[6]) << 6;
  res |= digitalReadFast(inputPin[7]) << 7;
  res = ~res;

  /*for(byte i = 0; i < inputPinLength; i++) { //should be quite fast on teensy. TODO: test this
    res |= (!digitalReadFast(inputPin[i])) << i; //invert due to active low pin
}*/
  if((res & B00000001) != 0 && armInt && sendModifier == 1) {
    pinMode(outputPin[0], OUTPUT); //send shift key
    armInt = false;
  }
  else if((res & sendX) != 0 && armInt) {
    pinMode(outputPin[sendYIdx], OUTPUT); //hopefully standard pinMode() will be fast enough
    sendCounter++;
    armInt = false;
  }
  else if(res == 0) {
    armInt = true;
    for(byte i = 0; i < outputPinLength; i++) //speed does not matter here
        pinMode(outputPin[i], INPUT);
  }
  pinMode(2, OUTPUT);
}
/*
void isrClear() {//reset pulled low pin
    byte res = 0;
    for(byte i = 0; i < inputPinLength; i++) { //should be quite fast on teensy. TODO: test this
      res |= (!digitalReadFast(inputPin[i])) << i; //invert due to active low pin
    }
    if(res == 0) { //probably unneded, if a pin rises again, because of the protocol
      armInt = true;
      pinMode(outputPin[sendYIdx], INPUT); //hopefully sendYIdx is still set to the right value, else need to set all pins to input
    }
}*/


/*
 * commented out function portcd_interrupt
 * in file cores/teensy3/pins_teensy.c
 * @L310 - L332
 */

/*
static void portcd_interrupt() {
    if((PINC | B00100111) != 255 || (PIND | B01100011) != 255) //if one went low
        isr();

    pinMode(20, OUTPUT);
    pinMode(20, INPUT);
}*/
/*
void portd_isr() {
    if((PIND | B01100011) != 255)
        isr();
}
*/
void emptyIsr() {

}

void setup() {
  // put your setup code here, to run once:
  pinMode(4, OUTPUT);
  /*attachInterruptVector(IRQ_PORTC, isrC);
  attachInterruptVector(IRQ_PORTD, isrD);*/
  for (byte i = 0; i < inputPinLength; i++)
  {
    pinMode(inputPin[i], INPUT_PULLUP);
    attachInterrupt(inputPin[i], isr, CHANGE);
    //attachInterrupt(inputPin[i], isrClear, RISING);
    //pciSetup(i);
  }
  for(byte i = 0; i < sizeof(toPrintBuf); i++) //clear buffer array
  {
    toPrintBuf[i] = 0;
  }
  Serial.begin(115200);
  /*digitalWrite(2, LOW);
  pinMode(2, OUTPUT);
  for(byte i = 0; i < 250; i++) { //test speed of pinMode function
    digitalWriteFast(2, HIGH);
    digitalWriteFast(2, LOW);
  }
  for(byte i = 0; i < 250; i++) { //test speed of pinMode function
    pinMode(2, INPUT);
    pinMode(2, OUTPUT);
}*/
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
              Serial.println(String(x) + String(y));
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
    sendY = 1 << y;
    sendYIdx = y;
    sendYLow = (1 << y) & B00111111; //set A0-A5 (y0-y5)
    sendYHigh = ((1 << y) >> 4) & B00001100; //set 2,3 (y6,y7)
  }
  else
  {
    sendX = 255;
    sendY = 0;
    sendYIdx = 255;
    sendYLow = 0;
    sendYHigh = 0;
    sendModifier = 0;
  }
}
