int flag;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("SM_S\r\n");
  flag = 0;
  randomSeed(analogRead(0));
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 120 && flag == 0; i++)
  {
    Serial.print(120-i, HEX);
    Serial.print("\r\n");
    delay(100);

    if (i == 119)
    {
      flag = 1;
    }
  }
  
  if (flag == 1)
  {
    Serial.print("SM_R\r\n");

    delay(5000);

    Serial.print("SM_S\r\n");

    delay(3000);

    Serial.print("SM_R\r\n");

    flag = 2;

    delay(2000);
  }

  if (flag == 2)
  {
    int rnum = random(0, 3);
    switch(rnum)
    {
    case 0: Serial.print("INT_MPU\r\n"); break;
    case 1: Serial.print("INT_CON\r\n"); break;
    case 2: Serial.print("INT_FCU\r\n"); break;
    }
    flag = 3;
  }
}
