#include <SoftwareSerial.h>

#define OK 1
#define NOTOK 2
#define TIMEOUT 3
#define RST 2

#define SERIALTIMEOUT 3000
SoftwareSerial A6board (4, 5);
char end_c[] {0x1a,'\0'};

void setup() {
  A6board.begin(115200);   // the GPRS baud rate
  Serial.begin(115200);    // the GPRS baud rate
  Serial.println("Start");

  pinMode(RST, OUTPUT);
  digitalWrite(RST, HIGH);
  delay(5000);
  digitalWrite(RST, LOW);
  delay(500);
  if (A6begin() != OK) {
    Serial.println("Error");
    while (1 == 1);
  }
}

void loop(){
  Serial.println("-------------------------Start------------------------------");
  byte message[] = {0x10, 0x12, 0x00, 0x04, 0x4d, 0x51 , 0x54 , 0x54 , 0x04 , 0x02 , 0x00 , 0x3c , 0x00 , 0x06 , 0x41 , 0x42 , 0x43 , 0x44 , 0x45 , 0x46 , 0x30 , 0x13 , 0x00 , 0x08 , 0x76 , 0x61 , 0x6c , 0x65 , 0x74 , 0x72 , 0x6f , 0x6e , 0x68 , 0x65 , 0x6c , 0x6c , 0x6f , 0x72 , 0x61, 0x76 , 0x69 };
  A6command("AT+CGATT=1", "OK", "yy", 20000, 1);
  A6command("AT+CGDCONT=1,\"IP\",\"wm\"", "OK", "yy", 20000, 1);
  A6command("AT+CGACT=1,1", "OK", "yy", 10000, 1);
  A6command("AT+CIPSTART=\"TCP\",\"iot.eclipse.org\",1883", "CONNECT OK", "yy", 25000, 1);
  A6command("AT+CIPSEND", ">", "yy", 10000, 1); 
  A6board.write(message, sizeof(message));
  A6board.println(end_c);
  A6command("AT+CIPSTATUS", "OK", "yy", 10000, 1);
  A6command("AT+CIPCLOSE", "OK", "yy", 15000, 1);
  Serial.println("-------------------------End------------------------------");
  delay(30000);
}

byte A6waitFor(String response1, String response2, int timeOut) {
  unsigned long entry = millis();
  int count = 0;
  String reply = A6read();
  byte retVal = 99;
  do {
    reply = A6read();
    if (reply != "") {
      Serial.print((millis() - entry));
      Serial.print(" ms ");
      Serial.println(reply);
    }
  } while ((reply.indexOf(response1) + reply.indexOf(response2) == -2) && millis() - entry < timeOut );
  if ((millis() - entry) >= timeOut) {
    retVal = TIMEOUT;
  } else {
    if (reply.indexOf(response1) + reply.indexOf(response2) > -2) retVal = OK;
    else retVal = NOTOK;
  }
  //  Serial.print("retVal = ");
  //  Serial.println(retVal);
  return retVal;
}

byte A6command(String command, String response1, String response2, int timeOut, int repetitions) {
  byte returnValue = NOTOK;
  byte count = 0;
  while (count < repetitions && returnValue != OK) {
    A6board.println(command);
    Serial.print("Command: ");
    Serial.println(command);
    if (A6waitFor(response1, response2, timeOut) == OK) {
      //     Serial.println("OK");
      returnValue = OK;
    } else returnValue = NOTOK;
    count++;
  }
  return returnValue;
}




void A6input() {
  String hh;
  char buffer[100];
  while (1 == 1) {
    if (Serial.available()) {
      hh = Serial.readStringUntil('\n');
      hh.toCharArray(buffer, hh.length() + 1);
      if (hh.indexOf("ende") == 0) {
        A6board.write(end_c);
        Serial.println("ende");
      } else {
        A6board.write(buffer);
        A6board.write('\n');
      }
    }
    if (A6board.available()) {
      Serial.write(A6board.read());
    }
  }
}


bool A6begin() {
  A6board.println("AT+CREG?");
  byte hi = A6waitFor("1,", "5,", 1500);  // 1: registered, home network ; 5: registered, roaming
  while ( hi != OK) {
    A6board.println("AT+CREG?");
    hi = A6waitFor("1,", "5,", 1500);
  }

  if (A6command("AT&F0", "OK", "yy", 5000, 2) == OK) {   // Reset to factory settings
    if (A6command("ATE0", "OK", "yy", 5000, 2) == OK) {  // disable Echo
      if (A6command("AT+CMEE=2", "OK", "yy", 5000, 2) == OK) return OK;  // enable better error messages
      else return NOTOK;
    }
  }
}

void ShowSerialData()
{
  unsigned long entry = millis();
  while ( A6board.available() != 0 && millis() - entry < SERIALTIMEOUT)
    Serial.println(A6board.readStringUntil('\n'));
}

String A6read() {
  String reply = "";
  if (A6board.available())  {
    reply = A6board.readString();
  }
  //  Serial.print("Reply: ");
  //  Serial.println(reply);
  return reply;
}

