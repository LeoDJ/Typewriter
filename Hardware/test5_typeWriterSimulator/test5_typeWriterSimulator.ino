#define lowTime 30
#define highTime 140
#define repeatTime 7800


void setup() {
  // put your setup code here, to run once:
  DDRD |= B11111100; //set pins 2-7 as outputs
  PORTD |= B11111100; //and high
  DDRB |= B00000011; //set pins 8&9 as outputs
  PORTB |= B00000011; //and high
}

void loop() {
  // put your main code here, to run repeatedly:
  PORTD &= B11111011;
  delayMicroseconds(lowTime);
  PORTD |= B11111100;
  delayMicroseconds(highTime);
  PORTD &= B11110111;
  delayMicroseconds(lowTime);
  PORTD |= B11111100;
  delayMicroseconds(highTime);
  PORTD &= B11101111;
  delayMicroseconds(lowTime);
  PORTD |= B11111100;
  delayMicroseconds(highTime);
  PORTD &= B11011111;
  delayMicroseconds(lowTime);
  PORTD |= B11111100;
  delayMicroseconds(highTime);
  PORTD &= B10111111;
  delayMicroseconds(lowTime);
  PORTD |= B11111100;
  delayMicroseconds(highTime);
  PORTD &= B01111111;
  delayMicroseconds(lowTime);
  PORTD |= B11111100;
  delayMicroseconds(highTime);
  PORTB &= B11111110;
  delayMicroseconds(lowTime);
  PORTB |= B00000011;
  delayMicroseconds(highTime);
  PORTB &= B11111101;
  delayMicroseconds(lowTime);
  PORTB |= B00000011;
  delayMicroseconds(repeatTime);
}
