/*
  EEPROMEx.cpp - Extended EEPROM library
  Copyright (c) 2012 Thijs Elenbaas.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "EEPROMex.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

 #define _EEPROMEX_VERSION 1_0_0 // software version of this library
 //#define _EEPROMEX_DEBUG         // Enables logging of maximum of writes and out-of-memory
/******************************************************************************
 * Constructors
 ******************************************************************************/

// Boards with ATmega328, Duemilanove, Uno, Uno SMD, Lilypad - 1024 bytes (1 kilobyte)
// Boards with ATmega1280 or 2560, Arduino Mega series – 4096 bytes (4 kilobytes)
// Boards with ATmega168, Lilypad, old Nano, Diecimila  – 512 bytes
// By default we choose conservative settings
EEPROMClassEx::EEPROMClassEx()
  :  _allowedWrites(100)
{
}
 
/******************************************************************************
 * User API
 ******************************************************************************/

 /**
 * Set starting position and memory size that EEPROMEx may manage
 */
void EEPROMClassEx::setMemPool(int base, int memSize) {
	//Base can only be adjusted if no addresses have already been issued
	if (_nextAvailableaddress == _base) 
		_base = base;
		_nextAvailableaddress=_base;
	
	//Ceiling can only be adjusted if not below issued addresses
	if (memSize >= _nextAvailableaddress ) 
		_memSize = memSize;

	#ifdef _EEPROMEX_DEBUG    
	if (_nextAvailableaddress != _base) 
		Serial.println("Cannot change base, addresses have been issued");

	if (memSize < _nextAvailableaddress )  
		Serial.println("Cannot change ceiling, below issued addresses");
	#endif	
	
}

/**
 * Set global maximum of allowed writes
 */
void EEPROMClassEx::setMaxAllowedWrites(int allowedWrites) {
#ifdef _EEPROMEX_DEBUG
	_allowedWrites = allowedWrites;
#endif			
}

/**
 * Get a new starting address to write to. Adress is negative if not enough space is available
 */
int EEPROMClassEx::getAddress(int noOfBytes){
	int availableaddress   = _nextAvailableaddress;
	_nextAvailableaddress += noOfBytes;

#ifdef _EEPROMEX_DEBUG    
	if (_nextAvailableaddress > _memSize) {
		Serial.println("Attempt to write outside of EEPROM memory");
		return -availableaddress;
	} else {
		return availableaddress;
	}
#endif
	return availableaddress;		
}
 
/**
 * Check if EEPROM memory is ready to be accessed
 */
bool EEPROMClassEx::isReady() {
	return eeprom_is_ready();
}

/**
 * Read a single byte
 * This function performs as readByte and is added to be similar to the EEPROM library
 */
uint8_t EEPROMClassEx::read(int address)
{
	return readByte(address);
}

/**
 * Read a single bit
 */
bool EEPROMClassEx::readBit(int address, byte bit) {
	  if (bit> 7) return false; 
	  if (!isReadOk(address+sizeof(uint8_t))) return false;
	  byte byteVal =  eeprom_read_byte((unsigned char *) address);      
	  byte bytePos = (1 << bit);
      return (byteVal & bytePos);
}

/**
 * Read a single byte
 */
uint8_t EEPROMClassEx::readByte(int address)
{	
	if (!isReadOk(address+sizeof(uint8_t))) return 0;
	return eeprom_read_byte((unsigned char *) address);
}

/**
 * Read a single 16 bits integer
 */
uint16_t EEPROMClassEx::readInt(int address)
{
	if (!isReadOk(address+sizeof(uint16_t))) return 0;
	return eeprom_read_word((uint16_t *) address);
}

/**
 * Read a single 32 bits integer
 */
uint32_t EEPROMClassEx::readLong(int address)
{
	if (!isReadOk(address+sizeof(uint32_t))) return 0;
	return eeprom_read_dword((unsigned long *) address);
}

/**
 * Read a single float value
 */
float EEPROMClassEx::readFloat(int address)
{
	if (!isReadOk(address+sizeof(float))) return 0;
	float _value;
	readBlock<float>(address, _value);
	return _value;
}

/**
 * Read a single double value (size will depend on board type)
 */
double EEPROMClassEx::readDouble(int address)
{
	if (!isReadOk(address+sizeof(double))) return 0;	
	double _value;
	readBlock<double>(address, _value);
	return _value;
}

/**
 * Write a single byte
 * This function performs as writeByte and is added to be similar to the EEPROM library
 */
