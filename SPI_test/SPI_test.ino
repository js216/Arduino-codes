 #include <SPI.h>

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
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) { 
      // read all registers
      case 'r':
        read_all_registers();
        break;

      // write register
      case 'w':
        // determine what register to write to
        char reg = Serial.read() - 48;
        Serial.print(reg_len[reg]);
        return;

        // read data to be written to the register
        char data[reg_len[reg]];
        for (int i=0; i<reg_len[reg]; i++)
          data[i] = Serial.read();

        // write to SPI
        write_register(reg, data);
        break;
    }
  }
}

void loop()
{
}

void read_register(char reg)
{
  // register sanity check
  if (reg >= 23) {
    Serial.print("Invalid register!");
    return -1;
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
  Serial.print("Writing to register ");
  Serial.print(reg, HEX);
  Serial.print(":   \n");
  if (reg >= 23) {
    Serial.print("Invalid register!\n");
    return -1;
  }

  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_pin, LOW);

  // transfer data
  SPI.transfer(reg | B00000000);
  for (int i=0; i<reg_len[i]; i++) {
    Serial.print(data[i], BIN);
    Serial.print(",");
//    result[i] = SPI.transfer(data[i]);
  }
  Serial.print("\n");

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}
