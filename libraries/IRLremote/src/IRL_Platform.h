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

#if defined(ARDUINO_ARCH_AVR) || defined(DMBS_ARCH_AVR8)
    #include <util/atomic.h>
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    // copied from https://github.com/wizard97/SimplyAtomic/blob/master/esp8266.h

    #ifndef __STRINGIFY
    #define __STRINGIFY(a) #a
    #endif

    #ifndef xt_rsil
        #define xt_rsil(level) (__extension__({uint32_t state; __asm__ __volatile__("rsil %0," __STRINGIFY(level) : "=a" (state)); state;}))
    #endif

    #ifndef xt_wsr_ps
        #define xt_wsr_ps(state)  __asm__ __volatile__("wsr %0,ps; isync" :: "a" (state) : "memory")
    #endif

    static __inline__ void SA_iRestore(const  uint32_t *__s)
    {
        xt_wsr_ps(*__s);
    }

    // Note value can be 0-15, 0 = Enable all interrupts, 15 = no interrupts
    #define SA_ATOMIC_RESTORESTATE uint32_t _sa_saved   \
        __attribute__((__cleanup__(SA_iRestore))) = xt_rsil(15)

    #define ATOMIC_RESTORESTATE
    #define ATOMIC_BLOCK(A) \
        for ( SA_ATOMIC_RESTORESTATE, _sa_done =  1;    \
            _sa_done; _sa_done = 0 )
#else
    #error "This library supports only AVR and ESP8266 Boards."
#endif
