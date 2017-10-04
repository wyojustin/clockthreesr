#include <inttypes.h>
#include "EDL.h"


/*
 * Return true if eeprom does not violate EEPROM bounds.
 */
bool did_check_eeprom(){
  uint8_t did, len, n;
  int16_t addr = 0;
  bool out = true;

  n = get_n_dids();
  for(uint8_t i = 0; i < n && addr < MAX_EEPROM_ADDR - 2; i++){
    addr += EEPROM.read(addr + 1);
    if(addr > MAX_EEPROM_ADDR){
      out = false;
    }
  }
  return out;
}

/*
 * Return the number if DID's currently stored
 */
uint8_t get_n_dids(){
  return EEPROM.read(MAX_EEPROM_ADDR);
}


/* 
 * Copy DID to data to dest.  
 * dest -- location to copy DID record.  It must have enough space allocated to contain entire record.
 * len_p -- ouput, if output is true, the value pointed to by this pointer will contian the length for this did
 */
bool did_read(uint8_t did, char *dest, uint8_t *len_p){
  int16_t addr;
  bool status = false;

  if(get_did_addr(did, &addr, len_p)){
    for(uint8_t i = 0; i < *len_p; i++){
      dest[i] = EEPROM.read(addr + i);
    }
    status = true;
  }
  return status;
}

/* 
 * Write a valid DID stored in data.
 * return true if successful, false otherwise.
 */
bool did_write(char* data){
  uint8_t n = get_n_dids();
  int16_t addr = 0;
  uint8_t did = data[0];
  uint8_t len = data[1];
  bool status = false;
  
  int16_t tmp_addr;
  uint8_t tmp_l;
  if((did != 0) && (did != 255)){ // 0 and 255 are not a valid dids
    if(!get_did_addr(did, &tmp_addr, &tmp_l)){ // check if did already exists
      if(did_next_addr(&addr)){                // find next EEPROM address
	if(addr + len < MAX_EEPROM_ADDR - 1){  // make sure there is enough space
	  for(uint8_t i = 0; i < len; i++){
	    EEPROM.write(addr + i, data[i]);   // copy to EEPROM
	  }
	  EEPROM.write(MAX_EEPROM_ADDR, n + 1); // update # dids
	  status = true;
	}
      }
    }
  }
  return status;
}

/*
 * Get next available DID address.  
 * Return true if space any space remains, otherwise false
 */
bool did_next_addr(int16_t *addr_p){
  uint8_t n = EEPROM.read(MAX_EEPROM_ADDR);
  bool status = false;
  *addr_p = 0;

  // go to end of the line of DIDs
  for(uint8_t i = 0; 
      i < n && *addr_p < MAX_EEPROM_ADDR - 2; 
      i++){
    *addr_p += EEPROM.read(*addr_p + 1);
  }
  if(*addr_p < MAX_EEPROM_ADDR - 2){
    status = true;
  }
  else{
    // strcpy(serial_msg, "mem");
  }
  return status;
}

/*
 * delete data store under identifier did
 * return true if successful, false otherwise
 */
bool did_delete(uint8_t did){
  // TODO: rather than move everything down, see if another DID has same length
  //       then just copy over!  faster!
  int16_t addr, next_addr;
  uint8_t len;
  uint8_t n = EEPROM.read(MAX_EEPROM_ADDR);
  bool status = false;
  if(get_did_addr(did, &addr, &len)){
    status = true;
    if(n == 1){ // only one --> delete from address 0
      len = EEPROM.read(1);
      for(uint8_t i = 0; i < len; i++){
	EEPROM.write(i, 0);
      }
      EEPROM.write(MAX_EEPROM_ADDR, 0);
    }
    else{
      if(did_next_addr(&next_addr)){
	// slide everything down
	for(int i = addr; i < next_addr - 1 - len; i++){
	  EEPROM.write(i, EEPROM.read(i + len));
	}
	// delete tail
	for(int i = next_addr - len; i < next_addr; i++){
	  EEPROM.write(i, 0);
	}
      }
      if(n > 0){
	EEPROM.write(MAX_EEPROM_ADDR, n - 1);
      }
    }
  }
  return status;
}

/*
 * Get the address of DID identified by did.
 * did -- data id
 * addr_p -- output, if output is true, the value pointed to by this pointer will contian the address for this did
 * len_p -- output, if output is true, the value pointed to by this pointer will contian the length for this did
 */
bool get_did_addr(uint8_t did, int16_t* addr_p, uint8_t *len_p){
  bool status = false;
  uint8_t id;
  if(0 < did && did <  255){
    *addr_p = 0;
    id = EEPROM.read(*addr_p);
    uint8_t n = EEPROM.read(MAX_EEPROM_ADDR);
    uint8_t i = 0;
    while((*addr_p < MAX_EEPROM_ADDR - 2) && 
	  (did != id) && 
	  (i++ < n)){
      *len_p = EEPROM.read(*addr_p + 1);
      *addr_p += *len_p;
      id = EEPROM.read(*addr_p);
    }
    if(id == did){
      // make sure it is not too long
      if(*addr_p < MAX_EEPROM_ADDR - 2){
	*len_p = EEPROM.read(*addr_p + 1);
	status = true;
      }
    }
    else{
    }
  }
  return status;
}

/*
 * Replace byte at record location "offset" of record stored at did with
 * new_byte.  Cannot use this to modify DID or SIZE. Use delete for thes
 * actions.
 */
bool did_edit(uint8_t did, uint8_t offset, uint8_t new_byte){
  bool status = false;
  int16_t addr;
  uint8_t len;

  if(get_did_addr(did, &addr, &len)){
    if(1 < offset && offset < len){
      EEPROM.write(addr + offset, new_byte);
      status = true;
    }
  }
  return status;
}

/*
 * clear the EEPROM (stuff with all zeros)
 */
void did_format_eeprom(){
  for(uint16_t i = 0; i <= MAX_EEPROM_ADDR; i++){
    EEPROM.write(i, 0);
  }
}
