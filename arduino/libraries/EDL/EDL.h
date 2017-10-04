/*
  EDL.h -- EEPROM DATA LIBRARY for Arduino.  
  Data handling routines for easy assured access to EEPROM data.
  Each record is reference by a Data ID or DID for short.  
  The library has been optimized for storage (as opposed to fast access).

  *** First two bytes of a DID record are always always always DID and LEN

  Justin Shaw
  LIBRARY VERSION: 0.01, DATED 1/6/2011

Licenced under Creative Commons Attribution.
Attribution 3.0 Unported
*/

#ifndef EDL_H
#define ELD_H
#include "EEPROM.h"

const uint16_t MAX_EEPROM_ADDR = 1023;

/* the length parameter len includes the did byte and the len byte itself.
 *
 * did len = payload len + 2 (1 for did, 1 for len itself)
 *
 * did,len,payload_bytes
 * 
 * EEPROM Map -- no gaps between DIDs!
 *      DID: 0                 1       2     3     4   ...
 *            |---------------->|------>|---->|---->|---...
 * ADDR/LEN: 0/20              20/7    27/5  32/5  37/ ...  
 * number of DIDs stored in last byte of EEPROM
 */


/*
 * Return true if eeprom does not violate EEPROM bounds.
 */
bool did_check_eeprom();

/*
 * Return the number if DID's currently stored
 */
uint8_t get_n_dids();

/*
 * Get next available DID address.  
 * Return true if space any space remains, otherwise false
 */
bool did_next_addr(int16_t *addr_p);

/* 
 * Copy DID to data to dest.  
 * dest -- location to copy DID record.  It must have enough space allocated to contain entire record.
 * len_p -- ouput, if output is true, the value pointed to by this pointer will contian the length for this did
 * First two bytes are always always always DID and LEN
 */
bool did_read(uint8_t did, char *dest, uint8_t *len_p);

/* 
 * Write a valid DID stored in data.
 * return true if successful, false otherwise.
 */
bool did_write(char* data);

/*
 * Get the address of DID identified by did.
 * did -- data id
 * addr_p -- output, if output is true, the value pointed to by this pointer will contian the address for this did
 * len_p -- output, if output is true, the value pointed to by this pointer will contian the length for this did
 * First two bytes are always always always DID and LEN
 */
bool get_did_addr(uint8_t did, int16_t* addr_p, uint8_t *len_p);

/*
 * delete data store under identifier did
 * return true if successful, false otherwise
 */
bool did_delete(uint8_t did);

/*
 * Replace byte at record location "offset" of record stored at did with
 * new_byte.  Cannot use this to modify DID or SIZE. Use delete for thes
 * actions.
 */
bool did_edit(uint8_t did, uint8_t offset, uint8_t new_byte);

/*
 * clear the EEPROM (stuff with all zeros)
 */
void did_format_eeprom();

#endif
