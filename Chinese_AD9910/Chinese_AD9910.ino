 #include <SPI.h>

// length of AD9910 internal registers
const int reg_len[] = {4,4,4,4,4,6,6,4,2,4,4,8,8,4,8,8,8,8,8,8,8,8,4};

// SCK = Clock = pin 13
// MOSI = Data Output = pin 11
// MISO = Data Input = pin 12
const int CS_pin = 33;
const int IO_update_pin = 34;
const int reset_pin = 35;

unsigned int result[8];

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}

char demandSerialByte(const int N = 1000)
{
  for (int i=0; i<N; ++i) {
    if (Serial.available() > 0)
      return Serial.read();
    else
      delay(1);
  }

  return 0;
}

void setup()
{
  // for AD9910 communication
  SPI.begin();

  // for computer communication
  Serial.begin(9600);

  // for FPGA communication
  Serial1.setTX(1);
  Serial1.setRX(0);
  Serial1.begin(115200, SERIAL_8N1);

  // CS is active-low
  pinMode(CS_pin, OUTPUT);
  digitalWrite(CS_pin, LOW);

  pinMode(IO_update_pin, OUTPUT);
  digitalWrite(IO_update_pin, LOW);

  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, LOW);
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {

      ///////////////////////////////////////////////////////////
      /////////             COMMANDS FOR FPGA             ///////
      ///////////////////////////////////////////////////////////
      
      // "get" commands
      case 't':
      case 'f':
      case 'n':
      case 'p':
      case 'i':
      case 'd':
      case 's':
      case 'l':
      case 'a':
      case 'e':
      case 'o':
        Serial1.print(cmd);
        break;
        
      // "set" commands
      case 'T':
      case 'F':
      case 'N':
        Serial1.print(cmd);
        Serial1.print( demandSerialByte() );
        break;
      case 'P':
      case 'I':
      case 'S':
      case 'L':
        Serial1.print(cmd);
        Serial1.print( demandSerialByte() );
        Serial1.print( demandSerialByte() );
        break;
        
      ///////////////////////////////////////////////////////////
      /////////             COMMANDS FOR AD9910           ///////
      ///////////////////////////////////////////////////////////
      
      case '1':
        read_one_register( demandSerialByte() );
        break;
        
      case 'r':
        read_all_registers();
        break;

      case '0':
        digitalWrite(reset_pin, HIGH);
        delay(100);
        digitalWrite(reset_pin, LOW);
        Serial.print("Device reset complete.\n");
        break;

      case 'w':
        char reg = Serial.read();
        char data[reg_len[reg]];
        for (int i=0; i<reg_len[reg]; i++)
           data[i] = Serial.read();
        write_register(reg, data);
        break;
    }
  }
}

// echo FPGA serial communications to USB
void serialEvent1()
{
  while (Serial1.available() > 0) {
    char data = Serial1.read();
    Serial.println(data, HEX);
  }
}

void loop()
{
}

void read_register(int reg)
{
  // register sanity check
  if (reg >= 23) {
    Serial.print("Invalid register: ");
    Serial.print(reg, HEX);
    Serial.print("\n");
    return;
  }
  
  // begin transaction
  SPI.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0));

  // flash the IO_RESET (as well as the CS) to clear potential unfinished transactions
  digitalWrite(CS_pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(CS_pin, LOW);
  
  SPI.transfer(reg | B10000000);

  // read data
  for (int i=0; i<reg_len[reg]; i++)
    result[i] = SPI.transfer(0x00);

  // end transaction
  SPI.endTransaction();
}

void read_one_register(int reg)
{
  // Read out the contents of the register
  Serial.print("Reading from register ");
  Serial.print(reg, HEX);
  Serial.print(":   \n  ");
  read_register(reg);
  for (int i=0; i<reg_len[reg]; i++) {
    printBits(result[i]);
    Serial.print(",");
  }
  Serial.print("\n");
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
    Serial.print("Invalid register: ");
    Serial.print(reg, HEX);
    Serial.print("\n");
    return;
  }

  // Read out the contents of the register
  Serial.print("Reading from register ");
  Serial.print(reg, HEX);
  Serial.print(":   \n  ");
  read_register(reg);
  for (int i=0; i<reg_len[reg]; i++) {
    printBits(result[i]);
    Serial.print(",");
  }
  Serial.print("\n");

  // begin transaction
  Serial.print("Writing to register ");
  Serial.print(reg, HEX);
  Serial.print(":   \n  ");
  SPI.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0));

  // flash the IO_RESET (as well as the CS) to clear potential unfinished transactions
  digitalWrite(CS_pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(CS_pin, LOW);

  // transfer data
  SPI.transfer(reg & B01111111);
  for (int i=0; i<reg_len[reg]; i++) {
    printBits(data[i]);
    Serial.print(",");

    // do the SPI transfer
    result[i] = SPI.transfer(data[i]);
    
    // transfer data from IO buffers to internal registers
    digitalWrite(IO_update_pin, HIGH);
    delayMicroseconds(5);
    digitalWrite(IO_update_pin, LOW);
  }
  Serial.print("\n");

  // end transaction
  SPI.endTransaction();

  // Read out the contents of the register
  Serial.print("Reading from register ");
  Serial.print(reg, HEX);
  Serial.print(":   \n  ");
  read_register(reg);
  for (int i=0; i<reg_len[reg]; i++) {
    printBits(result[i]);
    Serial.print(",");
  }
  Serial.print("\n");
}
