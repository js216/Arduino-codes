  byte PORT_B[] = {16,17,19,18,0,1,32,25};
byte PORT_C[] = {15,22,23,9,10,13,11,12,28,27,29,30};
byte PORT_D[] = {2,14,7,8,6,20,21,5};

// PID parameters
const int setpoint = 300;
const int divisor = 2;
const int KP = 1;
const int KI = 0;
const int KD = 0;

// PID control variables
int val, error, P, I, D;
int out = 0;

// loop variable to moderate serial printing
int i = 0;

void setup()
{
  // set pins to output
  for (int i=0; i<2; i++)
    pinMode(PORT_B[i],OUTPUT);
  for (int i=0; i<8; i++)
    pinMode(PORT_C[i],OUTPUT);
  for (int i=0; i<8; i++)
    pinMode(PORT_D[i],OUTPUT);

  // analog input  
  pinMode(14, INPUT);

  // select AD9910 amplitude modulation mode
  GPIOB_PDOR = B11111100;

  // initialize output to off
  GPIOC_PDOR = 0;
}

void loop()
{
  while (1) {
    // calculate error
    val = analogRead(14);
    error = setpoint - val;

    // calculate PID control values
    P = (KP * error)/divisor;

    // not yet implemented
    I = 0;
    D = 0;

    // update DAC output
    out += P + I + D;
    GPIOC_PDOR = out;

    // serial printing
    if (i > 32768)
      i = 0;
    if (i == 0) {
      Serial.print("val=");
      Serial.print(val);
      Serial.print(", error=");
      Serial.print(error);
      Serial.print(", P = ");
      Serial.println(P);
    }
    i++;
  }
}