bool EEPROMClassEx::write(int address, uint8_t value)
{
	return writeByte(address, value);
}

/**
 * Write a single bit
 */
bool EEPROMClassEx::writeBit(int address, uint8_t bit, bool value) {
	updateBit(address, bit, value);
	return true;
}

/**
 * Write a single byte
 */
bool EEPROMClassEx::writeByte(int address, uint8_t value)
{
	if (!isWriteOk(address+sizeof(uint8_t))) return false;
	eeprom_write_byte((unsigned char *) address, value);
	return true;
}

/**
 * Write a single 16 bits integer
 */
bool EEPROMClassEx::writeInt(int address, uint16_t value)
{
	if (!isWriteOk(address+sizeof(uint16_t))) return false;
	eeprom_write_word((uint16_t *) address, value);
	return true;
}

/**
 * Write a single 32 bits integer
 */
bool EEPROMClassEx::writeLong(int address, uint32_t value)
{
	if (!isWriteOk(address+sizeof(uint32_t))) return false;
	eeprom_write_dword((unsigned long *) address, value);
	return true;
}

/**
 * Write a single float value
 */
bool EEPROMClassEx::writeFloat(int address, float value)
{
	return (writeBlock<float>(address, value)!=0);	
}

/**
 * Write a single double value (size will depend on board type)
 */
bool EEPROMClassEx::writeDouble(int address, double value)
{
	return (writeBlock<float>(address, value)!=0);	
}

/**
 * Update a single byte
 * The EEPROM will only be overwritten if different. This will reduce wear.
 * This function performs as updateByte and is added to be similar to the EEPROM library
 */
bool EEPROMClassEx::update(int address, uint8_t value)
{
	return (updateByte(address, value));
}

/**
 * Update a single bit
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool EEPROMClassEx::updateBit(int address, uint8_t bit, bool value) 
{
	  if (bit> 7) return false; 
	  
	  byte byteValInput  = readByte(address);
	  byte byteValOutput = byteValInput;	  
	  // Set bit
	  if (value) {	    
		byteValOutput |= (1 << bit);  //Set bit to 1
	  } else {		
	    byteValOutput &= ~(1 << bit); //Set bit to 0
	  }
	  // Store if different from input
	  if (byteValOutput!=byteValInput) {
		writeByte(address, byteValOutput);	  
	  }
	  return true;
}


/**
 * Update a single byte
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool EEPROMClassEx::updateByte(int address, uint8_t value)
{
	return (updateBlock<uint8_t>(address, value)!=0);
}

/**
 * Update a single 16 bits integer 
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool EEPROMClassEx::updateInt(int address, uint16_t value)
{
	return (updateBlock<uint16_t>(address, value)!=0);
}

/**
 * Update a single 32 bits integer 
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool EEPROMClassEx::updateLong(int address, uint32_t value)
{
	return (updateBlock<uint32_t>(address, value)!=0);
}

/**
 * Update a single float value 
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool EEPROMClassEx::updateFloat(int address, float value)
{
	return (updateBlock<float>(address, value)!=0);
}

/**
 * Update a single double value (size will depend on board type)
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool EEPROMClassEx::updateDouble(int address, double value)
{
	return (updateBlock<double>(address, value)!=0);
}

/**
 * Performs check to see if writing to a memory address is allowed
 */
bool EEPROMClassEx::isWriteOk(int address)
{
#ifdef _EEPROMEX_DEBUG    
	_writeCounts++;
	if (_allowedWrites == 0 || _writeCounts > _allowedWrites ) {
		Serial.println("Exceeded maximum number of writes");
		return false;
	}
	
	if (address > _memSize) {
		Serial.println("Attempt to write outside of EEPROM memory");
		return false;
	} else {
		return true;
	}
#endif		
	return true;
}

/**
 * Performs check to see if reading from a memory address is allowed
 */
bool EEPROMClassEx::isReadOk(int address)
{
#ifdef _EEPROMEX_DEBUG    
	if (address > _memSize) {
		Serial.println("Attempt to write outside of EEPROM memory");
		return false;
	} else {
		return true;
	}
#endif
	return true;	
}

int EEPROMClassEx::_base= 0;
int EEPROMClassEx::_memSize= 512;
int EEPROMClassEx::_nextAvailableaddress= 0;
int EEPROMClassEx::_writeCounts =0;

EEPROMClassEx EEPROM;
