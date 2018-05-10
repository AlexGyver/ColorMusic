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

#include "IRL_Platform.h"

//==============================================================================
// CIRL_DecodeSpaces Class
//==============================================================================

template<class T, int blocks>
class CIRL_DecodeSpaces
{
public:
    // User API to access library data
    inline bool available(void);
    inline bool receiving(void);

protected:
    // Temporary buffer to hold bytes for decoding the protocol
    static volatile uint8_t count;
    static uint8_t data[blocks];

    // Interrupt function that is attached
    inline void resetReading(void);
    static void interrupt(void);
    static constexpr uint8_t interruptMode = FALLING;

    // Interface that is required to be implemented
    //static inline bool checksum(void);
    //static inline void holding(void);
    //static constexpr uint32_t limitTimeout = VALUE;
    //static constexpr uint32_t limitLead = VALUE;
    //static constexpr uint32_t limitHolding = VALUE;
    //static constexpr uint32_t limitLogic = VALUE;
    //static constexpr uint32_t limitRepeat = VALUE;
    //static constexpr uint8_t irLength = VALUE;
};


//==============================================================================
// Static Data
//==============================================================================

// Protocol temporary data
template<class T, int blocks>
volatile uint8_t CIRL_DecodeSpaces<T, blocks>::count = 0;
template<class T, int blocks>
uint8_t CIRL_DecodeSpaces<T, blocks>::data[blocks] = { 0 };


//==============================================================================
// CIRL_DecodeSpaces Implementation
//==============================================================================

template<class T, int blocks>
bool CIRL_DecodeSpaces<T, blocks>::available(void){
    return count > (T::irLength / 2);
}


template<class T, int blocks>
void CIRL_DecodeSpaces<T, blocks>::resetReading(void){
    // Reset reading
    count = 0;
}


template<class T, int blocks>
void CIRL_DecodeSpaces<T, blocks>::interrupt(void)
{
    // Block if the protocol is already recognized
    if (count > (T::irLength / 2)) {
        return;
    }

    // Get time between previous call and decode
    auto duration = T::nextTime();

    // On a timeout abort pending readings and start next possible reading
    if (duration >= T::limitTimeout) {
        count = 0;
    }

    // On a reset (error in decoding) wait for a timeout to start a new reading
    // This is to not conflict with other protocols while they are sending 0/1
    // which might be similar to a lead in this protocol
    else if (count == 0) {
        return;
    }

    // Check Mark Lead (requires a timeout)
    else if (count == 1)
    {
        // Wrong lead
        if (duration < T::limitHolding)
        {
            count = 0;
            return;
        }
        // Check for a "button holding" lead
        else if (duration < T::limitLead)
        {
            // Abort if last valid button press is too long ago
            if ((T::mlastTime - \
                T::mlastEvent) >= \
                T::limitRepeat)
            {
                count = 0;
                return;
            }

            // Received a Nec Repeat signal
            // Next mark (stop bit) ignored due to detecting techniques
            T::holding();
            count = (T::irLength / 2);
            T::mlastEvent = T::mlastTime;
        }
        // Else normal lead, continue processing
    }

    // Check different logical space pulses (mark + space)
    else
    {
        // Get number of the Bits (starting from zero)
        // Substract the first lead pulse
        uint8_t length = count - 2;

        // Move bits (MSB is zero)
        data[length / 8] >>= 1;

        // Set MSB if it's a logical one
        if (duration >= T::limitLogic) {
            data[length / 8] |= 0x80;
        }

        // Last bit (stop bit following)
        if (count >= (T::irLength / 2))
        {
            // Check if the protcol's command checksum is correct
            if (T::checksum()) {
                T::mlastEvent = T::mlastTime;
            }
            else {
                count = 0;
                return;
            }
        }
    }

    // Next reading, no errors
    count++;
}

/*
 * Return true if we are currently receiving new data
 */
template<class T, int blocks>
bool CIRL_DecodeSpaces<T, blocks>::receiving(void)
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
            uint32_t timeout = T::mlastTime;
            uint32_t time = micros();
            timeout = time - timeout;

            // Check for a new timeout
            if (timeout >= T::limitTimeout) {
                count = 0;
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
