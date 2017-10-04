#include "C3SB.h"
#include "Wire.h"

C3SB c3sb = C3SB(MASTER_ID, "ATMEGA328p");
char* long_msg = "This is a long message longer than 32 bytes.  Can we send it???????";
void setup(){
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Wire.begin();
  Serial.begin(57600);
  Serial.println("Begin");
}

void loop(){
  c3sb.write_to(4, long_msg, strlen(long_msg));
  delay(1000);
}
