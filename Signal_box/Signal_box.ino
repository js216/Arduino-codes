// general AD9910 pins
const int IOUP = 0;
const int RST[] = {1,2,3,4,5};
const int CSB[] = {34,33,32,21,30};

// frequency counters output bits and input clock
const int c_bits[][3] = {
  {8,7,6},
  {26,25,24},
  {29,28,27},
  {14,15,16},
  {35,36,37}
};
const int c_clk[] = {22,23,39,38,9};

// thermometer data
const int t_data[] = {21,20,19,18,17};


///////////////////////////////////////////////
////    INTERRUPTS                  ///////////
///////////////////////////////////////////////

volatile char c1_state,c2_state,c3_state,c4_state,c5_state;

void incr_c1()
{
  // increment counter
  c1_state = (c1_state+1)%8;
  Serial.println(c1_state, DEC);

  // split counter into three bits
  for (int j=0; j<3; j++) {
   if ( (0x04>>j) & c1_state )
       digitalWrite (c_bits[0][2-j],HIGH);
   else
       digitalWrite (c_bits[0][2-j],LOW);
 }
}

void incr_c2(){}
void incr_c3(){}
void incr_c4(){}
void incr_c5(){}

///////////////////////////////////////////////
////    DEVICE INITIALIZATION       ///////////
///////////////////////////////////////////////

void setup()
{
  Serial.begin(9600);

  // set pin directions
  pinMode (IOUP,OUTPUT);
  for (int i=0; i<5; i++) {
    pinMode (RST[i],OUTPUT);
    pinMode (CSB[i],OUTPUT);
    for (int j=0; j<3; j++)
      pinMode (c_bits[i][j],OUTPUT);
  }

  // set interrupt pins
  for (int i=0; i<5; i++)
    pinMode (c_clk[i],INPUT);
  attachInterrupt(digitalPinToInterrupt(c_clk[0]), incr_c1, RISING);
  attachInterrupt(digitalPinToInterrupt(c_clk[1]), incr_c2, RISING);
  attachInterrupt(digitalPinToInterrupt(c_clk[2]), incr_c3, RISING);
  attachInterrupt(digitalPinToInterrupt(c_clk[3]), incr_c4, RISING);
  attachInterrupt(digitalPinToInterrupt(c_clk[4]), incr_c5, RISING);

  // set default pin values
  digitalWrite (IOUP, LOW);
  for (int i=0; i<5; i++) {
    digitalWrite (RST[i], LOW);
    digitalWrite (CSB[i], HIGH);
    for (int j=0; j<3; j++)
      digitalWrite (c_bits[i][j],LOW);
  }
}

///////////////////////////////////////////////
////    SERIAL COMMUNICATION        ///////////
///////////////////////////////////////////////

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'e':
        break;
    }
  }
}

///////////////////////////////////////////////
////    MAIN DEVICE LOOP            ///////////
///////////////////////////////////////////////

void loop()
{
}
