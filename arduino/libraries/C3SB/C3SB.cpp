/*
  C3SB is a serial protocol based on the arduino Wire library.

  LIBRARY DATED July 6, 2011

  Licenced under Creative Commons Attribution.
  Attribution 3.0 Unported
  This license is acceptable for Free Cultural Works.
  You are free:

  * to Share — to copy, distribute and transmit the work
  * to Remix — to adapt the work
  *

  Under the following conditions:

  *

  Attribution — You must attribute the work in the manner specified by 
  the author or licensor (but not in any way that suggests that they endorse
  you or your use of the work).

  Attribute this work:
  Information
  What does "Attribute this work" mean?
  The page you came from contained embedded licensing metadata, including
  how the creator wishes to be attributed for re-use. You can use the HTML here 
  to cite the work. Doing so will also include metadata on your page so that 
  others can find the original work as well.

  With the understanding that:
  * Waiver — Any of the above conditions can be waived if you get permission 
  from the copyright holder.
  * Public Domain — Where the work or any of its elements is in the public 
  domain under applicable law, that status is in no way affected by the 
  license.
  * Other Rights — In no way are any of the following rights affected by the
  license:
  o Your fair dealing or fair use rights, or other applicable copyright
  exceptions and limitations;
  o The author's moral rights;
  o Rights other persons may have either in the work itself or in how 
  the work is used, such as publicity or privacy rights.
  * Notice — For any reuse or distribution, you must make clear to others 
  the license terms of this work. The best way to do this is with a link 
  to this web page.
*/

#include "C3SB.h"
#include <inttypes.h>
#include "Wire.h"

C3SB::C3SB(){
}
void C3SB::init(char* desc){
  msg_id = 0;
  for(uint8_t i=0; i < I2C_BUFFER_LEN && desc[i]; i++){
    description[i] = desc[i];
  }
  // Wire.onReceive(handle_receive);
  // Wire.onRequest(handle_request);
}

boolean C3SB::message_from(uint8_t device_id,
			   uint8_t msg_id,
			   uint8_t* dest,
			   uint8_t n_byte){
  write_to(device_id, &msg_id, 1);
  read_from(device_id, dest, n_byte);
}

boolean C3SB::message_to(uint8_t device_id,
			 uint8_t msg_id,
			 uint8_t* payload,
			 uint8_t n_byte){
  write_to(device_id, &msg_id, 1);
  write_to(device_id, payload, n_byte);
}

uint8_t C3SB::read_from(uint8_t device_id,
			uint8_t *dest,
			uint8_t n_byte){
  other_id = device_id;
  uint8_t out = 0;
  uint8_t bytes_this_read;

  uint8_t n_reads = n_byte / I2C_BUFFER_LEN;
  for(uint8_t i = 0; i < n_reads; i++){
     bytes_this_read = raw_read(dest + i * I2C_BUFFER_LEN, I2C_BUFFER_LEN);
     if(bytes_this_read != I2C_BUFFER_LEN){
       Serial.print("Missing ");
       Serial.print(I2C_BUFFER_LEN - bytes_this_read);
       Serial.println(" bytes.");
     }
     out += bytes_this_read;
  }
  if(n_byte % I2C_BUFFER_LEN){
    out += raw_read(dest + n_reads * I2C_BUFFER_LEN, n_byte % I2C_BUFFER_LEN);
  }
  return out;
}

boolean C3SB::write_to(uint8_t device_id,
		       uint8_t *payload,
		       uint8_t n_byte){
  other_id = device_id;

  uint8_t n_writes = n_byte / (I2C_BUFFER_LEN - 1);
  for(uint8_t i = 0; i < n_writes; i++){
    raw_send(payload + i * (I2C_BUFFER_LEN - 1), I2C_BUFFER_LEN - 1, true);
  }
  raw_send(payload + n_writes * (I2C_BUFFER_LEN - 1), 
	    n_byte % (I2C_BUFFER_LEN - 1), true);
}

void C3SB::raw_send(uint8_t* data, uint8_t n_byte, boolean write_byte){
  Wire.beginTransmission(other_id);
  // Serial.print("Start to ");
  // Serial.print(other_id, DEC);
  // Serial.print(", ");
  if(write_byte){
#if defined(ARDUINO) && ARDUINO >= 100
    WIRE_WRITE(C3SB_WRITE_MSG, 1);
#else
    Wire.send(C3SB_WRITE_MSG);
#endif
  }
  for(int i = 0; i < n_byte && i < I2C_BUFFER_LEN - 1; i++){
    // Serial.print(data[i]);
#if defined(ARDUINO) && ARDUINO >= 100
    WIRE_WRITE(data[i], 1);
#else
    Wire.send(data[i]);
#endif
  }
  // Serial.println("");
  Wire.endTransmission();
}
uint8_t C3SB::raw_read(uint8_t* data, uint8_t n_byte){
  Wire.requestFrom(other_id, n_byte);    // trigger slave handle_resquest()
  uint8_t i;
  for(i = 0; i < n_byte && Wire.available(); i++){
    data[i] = WIRE_READ;
  }
  return i;
}

void C3SB::handle_receive(int n_byte){
  msg_id = WIRE_READ;
  // todo: handle longer reciepts!
}
void C3SB::handle_request(){
  // handle descreption requests
  if(msg_id == C3SB_DESC_ID){
#if defined(ARDUINO) && ARDUINO >= 100
    WIRE_WRITE(description, I2C_BUFFER_LEN);
#else
    Wire.send(description, I2C_BUFFER_LEN);
#endif
  }
  else{
    // user_req_handler(msg_id); // pass on to user routine
  }
}
