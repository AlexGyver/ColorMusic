/*
Copyright (c) 2014-2015 NicoHood
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// Include guard
#pragma once

//================================================================================
// Protocol Definitions
//================================================================================

//HashIR
#define HASHIR_BLOCKS 255				// 0-65535 (maximum input length)
#define HASHIR_TIMEOUT (0xFFFF/4)		// 65535, max timeout
#define HASHIR_TIME_THRESHOLD 10000UL	// 0-32bit

//================================================================================
// Decoding Class
//================================================================================

class HashIR : public CIRLData{
public:
	HashIR(void){
		// Empty	
	}
	
	// Hide anything that is inside this class so the user dont accidently uses this class
	template<typename protocol, typename ...protocols>
	friend class CIRLremote;
	
private:
	static inline uint8_t getSingleFlag(void) __attribute__((always_inline));
	static inline bool requiresCheckTimeout(void) __attribute__((always_inline));
	static inline void checkTimeout(void) __attribute__((always_inline));
	static inline bool available(void) __attribute__((always_inline));
	static inline void read(IR_data_t* data) __attribute__((always_inline));
	static inline bool requiresReset(void) __attribute__((always_inline));
	static inline void reset(void) __attribute__((always_inline));
	
	// Decode functions for a single protocol/multiprotocol for less/more accuration
	static inline void decodeSingle(const uint16_t &duration) __attribute__((always_inline));
	static inline void decode(const uint16_t &duration) __attribute__((always_inline));

protected:
	// Temporary buffer to hold bytes for decoding the protocols
	// not all of them are compiled, only the used ones
	static uint16_t durationHashIR;
	static uint16_t countHashIR;
	static uint32_t commandHashIR;
};


uint8_t HashIR::getSingleFlag(void){
	return CHANGE;
}


bool HashIR::requiresCheckTimeout(void){
	// Used in this protocol
	return true;
}


void HashIR::checkTimeout(void){
	// This function is executed with interrupts turned off
	if (countHashIR) {
		// Check if reading timed out and save value.
		if ((micros() - IRLLastTime) >= HASHIR_TIMEOUT) {
			// Flag a new input if reading timed out
			countHashIR--;
			
			IRLProtocol = IR_HASH;
			IRLLastEvent = IRLLastTime;
		}
	}
}


bool HashIR::available(void)
{
	// Only return a value if this protocol has new data
	if(IRLProtocol == IR_HASH)
		return true;
	else
		return false;	
}


void HashIR::read(IR_data_t* data){
	// Only (over)write new data if this protocol received any data
	if(IRLProtocol == IR_HASH){
		// Save address as length.
		// You can check the address/length to prevent triggering on noise
		data->address = countHashIR;
		
		// Save calculated hash
		data->command = commandHashIR;
	}
}


bool HashIR::requiresReset(void){
	// Used in this protocol
	return true;
}


void HashIR::reset(void){
	// Reset protocol for new reading
	countHashIR = 0;
	commandHashIR = FNV_BASIS_32;
}


void HashIR::decodeSingle(const uint16_t &duration){
	// Reading timed out
	if(duration >= HASHIR_TIMEOUT)
	{
		// Start a new reading sequence.
		if(countHashIR == 0){
			countHashIR++;
		}
		// Ignore the very first timeout of each reading.
		// Otherwise flag a new input and stop reading.
		else if(countHashIR != 1){
			countHashIR--;
			IRLProtocol = IR_HASH;
		}
		return;
	}
	
	// Only save data if a sequence is running.
	// This is required to avoid corrupted data
	// when starting capturing at the middle of a sequence.
	if(countHashIR)
	{
		// Converts the raw code values into a 32-bit hash code.
		// Hopefully this code is unique for each button.
		// This isn't a "real" decoding, just an arbitrary value.
		// Code taken from https://github.com/z3t0/Arduino-IRremote

		// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
	#define FNV_PRIME_32 16777619UL
	#define FNV_BASIS_32 2166136261UL

		// Only compare after the first value got received
		if(countHashIR > 1){
			// Get both values
			auto oldval = durationHashIR;
			auto newval = duration;

			// Compare two tick values, returning 0 if newval is shorter,
			// 1 if newval is equal, and 2 if newval is longer
			// Use a tolerance of 75%
			uint8_t value = 1;
			if (newval < (oldval * 3 / 4)) {
				value = 0;
			}
			else if (oldval < (newval * 3 / 4)) {
				value = 2;
			}

			// Add value into the hash
			commandHashIR = (commandHashIR * FNV_PRIME_32) ^ value;
		}
		
		// Save last time and count up
		countHashIR++;
		durationHashIR = duration;

		// Flag a new input if buffer is full
		if((countHashIR > HASHIR_BLOCKS)){
			countHashIR--;
			IRLProtocol = IR_HASH;
		}
	}
}


void HashIR::decode(const uint16_t &duration) {
	// Wait some time after the last protocol.
	// This way it can finish its (possibly ignored) stop bit.
	uint8_t lastProtocol = IRLProtocol | IR_NEW_PROTOCOL;
	if(lastProtocol != IR_HASH && lastProtocol != IR_NEW_PROTOCOL && (IRLLastTime - IRLLastEvent < HASHIR_TIME_THRESHOLD))
		return;

	decodeSingle(duration);
}
