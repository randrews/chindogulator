#include <Keyboard.h>
//#define DEBUG

const int COLS[5] = {3, 2, 0, 1, 4};
const int ROWS[5] = {12, 6, 8, 9, 10};

class Column {
  private:
  int currentState; // Bitfield for the current button state (1 for pressed)
  int oldState; // The last state (for edge detection)
  int fifo[3]; // A queue of row-pin-states (active low like the pins,
               // but just used for debouncing
  public:
  Column();
  int pin; // Which pin this column has in common
  void scan(); // Update button states
  const int edges(); // Return a bitfield of which buttons in this column
                     // are newly-pressed this tick
};

Column cols[5];

class Calc {
  private:
  String x,y,z,t; // The current register contents
  String previousX;
  bool neg, dec; // Whether the current X register is negative / has a dec point
  bool soft; // If 'soft' then pressing any digit replaces X, rather than appends to it
  bool pushsoft; // 'pushsoft' pushes a new cell with the new digit
  void afterKey();
  public:
  Calc();
  void pressDigit(const int digit);
  void pressEnter();
  void pressDecimal();
  void pressDrop();
  void pressDel();
  void pressPosNeg();
  void pressOp(const char op);
  void pressTrig(const char fn);
  void pressStart();
  void printDebug() const;
};

Calc calc;

void setup() {
  // All column pins are active-low outputs,
  // all row pins are inputs with pullups
  for(int n = 0; n < 5; n++) {
    pinMode(COLS[n], OUTPUT);
    pinMode(ROWS[n], INPUT);
    digitalWrite(COLS[n], 1);
    digitalWrite(ROWS[n], 1);
    cols[n].pin = COLS[n];
  }

  // Debug output over serial
  #ifdef DEBUG
  Serial.begin(9600);
  while(!Serial);
  #endif

  Keyboard.begin();
}

void loop() {
  for(int n = 0; n < 5; n++) {
    cols[n].scan();
  }

  int col0 = cols[0].edges();
  if(col0 & 1) calc.pressTrig('l');
  if(col0 & 2) calc.pressTrig('t');
  if(col0 & 4) calc.pressTrig('c');
  if(col0 & 8) calc.pressTrig('s');
  if(col0 & 16) calc.pressStart();

  int col1 = cols[1].edges();
  if(col1 & 1) calc.pressDigit(0);
  if(col1 & 2) calc.pressDigit(1);
  if(col1 & 4) calc.pressDigit(4);
  if(col1 & 8) calc.pressDigit(7);
  if(col1 & 16) calc.pressDrop();

  int col2 = cols[2].edges();
  if(col2 & 1) calc.pressDecimal();
  if(col2 & 2) calc.pressDigit(2);
  if(col2 & 4) calc.pressDigit(5);
  if(col2 & 8) calc.pressDigit(8);
  if(col2 & 16) calc.pressDel();

  int col3 = cols[3].edges();
  if(col3 & 1) calc.pressPosNeg();
  if(col3 & 2) calc.pressDigit(3);
  if(col3 & 4) calc.pressDigit(6);
  if(col3 & 8) calc.pressDigit(9);
  if(col3 & 16) calc.pressOp('^');

  int col4 = cols[4].edges();
  if(col4 & 1) calc.pressEnter();
  if(col4 & 2) calc.pressOp('+');
  if(col4 & 4) calc.pressOp('-');
  if(col4 & 8) calc.pressOp('*');
  if(col4 & 16) calc.pressOp('/');
}

Column::Column() {
  currentState = 0;
  oldState = currentState;
  fifo[0] = fifo[1] = fifo[2] = 0b00011111;
}

void Column::scan() {
  fifo[2] = fifo[1];
  fifo[1] = fifo[0];
  fifo[0] = 0;
  digitalWrite(pin, 0);
  for(int n = 0; n < 5; n++) {
    if (digitalRead(ROWS[n])) fifo[0] |= (1 << n);
  }
  digitalWrite(pin, 1);
  oldState = currentState;
  currentState = 31 - (fifo[0] | fifo[1] | fifo[2]);
}

const int Column::edges() {
  return currentState & ~oldState;
}

Calc::Calc() {
  x = "0"; y = "0"; z = "0"; t = "0";
  neg = dec = false;
  soft = true;
}

void Calc::pressStart() {
  x = "0"; y = "0"; z = "0"; t = "0";
  neg = dec = false;
  soft = true;
  previousX = "";
  afterKey();
}

void Calc::printDebug() const {
  #ifdef DEBUG
  Serial.println("--------------------");
  Serial.println("t:\t" + t);
  Serial.println("z:\t" + z);
  Serial.println("y:\t" + y);
  Serial.println("x:\t" + x);
  #endif
}

void Calc::pressDigit(const int digit) {
  if (pushsoft) {
    t = z; z = y; y = x;
    x = digit;
    neg = dec = false;
    soft = (digit == 0);
    pushsoft = false;    
  } else if (soft) {
    x = digit;
    neg = dec = false;
    soft = (digit == 0);
  } else {
    x = x + digit;
    soft = false;
  }
  afterKey();
}

void Calc::pressEnter() {
  t = z;
  z = y;
  y = x;
  soft = true;
  
  afterKey();
}

void Calc::pressDecimal() {
  if (pushsoft) {
    t = z; z = y; y = x;
    dec = true;
    soft = pushsoft = false;
    x = "0.";    
  } else if (soft) {
    dec = true;
    soft = false;
    x = "0.";
  } else if (!dec) {
    dec = true;
    x = x + ".";
  }

  afterKey();
}

void Calc::pressDrop() {
  x = y; y = z; z = t; t = "0";
  soft = true;
  afterKey();
}

void Calc::pressDel() {
  if (!soft) {
    if (neg && x.length() == 2 || !neg && x.length() == 1) {
      x = "0";
      soft = true;
    } else {
      if (x.endsWith(".")) { dec = false; }
      x = x.substring(0, x.length()-1);
    }
  } else {
    x = "0";
    neg = dec = false;
  }
  afterKey();
}

void Calc::pressPosNeg() {
  if (neg) {
    x = x.substring(1);
  } else {
    x = "-" + x;
  }
  neg = !neg;
  afterKey();
}

void Calc::pressOp(const char op) {
  double xd = x.toDouble();
  double yd = y.toDouble();
  double result = 0;
  switch (op) {
    case '+': result = yd + xd; break;
    case '-': result = yd - xd; break;
    case '*': result = yd * xd; break;
    case '/': result = yd / xd; break;
    case '^': result = pow(yd, xd); break;
  }
  x = result;
  y = z;
  z = t;
  t = "0";
  pushsoft = true;
  afterKey();
}

void Calc::pressTrig(const char fn) {
  double xd = x.toDouble();
  double result = 0;
  switch (fn) {
    case 's': result = sin(xd); break;
    case 'c': result = cos(xd); break;
    case 't': result = tan(xd); break;
    case 'l': result = log(xd); break;
  }
  x = result;
  pushsoft = true;
  afterKey();
}

void Calc::afterKey() {
  printDebug();
  for (int n = 0; n < previousX.length(); n++) {
    Keyboard.write(KEY_BACKSPACE);
  }
  Keyboard.print(x);
  previousX = x;
}
