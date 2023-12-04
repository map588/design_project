void setup() {
    // Serial.begin(9600);
    // while(!Serial && !Serial.available()){}

    pinMode(A1,  INPUT_PULLUP);
    pinMode(A0,  INPUT_PULLUP);
    pinMode(15,  INPUT_PULLUP);
    pinMode(14,  INPUT_PULLUP);
    pinMode(5,  OUTPUT);
    pinMode(6,  OUTPUT);
    pinMode(7,  OUTPUT);
    pinMode(8,  OUTPUT);
    pinMode(9,  OUTPUT);

    pinMode(2,  OUTPUT);
    pinMode(3,  OUTPUT);
    pinMode(17, OUTPUT);
    pinMode(30, OUTPUT); 

    digitalWrite(30, HIGH);

    PORTD |=  (0b11);
}

//  | 1 | 1 | 1 | 1 |
//  | 1 | 1 | 1 | 1 |
//  | 1 | 1 | 1 | 1 |
//  | 1 | 1 | 1 | 1 |
//  | 1 | 1 | 1 | 1 |



const int lookupTable[4][5] = {
    {  3,  2,  1,  0, -1 },   // Row 0 (Num Lock is -1 as it's not used in lookup)
    {  8,  7,  6,  5,  4 },  // Row 1
    { 13, 12, 11, 10,  9 },  // Row 2
    { 18, 17, 16, 15, 14 },       // Row 3   
};

const byte dataPin  =  2;
const byte clockPin =  3;
const byte RXLED    = 17;
const byte TXLED    = 30;

//columns and rows are switched because of the rotation
static inline void handleKeyPress(byte row, byte column) {

  static bool keyStates[4][5] = {false};
  static bool numLockActive = false;


    if(column == 4)
        return;

    if (!keyStates[column][row]) {
        keyStates[column][row] = true; // Mark key as pressed

        if (column == 0 && row == 4) {
            digitalWrite(RXLED, numLockActive); 
            numLockActive = !numLockActive; // Toggle Num Lock state
            return;
        }

        int8_t keyNumber = lookupTable[column][row];
        if (numLockActive) {
            keyNumber += 19; // Adjust if Num Lock is active
        }

        PORTD &= ~(0b10); //Port D:       1 = LOW  (dataPin)
        shiftOut(dataPin, clockPin, MSBFIRST, keyNumber);
        PORTD |=  (0b11); //Port D: 1 and 0 = HIGH (clockPin)

        // Serial.print("key: ");
        // Serial.println(keyNumber);
        return;
    }
    else{
      keyStates[column][row] = false;
      return; 
    }
}

void loop() {
  static byte lastStates[5] = {0b1111, 0b1111, 0b1111, 0b1111, 0b1111}; // Last state of each row
  byte currentStates[5] = {0b1111, 0b1111, 0b1111, 0b1111, 0b1111}; // Current state of each row
  static byte row;
  static byte column; 
  for (int i = 0; i < 5; i++) {
    // Activate the row - Direct Port Manipulation
    switch (i) {
      case 0: PORTC &= ~(1 << PC6); break; // Row 5 -> PC6
      case 1: PORTD &= ~(1 << PD7); break; // Row 6 -> PD7
      case 2: PORTE &= ~(1 << PE6); break; // Row 7 -> PE6
      case 3: PORTB &= ~(1 << PB4); break; // Row 8 -> PB4
      case 4: PORTB &= ~(1 << PB5); break; // Row 9 -> PB5
    }
    delayMicroseconds(10);
    // Read columns - Direct PIN Register Reading
    byte columnState = 0;
    columnState |= (PINF & (1 << PF6)) ? 0 : 1;  // A1 -> PF6
    columnState |= (PINF & (1 << PF7)) ? 0 : 2;  // A0 -> PF7
    columnState |= (PINB & (1 << PB1)) ? 0 : 4;  // 15 -> PB1
    columnState |= (PINB & (1 << PB3)) ? 0 : 8;  // 14 -> PB3
    currentStates[i] = columnState;

    // Deactivate the row - Direct Port Manipulation
    switch (i) {
      case 0: PORTC |= (1 << PC6); break;
      case 1: PORTD |= (1 << PD7); break;
      case 2: PORTE |= (1 << PE6); break;
      case 3: PORTB |= (1 << PB4); break;
      case 4: PORTB |= (1 << PB5); break;
    }


    // Compare current state with the last state
    if (currentStates[i] != lastStates[i]) {
      byte changes = currentStates[i] ^ lastStates[i]; // XOR to find changes
          row = i;
          switch(changes){
            case 1:  column = 0; break;
            case 2:  column = 1; break;
            case 4:  column = 2; break;
            case 8:  column = 3; break;
            default: column = 4; break;
          }

          handleKeyPress(row, column);



      //  byte print = changes | 0b10000;
      //  Serial.print(i);
      //  Serial.println(print, BIN);
       lastStates[i] = currentStates[i]; // Update the last state
   }
  }

  delayMicroseconds(500);
}


