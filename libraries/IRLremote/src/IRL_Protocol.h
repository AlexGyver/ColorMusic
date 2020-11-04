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
// IRL_Protocol Class
//==============================================================================

template<class T, class Protocol_data_t>
class CIRL_Protocol
{
public:
    // User API to access library data
    Protocol_data_t read(void);

protected:
    // Interface that is required to be implemented
    //inline Nec_data_t getData(void);
    //inline void resetReading(void);
};


//==============================================================================
// CIRL_Protocol Implementation
//==============================================================================

template<class T, class Protocol_data_t>
Protocol_data_t CIRL_Protocol<T, Protocol_data_t>::read(void)
{
    // If nothing was received return an empty struct
    Protocol_data_t retdata = Protocol_data_t();

    // Disable interrupts while accessing volatile data
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // Check and get data if we have new.
        if (static_cast<T*>(this)->available())
        {
            // Set last ISR to current time.
            // This is required to not trigger a timeout afterwards
            // and read corrupted data. This might happen
            // if the reading loop is too slow.
            static_cast<T*>(this)->mlastTime = micros();

            // Save the protocol data
            retdata = static_cast<T*>(this)->getData();

            // Reset reading
            static_cast<T*>(this)->resetReading();
        }
    }

    // Return the new protocol information to the user
    return retdata;
}
