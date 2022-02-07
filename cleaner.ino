#define interruptPin 2 //PIN_PD2
#define outPin 5 // PIN_PD5
#define debugPin 6 // PIN_PD6
#define debounceMillis 20
#define maxTime 15    // Maksymalny czas pracy w okresach timera (ok 1 s) po którym praca zostanie zakończona (ze względu na nagrewanie się przzetwornika)

bool is_on = false;   // Jeżeli pracuje True
unsigned long last_time = 0;  // Do denbouncingu
int counter = 0;      // Licznik okresów timera

void setup() {
  pinMode(outPin, OUTPUT);
  digitalWrite(outPin, HIGH);

  pinMode(debugPin, OUTPUT);
  
  noInterrupts();   // Wyłączeniue przerwań cli()
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), INT, CHANGE);  // Inicjalizacja przerwania zewnętrznego
  interrupts();   // Włączenie przerwań sei();
}

void loop() {
//  digitalWrite(debugPin, HIGH);
  if (is_on) {
    digitalWrite(debugPin, HIGH);
  } else {
    digitalWrite(debugPin, LOW);
  }
}

void set_timer() {
  if (TIMSK1 & ((1 << OCIE1A)|(1 << TOIE1))) {
    return;
  }
  noInterrupts();
  is_on = true;
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 1;     // Zerowanie licznika timera 1
  
  TCCR1B |= (1 << CS12) | (1 << CS10);    // 1024 prescaler - ok. 1 s
//  TCCR1B |= (1 << CS11) | (1 << CS10);    // 64 prescaler 
//  TCCR1B |= (1 << CS11);    // 8 prescaler
  
  TIMSK1 |= (1 << TOIE1);  // Przerwanie po przepełnieniu licznika
  TIFR1 &= ~(1 << TOV1); 
  counter = 0;
  interrupts();
}

void deactivate_timer() {
  TIMSK1 &= ~(1 << TOIE1); // Wyłączenie przerwań timera
  is_on = false;
}

ISR(TIMER1_OVF_vect)
{
  counter++;
  if (counter > 1) {
    
    digitalWrite(outPin, LOW);
    delay(500);
    digitalWrite(outPin, HIGH);

    if (counter > maxTime) {
      deactivate_timer();
    } else {
      delay(1000);
    
      digitalWrite(outPin, LOW);
      delay(500);
      digitalWrite(outPin, HIGH);
    }
  }
}

void INT() {
  delay(10);
  if (millis() - last_time < debounceMillis) return;
  last_time = millis();
  if(digitalRead(interruptPin) == LOW)
  {
    digitalWrite(outPin, LOW);
    delay(500);
    digitalWrite(outPin, HIGH);
    if (!is_on) {
      set_timer();
    } else {
      deactivate_timer();
    }
  }
}
