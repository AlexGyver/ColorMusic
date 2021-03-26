/*
Copyright (c) 2014-2018 NicoHood
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

#ifdef ARDUINO
#include <Arduino.h> // micros()
#endif
#include "IRL_Receive.h"
#include "IRL_Time.h"
#include "IRL_Protocol.h"
#include "IRL_Decode.h"

//==============================================================================
// Protocol Definitions
//==============================================================================

//HashIR
#define HASHIR_BLOCKS 255				// 0-65535 (maximum input length)
#define HASHIR_TIMEOUT (0xFFFF/4)		// 65535, max timeout
#define HashIR_TIMESPAN (HASHIR_TIMEOUT * 3)
#define HASHIR_TIME_THRESHOLD 10000UL	// 0-32bit

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619UL
#define FNV_BASIS_32 2166136261UL

typedef uint8_t HashIR_address_t;
typedef uint32_t HashIR_command_t;

// Struct that is returned by the read() function
struct HashIR_data_t
{
    HashIR_address_t address;
    HashIR_command_t command;
};

//==============================================================================
// Hash Decoding Class
//==============================================================================

class CHashIR : public CIRL_Receive<CHashIR>,
             public CIRL_Time<CHashIR>,
             public CIRL_Protocol<CHashIR, HashIR_data_t>
{
public:
    // User API to access library data
    inline bool available(void);
    inline bool receiving(void);

protected:
    static constexpr uint32_t timespanEvent = HashIR_TIMESPAN;

    friend CIRL_Receive<CHashIR>;
    friend CIRL_Protocol<CHashIR, HashIR_data_t>;

    // Interrupt function that is attached
    inline void resetReading(void);
    static inline void interrupt(void);
    static constexpr uint8_t interruptMode = CHANGE;

    // Protocol interface functions
    inline HashIR_data_t getData(void);
    static inline bool checksum(void);
    static inline void holding(void);

    // Protocol variables
    static volatile uint8_t count;
    static uint32_t hash;
    static volatile uint16_t lastDuration;
};


//==============================================================================
// Hash Decoding Implementation
//==============================================================================

HashIR_data_t CHashIR::getData(void){
    // Save address as length.
    // You can check the address/length to prevent triggering on noise
    HashIR_data_t retdata;
    retdata.address = count;
    retdata.command = hash;
    return retdata;
}


bool CHashIR::available(void){
    // First look for a timeout
    receiving();
    bool ret;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        ret = lastDuration == 0;
    }
    return ret;
}


void CHashIR::resetReading(void){
    // Reset reading
    hash = FNV_BASIS_32;
    lastDuration = 0xFFFF;
    count = 0;
}


bool CHashIR::receiving(void)
{
    bool ret = false;

    // Provess with interrupts disabled to avoid any conflicts
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // Check if we already recognized a timed out
        if (count == 0) {
            ret = false;
        }
        else
        {
            // Calculate difference between last interrupt and now
            uint32_t timeout = mlastTime;
            uint32_t time = micros();
            timeout = time - timeout;

            // Check for a new timeout
            if (timeout >= HASHIR_TIMEOUT)
            {
                // Flag new data if we previously received data
                if(count > 1) {
                    count--;
                    lastDuration = 0;
                    mlastEvent = mlastTime;
                }
                else {
                    count = 0;
                }
                ret = false;
            }
            // We are currently receiving
            else {
                ret = true;
            }
        }
    }

    return ret;
}


void CHashIR::interrupt(void)
{
    // Block if the protocol is already recognized
    if (lastDuration == 0) {
        return;
    }

    // Get time between previous call and decode
    auto duration = nextTime();

    // Reading timed out
    if(duration >= HASHIR_TIMEOUT)
    {
        // Start a new reading sequence.
        if(count == 0) {
            count++;
        }
        // Ignore the very first timeout of each reading.
        // Otherwise flag a new input and stop reading.
        else if(count != 1) {
            count--;
            lastDuration = 0;
            mlastEvent = mlastTime;
        }
        return;
    }

    // Only save data if a sequence is running.
    // This is required to avoid corrupted data
    // when starting capturing at the middle of a sequence.
    if(count)
    {
        // Converts the raw code values into a 32-bit hash code.
        // Hopefully this code is unique for each button.
        // This isn't a "real" decoding, just an arbitrary value.
        // Code taken from https://github.com/z3t0/Arduino-IRremote

        // Only compare after the first value got received
        if(count > 1)
        {
            // Get both values
            auto oldval = lastDuration;
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
            hash = (hash * FNV_PRIME_32) ^ value;
        }

        // Save last time and count up
        count++;

        // Flag a new input if buffer is full
        if(count >= HASHIR_BLOCKS){
            lastDuration = 0;
            mlastEvent = mlastTime;
        }
        else {
            lastDuration = duration;
        }
    }
}
