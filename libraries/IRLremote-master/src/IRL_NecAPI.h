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

#include "IRL_Nec.h"

//==============================================================================
// API Class
//==============================================================================

typedef void(*NecEventCallback)(void);
#define NEC_API_PRESS_TIMEOUT (500UL * 1000UL) // 0.5 seconds

template<const NecEventCallback callback, const uint16_t address = 0x0000>
class CNecAPI : public CNec
{
public:
    // User API to access library data
    inline void read(void);
    inline uint8_t command(void);
    inline uint8_t count(void);
    inline uint8_t duration(bool raw = false);
    inline uint8_t released(bool samebutton = false);
    inline constexpr uint32_t getTimeout(void);
    inline uint32_t nextTimeout(void);

protected:
    // Differenciate between timeout types
    enum TimeoutType : uint8_t
    {
        NO_TIMEOUT,     // Keydown
        TIMEOUT,         // Key release with timeout
        NEXT_BUTTON,     // Key release, pressed again
        NEW_BUTTON,     // Key release, another key is pressed
    } NecTimeoutType;

    // Keep track which key was pressed/held down how often
    uint8_t lastCommand = 0;
    uint8_t lastPressCount = 0;
    uint8_t lastHoldCount = 0;
};

//==============================================================================
// API Class Implementation
//==============================================================================

// Reads data from the nec protocol (if available) and processes it.
template<const NecEventCallback callback, const uint16_t address>
void CNecAPI<callback, address>::read(void) {
  auto data = CNec::read();

  // Check if the correct protocol and address (optional) is used
  bool firstCommand = data.address != 0xFFFF;
  if ((data.address == 0) || (address && firstCommand && (data.address != address)))
  {
    // Call the remote function again once the keypress timed out
    if (lastPressCount && (timeout() > getTimeout()))
    {
      // Flag timeout event, key was released and the current chain is over
      NecTimeoutType = TIMEOUT;
      callback();

      // Reset the button press and hold count after a timeout
      lastPressCount = 0;
      lastHoldCount = 0;
    }
    return;
  }

  // Count the first button press
  if (firstCommand)
  {
    // The same button was pressed twice in a short timespawn (500ms)
    if (data.command == lastCommand)
    {
      // Flag that the last button hold is over, the same key is held down again
      if (lastPressCount) {
        NecTimeoutType = NEXT_BUTTON;
        callback();
      }

      // Increase pressing streak
      if (lastPressCount < 255) {
        lastPressCount++;
      }
    }
    // Different button than before
    else
    {
      // Flag that the last button hold is over, a differnt key is now held down
      if (lastPressCount) {
        NecTimeoutType = NEW_BUTTON;
        callback();
      }
      lastPressCount = 1;
    }

    // Start a new series of button holding
    lastHoldCount = 0;

    // Save the new command. On a repeat (below) don't safe it.
    lastCommand = data.command;
  }
  // Count the button holding
  else
  {
    // Abort if no first press was recognized (after reset)
    if(!lastPressCount){
        return;
    }

    // Increment holding count
    if (lastHoldCount < 255) {
      lastHoldCount++;
    }
  }

  // Call the remote function and flag that the event was just received
  NecTimeoutType = NO_TIMEOUT;
  callback();
}


template<const NecEventCallback callback, const uint16_t address>
uint8_t CNecAPI<callback, address>::command(void)
{
    return lastCommand;
}

// Number of repeating button presses in a row
template<const NecEventCallback callback, const uint16_t address>
uint8_t CNecAPI<callback, address>::count(void)
{
    return lastPressCount;
}

// Duration (count) how long the current button press was held down.
// Pass true to also recognize keyup events
template<const NecEventCallback callback, const uint16_t address>
uint8_t CNecAPI<callback, address>::duration(bool raw)
{
    // Only recognize the actual keydown event
    if (NecTimeoutType == NO_TIMEOUT || raw) // TODO reorder?
    {
        return 1 + lastHoldCount;
    }
    return 0;
}

// True when the button is released:
// 1. Timeout of press series
// 2. Next press, new button
// 3. Next press, same button
// `--> pass true as parameter,
//      useful when measuring idential or single button press durations
// False when the button is held down:
// 1. Initial button press
// 2. Holding button down
// 3. Renewed button press
// Usually you want to use timeout() to check if the series ends
template<const NecEventCallback callback, const uint16_t address>
uint8_t CNecAPI<callback, address>::released(bool samebutton)
{
    if (NecTimeoutType == TIMEOUT || NecTimeoutType == NEW_BUTTON) {
        return 1 + lastHoldCount;
    }
    if (samebutton && NecTimeoutType == NEXT_BUTTON) {
        return 1 + lastHoldCount;
    }
    return 0;
}

template<const NecEventCallback callback, const uint16_t address>
constexpr uint32_t CNecAPI<callback, address>::getTimeout(void) {
    return NEC_API_PRESS_TIMEOUT;
}

// Return when the next timeout triggers.
// Zero means it already timed out.
template<const NecEventCallback callback, const uint16_t address>
uint32_t CNecAPI<callback, address>::nextTimeout(void)
{
    auto time = timeout();
    auto timeout = getTimeout();

    if(time >= timeout) {
        return 0;
    }

    return timeout - time;
}
