#include "C3SB.h"
#include "Wire.h"

C3SB c3sb = C3SB(C3SB_MASTER_ID, "ATMEGA328p");
char long_msg[100];
void setup(){
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Wire.begin();
  Serial.begin(57600);
  Serial.println("Begin");
}

void loop(){
  c3sb.read_from((uint8_t)2, long_msg, (int)100);
  Serial.print("From 2: ");
  Serial.println(long_msg);
  delay(1000);
}
