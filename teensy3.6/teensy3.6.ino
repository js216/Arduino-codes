// output test:
byte tableB[] = {16,17,19,18,0,1,32,25};
byte tableC[] = {15,22,23,9,10,13,11,12,28,27,29,30};
byte tableD[] = {2,14,7,8,6,20,21,5};

void setup()
{
  // set pins to output
  for (int i=0; i<2; i++)
    pinMode(tableB[i],OUTPUT);
  for (int i=0; i<8; i++)
    pinMode(tableC[i],OUTPUT);
  for (int i=0; i<8; i++)
    pinMode(tableD[i],OUTPUT);

  // select AD9910 frequency modulation mode
  GPIOB_PDOR = B11111100;
}

void loop()
{
  while (1) {
    for (int i=0; i<256; ++i) {
      GPIOD_PDOR = i;
      for (int j=0; j<256; ++j) {
        GPIOC_PDOR = j;
        delay(10);
      }
    }
  }
}
