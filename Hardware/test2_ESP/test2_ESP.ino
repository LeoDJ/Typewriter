
/* ABORTED due to timing and pin issues
 * timing of scanning is not consistent enough
 * and nodeMCU has got too few pins to do interrupts on all 8 scan lines
 *   (and keep some for I/O)
 */

unsigned long lastMicros=0, isrMicros;

void isr1() { isr(0); }
void isr2() { isr(3); }
void isr3() { isr(4); }
void isr5() { isr(5); }
void isr6() { isr(6); }
void isr7() { isr(7); }


void isr(byte id)
{
  isrMicros = micros();
  if(isrMicros - lastMicros > 80)
  {
    digitalWrite(D0, LOW);
    digitalWrite(D0, HIGH);
    //Serial.print(id);
    if(id == 0)
    {
      delayMicroseconds(168);
      //Serial.print(1);
      digitalWrite(D0, LOW);
      digitalWrite(D0, HIGH);
      delayMicroseconds(168);
      //Serial.print(2);
      digitalWrite(D0, LOW);
      digitalWrite(D0, HIGH);
    }
    lastMicros = isrMicros;
  }
  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH); //LED is inverted
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
  attachInterrupt(D1, isr1, FALLING);
  attachInterrupt(D2, isr2, FALLING);
  attachInterrupt(D3, isr3, FALLING);
  attachInterrupt(D5, isr5, FALLING);
  attachInterrupt(D6, isr6, FALLING);
  attachInterrupt(D7, isr7, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
}
