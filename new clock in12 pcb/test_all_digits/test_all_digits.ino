
// выводы для дешифратора
  int out1 = A3;
  int out2 = A1;
  int out4 = A0;
  int out8 = A2;
// выводы для транзисторных ключей
  int key1= 3;
  int key2 = 4;
  int key3 = 5;
  int key4 = 6;
 unsigned long nextDigitTime = 500;
 unsigned long lastTime = 0;
  int curDigit = 2;

void setup() {
  // put your setup code here, to run once:
  // задаем частоту ШИМ на 9 выводе 30кГц
  TCCR1B=TCCR1B&0b11111000|0x01;
  analogWrite(9,190);

  //задаем режим работы выходов микроконтроллера
  pinMode(out1,OUTPUT);
  pinMode(out2,OUTPUT);
  pinMode(out4,OUTPUT);
  pinMode(out8,OUTPUT);
 
  pinMode(key1,OUTPUT);
  pinMode(key2,OUTPUT);
  pinMode(key3,OUTPUT);
  pinMode(key4,OUTPUT);
}
 
 
void loop() {
  if (millis()-lastTime> nextDigitTime){
   lastTime = millis();
   curDigit++;
   if (curDigit == 10){
     curDigit = 0;  
   }
  }
  int digits[3]; // массив для текущего значения времени на четыре цифры
  digits[0] = curDigit;
  digits[1] = curDigit;
  digits[2] = curDigit;
  digits[3] = curDigit;
  show(digits); // вывести цифры на дисплей
}
void show(int a[])
{
  //выведем цифру a[0] на первый индикатор
  setNumber(a[0]);
  
  digitalWrite(key1,HIGH);
  delay(3);
  //потушим первый индикатор
  digitalWrite(key1,LOW);
  delay(1);
  
  //цифра a[1] на второй индикатор
  setNumber(a[1]);
  digitalWrite(key2,HIGH);
  delay(3);
  //потушим второй индикатор
  digitalWrite(key2,LOW);
  delay(1);
  
  //цифра a[2] на третий индикатор
  setNumber(a[2]);
  digitalWrite(key3,HIGH);
  delay(3);
  //потушим третий индикатор
  digitalWrite(key3,LOW);
  delay(1);

  //выведем цифру a[3] на четвертый индикатор
  setNumber(a[3]);
  digitalWrite(key4,HIGH);
  delay(3);
  //потушим четвертый индикатор
  digitalWrite(key4,LOW);
  delay(1);
}
// передача цифры на дешифратор
void setNumber(int num) 
{
  switch (num)
  {
    case 0:
    digitalWrite (out1,HIGH);
    digitalWrite (out2,HIGH);
    digitalWrite (out4,HIGH);
    digitalWrite (out8,LOW);
    break;
    case 1:
    digitalWrite (out1,HIGH);
    digitalWrite (out2,HIGH);
    digitalWrite (out4,LOW);
    digitalWrite (out8,LOW);
    break;
    case 2:
        digitalWrite (out1,LOW);
    digitalWrite (out2,HIGH);
    digitalWrite (out4,HIGH);
    digitalWrite (out8,LOW);
    break;
    case 3:
    digitalWrite (out1,LOW);
    digitalWrite (out2,LOW);
    digitalWrite (out4,HIGH);
    digitalWrite (out8,LOW);
    break;
    case 4:
    digitalWrite (out1,HIGH);
    digitalWrite (out2,LOW);
    digitalWrite (out4,LOW);
    digitalWrite (out8,LOW);
    break;
    case 5:
        digitalWrite (out1,HIGH);
    digitalWrite (out2,LOW);
    digitalWrite (out4,LOW);
    digitalWrite (out8,HIGH);

    break;
    case 6:
        digitalWrite (out1,LOW);
    digitalWrite (out2,LOW);
    digitalWrite (out4,LOW);
    digitalWrite (out8,HIGH);
    break;
    case 7:
    digitalWrite (out1,LOW);
    digitalWrite (out2,LOW);
    digitalWrite (out4,LOW);
    digitalWrite (out8,LOW);
    break;
    case 8:
            digitalWrite (out1,HIGH);
    digitalWrite (out2,LOW);
    digitalWrite (out4,HIGH);
    digitalWrite (out8,LOW);
    break;
    case 9:
    digitalWrite (out1,LOW);
    digitalWrite (out2,HIGH);
    digitalWrite (out4,LOW);
    digitalWrite (out8,LOW);
    break;
  }
}

 
