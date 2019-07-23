// output test:
byte tableB[] = {16,17,19,18,0,1,32,25};
byte tableC[] = {15,22,23,9,10,13,11,12,28,27,29,30};
byte tableD[] = {2,14,7,8,6,20,21,5};

void setup()
{
  for (int i=0; i<2; i++)
    pinMode(tableB[i],OUTPUT);
  for (int i=0; i<8; i++)
    pinMode(tableC[i],OUTPUT);
  for (int i=0; i<8; i++)
    pinMode(tableD[i],OUTPUT);
}

void loop()
{
  while (1==1) {
    for (int i=0; i<=8; i++) {
      GPIOB_PDOR = 1<<i;
      GPIOC_PDOR = 1<<i;
      GPIOD_PDOR = 1<<i;
      delay(500);
    }
  }
}
