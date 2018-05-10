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
#include <Arduino.h> // pinMode()
#endif

#ifdef DMBS_MODULE_BOARD
#include "board_pins.h"
#endif

#ifdef DMBS_MODULE_FASTPIN
#include "fastpin.h"
#endif

#ifdef DMBS_MODULE_PCINT
#include "pcint.h"
#endif

//==============================================================================
// IRL_Receive Class
//==============================================================================

template<class T>
class CIRL_Receive
{
public:
#ifdef ARDUINO
    // Attach the interrupt so IR signals are detected
    inline bool begin(uint8_t pin);
    inline bool end(uint8_t pin);
#endif

    // Alternative template option
    template<uint8_t pin> inline bool begin(void);
    template<uint8_t pin> inline bool end(void);

protected:
    // Interface that is required to be implemented
    //static inline void interrupt(void);
    //static constexpr uint8_t interruptMode = FALLING|RISING|CHANGE;
};

//==============================================================================
// CIRL_Receive Implementation
//==============================================================================

template<class T>
template<uint8_t pin>
bool CIRL_Receive<T>::begin(void)
{
#ifdef ARDUINO
    return begin(pin);
#else
    // Set pin to INPUT_PULLUP
    FastPin<pin> p;
    p.setInput();
    p.hi();

    // If PinChangeInterrupt library is used, try to attach it
    if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT){
        attachPCINT(digitalPinToPCINT(pin), T::interrupt, T::interruptMode);
        return true;
    }

    // Return an error if none of them work (pin has no Pin(Change)Interrupt)
    return false;
#endif
}

#ifdef ARDUINO
template<class T>
bool CIRL_Receive<T>::begin(uint8_t pin)
{
    // Get pin ready for reading.
    pinMode(pin, INPUT_PULLUP);

    // Try to attach PinInterrupt first
    if (digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT){
        attachInterrupt(digitalPinToInterrupt(pin), T::interrupt, T::interruptMode);
        return true;
    }

    // If PinChangeInterrupt library is used, try to attach it
#ifdef PCINT_VERSION
    else if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT){
        attachPCINT(digitalPinToPCINT(pin), T::interrupt, T::interruptMode);
        return true;
    }
#endif

    // Return an error if none of them work (pin has no Pin(Change)Interrupt)
    return false;
}
#endif

template<class T>
template<uint8_t pin>
bool CIRL_Receive<T>::end(void)
{
#ifdef ARDUINO
    return end(pin);
#else
    // Disable pullup.
    FastPin<pin> p;
    p.lo();

    // If PinChangeInterrupt library is used, try to detach it
    if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT){
        detachPCINT(digitalPinToPCINT(pin));
        return true;
    }

    // Return an error if none of them work (pin has no Pin(Change)Interrupt)
    return false;
#endif
}

#ifdef ARDUINO
template<class T>
bool CIRL_Receive<T>::end(uint8_t pin)
{
    // Disable pullup.
    pinMode(pin, INPUT);

    // Try to detach PinInterrupt first
    if (digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT){
        detachInterrupt(digitalPinToInterrupt(pin));
        return true;
    }

    // If PinChangeInterrupt library is used, try to detach it
#ifdef PCINT_VERSION
    else if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT){
        detachPCINT(digitalPinToPCINT(pin));
        return true;
    }
#endif

    // Return an error if none of them work (pin has no Pin(Change)Interrupt)
    return false;
}
#endif
