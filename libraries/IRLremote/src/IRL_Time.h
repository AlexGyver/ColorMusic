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
// IRL_Time Class
//==============================================================================

template<class T>
class CIRL_Time
{
public:
    // User API to access library data
    inline uint32_t timeout(void);
    inline uint32_t lastEvent(void);
    inline uint32_t nextEvent(void);

    // Interface that is required to be implemented
    //static constexpr uint32_t timespanEvent = VALUE;
    //static constexpr uint32_t limitTimeout = VALUE;

protected:
    // Time mangement functions
    static inline uint16_t nextTime(void);

    // Time values for the last interrupt and the last valid protocol
    static uint32_t mlastTime;
    static volatile uint32_t mlastEvent;
};


//==============================================================================
// Static Data
//==============================================================================

// Protocol temporary data
template<class T> uint32_t CIRL_Time<T>::mlastTime = 0;
template<class T> volatile uint32_t CIRL_Time<T>::mlastEvent = 0;


//==============================================================================
// CIRL_Time Implementation
//==============================================================================

/*
 * Returns duration between last interrupt and current time.
 * This will safe the last interrupt time to the current time.
 */
template<class T>
uint16_t CIRL_Time<T>::nextTime(void){
    // Save the duration between the last reading
    uint32_t time = micros();
    uint32_t duration_32 = time - mlastTime;
    mlastTime = time;

    // Calculate 16 bit duration. On overflow sets duration to a clear timeout
    uint16_t duration = duration_32;
    if (duration_32 > 0xFFFF) {
        duration = 0xFFFF;
    }

    return duration;
}


/*
 * Return relativ time between last event time (in micros)
 */
template<class T>
uint32_t CIRL_Time<T>::timeout(void)
{
    uint32_t timeout;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        timeout = mlastEvent;
    }

    uint32_t time = micros();
    timeout = time - timeout;

    return timeout;
}


/*
 * Return absolute last event time (in micros)
 */
template<class T>
uint32_t CIRL_Time<T>::lastEvent(void)
{
    uint32_t time;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        time = mlastEvent;
    }

    return time;
}


/*
 * Return when the next event can be expected.
 * Zero means at any time.
 * Attention! This value is a little bit too high in general.
 * Also for the first press it is even higher than it should.
 */
template<class T>
uint32_t CIRL_Time<T>::nextEvent(void)
{
    auto time = timeout();
    auto timespan = static_cast<T*>(this)->timespanEvent;

    if(time >= timespan) {
        return 0;
    }

    return timespan - time;
}
