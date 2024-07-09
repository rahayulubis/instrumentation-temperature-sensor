#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define ONE_WIRE_BUS 2
#define KIPAS 4
#define PEMANAS 5
#define pinA 8    //D0  kipas
#define pinB 6    //D1  pemanas
#define pinC 9    //D2
#define pinD 7    //D3
#define led1 A0   //auto manual
#define led2 A1   //kipas
#define led3 A2   //pemanas

SoftwareSerial mySerial(10, 11); // RX, TX
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 20, 4);

float suhuDS1820;
String Status;
String statusKipas = "OFF";
String statusPemanas = "OFF";

int modes = 0;
int tempA[3] = {0, 0, 0};
int tempB[3] = {0, 0, 0};

unsigned long m;

String buff_ser;
String command1On = "A1";     //manual
String command1Off = "a1";    //auto
String command2On = "A2";     //fan on
String command2Off = "a2";    //fan off
String command3On = "A3";     //heater on
String command3Off = "a3";    //heater off

void setup(void)
{
  Serial.begin(9600);
  mySerial.begin(9600);
  sensors.begin();
  lcd.init();
  
  pinMode(KIPAS, OUTPUT);
  pinMode(PEMANAS, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(pinC, INPUT);
  pinMode(pinD, INPUT);

  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(KIPAS, HIGH);
  digitalWrite(PEMANAS, HIGH);
  lcd.backlight();

  modes = 0;
}

void loop(void) {
  if (mySerial.available()) {
    char temp = char(mySerial.read());
    buff_ser += temp;
    Serial.print(temp);
  }

  if (buff_ser.length() > 16) {
    buff_ser = "";
  }

  if (buff_ser.indexOf(command1On) > -1) {
    Serial.println(command1On);
    mySerial.print(command1On);
    modes = 1;
    buff_ser = "";
  }

  if (buff_ser.indexOf(command1Off) > -1) {
    Serial.println(command1Off);
    mySerial.print(command1Off);
    modes = 0;
    buff_ser = "";
  }

  if (buff_ser.indexOf(command2On) > -1) {
    if (modes == 1) {
      Serial.println(command2On);
      mySerial.print(command2On);
      tempA[2] = HIGH;
      fan();
    }
    buff_ser = "";
  }

  if (buff_ser.indexOf(command2Off) > -1) {
    if (modes == 1) {
      Serial.println(command2Off);
      mySerial.print(command2Off);
      tempA[2] = LOW;
      fan();
    }
    buff_ser = "";
  }

  if (buff_ser.indexOf(command3On) > -1) {
    if (modes == 1) {
      Serial.println(command3On);
      mySerial.print(command3On);
      tempB[2] = HIGH;
      heater();
    }
    buff_ser = "";
  }

  if (buff_ser.indexOf(command3Off) > -1) {
    if (modes == 1) {
      Serial.println(command3Off);
      mySerial.print(command3Off);
      tempB[2] = LOW;
      heater();
    }
    buff_ser = "";
  }
  
  sensors.requestTemperatures();
  suhuDS1820 = sensors.getTempCByIndex(0);
  Serial.print("Suhu: ");
  Serial.print(suhuDS1820);
  Serial.print(" °C\n");

  tempA[0] = digitalRead(pinA); //Kipas
  tempB[0] = digitalRead(pinB); //Pemanas

  if (digitalRead(pinC) == HIGH) {
    delay(200);
    modes = 0;
  }

  if (digitalRead(pinD) == HIGH) {
    delay(200);
    modes = 1;
  }

  if (modes == 0) {
    digitalWrite (led1, LOW);

    if (suhuDS1820 > 30.00) {
      digitalWrite (KIPAS, LOW);
      digitalWrite (led2, LOW);
      digitalWrite (PEMANAS, HIGH);
      digitalWrite (led3, HIGH);
      Status = "Panas";
      statusKipas = "ON";
      statusPemanas = "OFF";
    }

    if (suhuDS1820 < 28.00) {
      digitalWrite (PEMANAS, LOW);
      digitalWrite (led3, LOW);
      digitalWrite (KIPAS, HIGH);
      digitalWrite (led2, HIGH);
      Status = "Dingin";
      statusKipas = "OFF";
      statusPemanas = "ON";
    }

    if (suhuDS1820 >= 28.00 && suhuDS1820 <= 30.00) {
      digitalWrite (KIPAS, HIGH);
      digitalWrite (led2, HIGH);
      digitalWrite (PEMANAS, HIGH);
      digitalWrite (led3, HIGH);
      Status = "Normal";
      statusKipas = "OFF";
      statusPemanas =  "OFF";
    }
  }

  if (modes == 1) {
    digitalWrite (led1, HIGH);

    if (tempA[0] != tempA[1]) {
      delay(200);
      if (digitalRead(pinA) == tempA[0]) {
        tempA[1] = tempA[0];

        if (tempA[1] == HIGH) {
          if (tempA[2] == LOW) {
            tempA[2] = HIGH;
          } else {
            tempA[2] = LOW;
          }
        }

        fan();
        
      }
    }

    if (tempB[0] != tempB[1]) {
      delay(200);
      if (digitalRead(pinB) == tempB[0]) {
        tempB[1] = tempB[0];

        if (tempB[1] == HIGH) {
          if (tempB[2] == LOW) {
            tempB[2] = HIGH;
          } else {
            tempB[2] = LOW;
          }
        }

        heater();
      }
    }
  }

  if (millis() - m >= 500) {
    Serial.print(digitalRead(led1));
    Serial.print("   ");
    Serial.print(digitalRead(led2));
    Serial.print("   ");
    Serial.print(digitalRead(led3));
    Serial.print("   ");
    Serial.print(modes);
    Serial.print("   ");
    Serial.print(digitalRead(pinA));
    Serial.print("   ");
    Serial.print(digitalRead(pinB));
    Serial.print("   ");
    Serial.print(digitalRead(pinC));
    Serial.print("   ");
    Serial.println(digitalRead(pinD));
    m = millis();
  }

  lcd.setCursor (0, 0);
  lcd.print("Suhu :              ");

  lcd.setCursor (6, 0);
  lcd.print(suhuDS1820);
  lcd.print((char)223);
  lcd.print("C");

  lcd.setCursor (0, 1);
  lcd.print("Status :            ");
  lcd.setCursor (8, 1);
  lcd.print(Status);

  lcd.setCursor (0, 2);
  lcd.print("Kipas :             ");
  lcd.setCursor (7, 2);
  lcd.print(statusKipas);

  lcd.setCursor (0, 3);
  lcd.print("Pemanas :           ");
  lcd.setCursor (9, 3);
  lcd.print(statusPemanas);
}

void heater() {
  if (tempB[2] == HIGH) {
    digitalWrite (PEMANAS, LOW);
    digitalWrite (led3, LOW);
    statusPemanas = "ON";
  }

  if (tempB[2] == LOW) {
    digitalWrite (PEMANAS, HIGH);
    digitalWrite (led3, HIGH);
    statusPemanas = "OFF";
  }
}

void fan() {
  if (tempA[2] == HIGH) {
    digitalWrite (KIPAS, LOW);
    digitalWrite (led2, LOW);
    statusKipas = "ON";
  }

  if (tempA[2] == LOW) {
    digitalWrite (KIPAS, HIGH);
    digitalWrite (led2, HIGH);
    statusKipas = "OFF";
  }
}
