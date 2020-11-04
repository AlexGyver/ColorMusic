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

//RawIR
#define RAWIR_BLOCKS 100				// 0-65535
#define RAWIR_TIMEOUT (0xFFFF/4)		// 65535, max timeout
#define RAWIR_TIME_THRESHOLD 10000UL	// 0-32bit

// Determine buffer length datatype
#if (RAWIR_BLOCKS >= 255)
#define RAWIR_DATA_T uint16_t
#else
#define RAWIR_DATA_T uint8_t
#endif

//================================================================================
// Decoding Class
//================================================================================

class RawIR : public CIRLData{
public:
	RawIR(void){
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

public:
	// Temporary buffer to hold bytes for decoding the protocols
	// not all of them are compiled, only the used ones
	static volatile RAWIR_DATA_T countRawIR;
	static volatile uint16_t dataRawIR[RAWIR_BLOCKS];
};


uint8_t RawIR::getSingleFlag(void){
	return CHANGE;
}


bool RawIR::requiresCheckTimeout(void){
	// Used in this protocol
	return true;
}


void RawIR::checkTimeout(void){
	// This function is executed with interrupts turned off
	if(countRawIR)
	{
		// Check if reading timed out and save value.
		if ((micros() - IRLLastTime) >= RAWIR_TIMEOUT)
		{
			// Flag a new input if reading timed out
			countRawIR--;

			// Ignore the very first timeout.
			// This should normally never happen, but in any case
			// try to avoid a timeout only flag with bufferlength 0.
			if(countRawIR){
				IRLProtocol = IR_RAW;
				IRLLastEvent = IRLLastTime;
			}
		}
	}
}


bool RawIR::available(void)
{
	// Only return a value if this protocol has new data
	if(IRLProtocol == IR_RAW)
		return true;
	else
		return false;	
}


void RawIR::read(IR_data_t* data){
	// Only (over)write new data if this protocol received any data
	if(IRLProtocol == IR_RAW){
		// Converts the raw code values into a 32-bit hash code.
		// Hopefully this code is unique for each button.
		// This isn't a "real" decoding, just an arbitrary value.
		// Code taken from https://github.com/z3t0/Arduino-IRremote

		// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619UL
#define FNV_BASIS_32 2166136261UL

		// Save address as length.
		// You can check the address/length to prevent triggering on noise
		data->address = countRawIR;

		// Iterate through all raw values and calculate a hash
		uint32_t hash = FNV_BASIS_32;
		for (typeof(countRawIR) i = 1; i < countRawIR; i++) {
			// Get both values
			auto oldval = dataRawIR[i - 1];
			auto newval = dataRawIR[i];

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
			hash = (hash * FNV_PRIME_32) ^ value;
		}

		// Save calculated hash
		data->command = hash;
	}
}


bool RawIR::requiresReset(void){
	// Used in this protocol
	return true;
}


void RawIR::reset(void){
	// Reset protocol for new reading
	countRawIR = 0;
}


void RawIR::decodeSingle(const uint16_t &duration){
	// Reading timed out
	if(duration >= RAWIR_TIMEOUT){
		// Ignore the very first timeout of each reading.
		if(countRawIR == 1){
			return;
		}
		// Otherwise flag a new input and stop reading.
		else if(countRawIR){
			countRawIR--;
			IRLProtocol = IR_RAW;
		}
		// Start a new reading sequence.
		else{
			countRawIR++;
		}
		return;
	}

	// Only save data if a sequence is running.
	// This is required to avoid corrupted data
	// when starting capturing at the middle of a sequence.
	if(countRawIR){
		// Save value and increase count
		dataRawIR[countRawIR - 1] = duration;
		countRawIR++;

		// Flag a new input if buffer is full
		if((countRawIR > RAWIR_BLOCKS)){
			countRawIR--;
			IRLProtocol = IR_RAW;
		}
	}
}


void RawIR::decode(const uint16_t &duration) {
	// Wait some time after the last protocol.
	// This way it can finish its (possibly ignored) stop bit.
	uint8_t lastProtocol = IRLProtocol | IR_NEW_PROTOCOL;
	if(lastProtocol != IR_RAW && lastProtocol != IR_NEW_PROTOCOL && (IRLLastTime - IRLLastEvent < RAWIR_TIME_THRESHOLD)){
		return;
	}

	decodeSingle(duration);
}
