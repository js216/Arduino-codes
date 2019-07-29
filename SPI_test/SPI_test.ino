 #include <SPI.h>

// length of AD9910 internal registers
const int reg_len[] = {4,4,4,4,4,6,6,4,2,4,4,8,8,4,8,8,8,8,8,8,8,8,4};

// SS = Select Device = pin 10
// SCK = Clock = pin 13
// MOSI = Data Output = pin 11
// MISO = Data Input = pin 12
const int CS_pin = 16;
const int IO_update_pin = 24;

unsigned int result[8];

void setup()
{
  SPI.begin();
  Serial.begin(9600);

  // CS is active-low
  pinMode(CS_pin, OUTPUT);
  digitalWrite(CS_pin, HIGH);

  pinMode(IO_update_pin, OUTPUT);
  digitalWrite(IO_update_pin, LOW);
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) { 
      case 'r':
        read_all_registers();
        break;

      case 'w':
        char reg = Serial.read();
        char data[reg_len[reg]];
        for (int i=0; i<reg_len[reg]; i++)
           data[i] = Serial.read();
        write_register(reg, data);
        break;
        
      // default settings: 85MHz output (for 1GHz clock)
      case 'd':
        char reg1 = 0x02;
        char data1[] = {B00011111, B00111111, B11000000, B00000000};
        write_register(reg1, data1);
        char reg2 = 0x0E;
        char data2[] = {B00001000, B10110101, B00000000, B00000000,
                       B00010101, B11000010, B10001111, B01011100};
        write_register(reg2, data2);
        break;
    }
  }
}

void loop()
{
}

void read_register(int reg)
{
  // register sanity check
  if (reg >= 23) {
    Serial.print("Invalid register!");
    return;
  }
  
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_pin, LOW);
  SPI.transfer(reg | B10000000);

  // read data
  for (int i=0; i<reg_len[reg]; i++)
    result[i] = SPI.transfer(0x00);

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}

void read_all_registers()
{
  Serial.print("Reading register contents:\n");
  for (int reg=0; reg<23; reg++) {
    Serial.print("  ");
    if (reg < 0x10)
      Serial.print(" ");
    Serial.print(reg, HEX);
    Serial.print(": ");
    read_register(reg);
    for (int i=0; i<reg_len[reg]; i++) {
//      Serial.print(result[i], BIN);
      printBits(result[i]);
      Serial.print(",");
    }
    Serial.print("\n");
  }
}

void write_register(char reg, char data[])
{
  // register sanity check
  if (reg >= 23) {
    Serial.print("Invalid register!\n");
    return;
  }

  // begin transaction
  Serial.print("Writing to register ");
  Serial.print(reg, HEX);
  Serial.print(":   \n  ");
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_pin, LOW);

  // transfer data
  SPI.transfer(reg & B01111111);
  for (int i=0; i<reg_len[reg]; i++) {
    printBits(data[i]);
    Serial.print(",");

    // do the SPI transfer
//    result[i] = SPI.transfer(data[i]);
    
    // transfer data from IO buffers to internal registers
    digitalWrite(IO_update_pin, HIGH);
    delay(10);
    digitalWrite(IO_update_pin, LOW);
  }
  Serial.print("\n");

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}

/* ============================
 * AUXILIARY FUNCTIONS
   ============================ */

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}
