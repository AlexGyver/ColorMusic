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

// IDE version check
#if defined(ARDUINO) && ARDUINO < 10606
#error IRLremote requires Arduino IDE 1.6.6 or greater. Please update your IDE.
#endif

// Software version
#define IRL_VERSION 202

// Delay_basic is only for avrs. With ARM sending is currently not possible
// TODO implement sending
#ifdef ARDUINO_ARCH_AVR
#include <util/delay_basic.h>
#endif

#include "IRL_Platform.h"

// Include all protocol implementations
#include "IRL_Nec.h"
#include "IRL_NecAPI.h"
#include "IRL_Panasonic.h"
#include "IRL_Hash.h"

// Include pre recorded IR codes from IR remotes
#include "IRL_Keycodes.h"
