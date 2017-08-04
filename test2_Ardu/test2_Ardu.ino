byte keys[8];
byte idx=0;
unsigned long lastMicros=0, isrMicros;

void isr()
{
  if(idx < 8)
  {
    byte lower = (PIND & B11100000) >> 5;
    byte upper = (PINB & B00011111) << 3;
    keys[idx] = upper | lower;
    ++idx;
  }
  isrMicros = micros();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), isr, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(micros() - isrMicros > 500 && idx > 7)
  {
    for(int i = 0; i < 8; i++)
    {
      Serial.write(keys[i]);
      //Serial.write(idx);
    }
    idx = 0;
  }
}
