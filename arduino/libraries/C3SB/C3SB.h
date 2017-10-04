#ifndef C3SB_H
#define C3SB_H
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
#include <inttypes.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#define WIRE_WRITE(ptr,n_byte) (Wire.write((uint8_t*)(ptr), (n_byte)))
#define WIRE_WRITE1(one_byte) (Wire.write((uint8_t)(one_byte)))
#define WIRE_READ Wire.read()
#else
#include "WProgram.h"
#define WIRE_WRITE(ptr,n_byte) (Wire.send((uint8_t*)(ptr), (n_byte))
#define WIRE_WRITE1(one_byte) (Wire.send((one_byte)))
#define WIRE_READ Wire.receive()
#endif

const uint8_t C3SB_MASTER_ID = 0;
const uint8_t I2C_BUFFER_LEN = 32;
const uint8_t C3SB_DESC_ID = 1;
const uint8_t C3SB_WRITE_MSG = 2; 

class C3SB{
 public:
  C3SB();
  void init(char* desc);
  char* target;
  uint8_t other_id;

  boolean message_from(uint8_t device_id,
		       uint8_t msg_id,
		       uint8_t* dest,
		       uint8_t n_byte);

  boolean message_to(uint8_t device_id,
		     uint8_t msg_id,
		     uint8_t* payload,
		     uint8_t n_byte);
  // master
  uint8_t read_from(uint8_t device_id,
		    uint8_t* dest,
		    uint8_t n_byte);
  
  // master
  boolean write_to(uint8_t device_id,
		   uint8_t *payload,
		   uint8_t n_byte);
  void raw_send(uint8_t* source, uint8_t n_byte, boolean write_byte);
  uint8_t raw_read(uint8_t* source, uint8_t n_byte);
 private:
  void handle_receive(int n_byte);
  void handle_request();
  uint8_t msg_id;
  uint8_t description[I2C_BUFFER_LEN];
};
#endif
