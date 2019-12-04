class TB6600
{
  public:
    TB6600(const int ENA_n, const int ENA_p, const int DIR_n, const int DIR_p, const int PUL_n, const int PUL_p)
      : X_ENgnd(ENA_n), X_EN_5v(ENA_p), X_DIRgnd(DIR_n), X_DIR_5v(DIR_p), X_STPgnd(PUL_n), X_STP_5v(PUL_p)
    {
    }

    void begin()
    {
      pinMode (X_ENgnd ,OUTPUT);
      pinMode (X_EN_5v ,OUTPUT);
      pinMode (X_DIRgnd,OUTPUT);
      pinMode (X_DIR_5v,OUTPUT);
      pinMode (X_STPgnd,OUTPUT);
      pinMode (X_STP_5v,OUTPUT);
      digitalWrite (X_ENgnd,  LOW);
      digitalWrite (X_EN_5v, HIGH);
      digitalWrite (X_DIRgnd, LOW);
      digitalWrite (X_DIR_5v, LOW);
      digitalWrite (X_STPgnd, LOW);
      digitalWrite (X_STP_5v, LOW);
    }

    void enable(const bool ENABLE)
    {
      if (ENABLE)
        digitalWrite (X_EN_5v, LOW);
      else
        digitalWrite (X_EN_5v, HIGH);
    }
    
    void set_dt(const int dt){
     this->dt = dt;
    }
    
    void move(const int NX){
      int xt;
      if (NX>X) {
        xt = NX-X;
        digitalWrite (X_DIR_5v,LOW);
        xt=1;
      } else {
        xt = X-NX;
        digitalWrite (X_DIR_5v,HIGH);
        xt=-1;
      }
    
      for (; X !=NX; X=X+xt) {
        digitalWrite (X_STP_5v, HIGH);
        delayMicroseconds (dt);
        digitalWrite (X_STP_5v, LOW);
        delayMicroseconds (dt);
      }
    }

  private:
    long X = 0;    // present position
    int dt = 400; // step duration

    const int X_ENgnd, X_EN_5v, X_DIRgnd, X_DIR_5v, X_STPgnd, X_STP_5v;
};




TB6600 motor_x = TB6600(2,3,4,5,6,7);
TB6600 motor_y = TB6600(8,9,10,11,12,13);

void setup()
{
  Serial.begin(9600);

  motor_x.begin();
  motor_y.begin();
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'e':
        motor_x.enable(Serial.parseInt());
        break;

      case 's':
        motor_x.set_dt(Serial.parseInt());
        break;

      case 'm':
        motor_x.move(Serial.parseInt());
        break;

      case 'E':
        motor_y.enable(Serial.parseInt());
        break;

      case 'S':
        motor_y.set_dt(Serial.parseInt());
        break;

      case 'M':
        motor_y.move(Serial.parseInt());
        break;
    }
  }
}

void loop()
{
}
