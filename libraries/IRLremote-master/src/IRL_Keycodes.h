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

#include "IRLremote.h"

//==============================================================================
// Protek 9700 Series
//==============================================================================

namespace IRL_Protek_Remote{

    // Protocol: (Extended) NEC
    typedef Nec_address_t IRL_address_t;
    typedef Nec_command_t IRL_command_t;
    typedef Nec_data_t IRL_data_t;
    static const uint16_t IRL_ADDRESS = 0x2222;

    enum IRL_Keycode : uint8_t
    {
        IRL_KEYCODE_POWER           = 0x02,
        IRL_KEYCODE_MUTE            = 0x16,

        IRL_KEYCODE_SCREEN          = 0x2B,
        IRL_KEYCODE_SATELLITE       = 0x2C,
        IRL_KEYCODE_TV_RADIO        = 0x2D,
        IRL_KEYCODE_TV_MUSIC        = 0x2E,

        IRL_KEYCODE_1               = 0x13,
        IRL_KEYCODE_2               = 0x14,
        IRL_KEYCODE_3               = 0x15,
        IRL_KEYCODE_4               = 0x17,
        IRL_KEYCODE_5               = 0x18,
        IRL_KEYCODE_6               = 0x19,
        IRL_KEYCODE_7               = 0x1B,
        IRL_KEYCODE_8               = 0x1C,
        IRL_KEYCODE_9               = 0x1D,
        IRL_KEYCODE_BACK            = 0x0E,
        IRL_KEYCODE_0               = 0x0C,
        IRL_KEYCODE_FAVORITE        = 0x04,

        IRL_KEYCODE_VOL_UP          = 0x20,
        IRL_KEYCODE_VOL_DOWN        = 0x21,
        IRL_KEYCODE_EPG             = 0x08,
        IRL_KEYCODE_INFO            = 0x12,
        IRL_KEYCODE_CHANNEL_UP      = 0x22,
        IRL_KEYCODE_CHANNEL_DOWN    = 0x23,

        IRL_KEYCODE_UP              = 0x09,
        IRL_KEYCODE_DOWN            = 0x07,
        IRL_KEYCODE_LEFT            = 0x0A,
        IRL_KEYCODE_RIGHT           = 0x06,
        IRL_KEYCODE_OK              = 0x11,

        IRL_KEYCODE_EXIT            = 0x10,
        IRL_KEYCODE_MENU            = 0x0F,

        IRL_KEYCODE_I_II            = 0x0D,
        IRL_KEYCODE_TELETEXT        = 0x1F,
        IRL_KEYCODE_SUBTITLE        = 0x01,
        IRL_KEYCODE_ADD             = 0x2F,

        IRL_KEYCODE_RED             = 0x27,
        IRL_KEYCODE_GREEN           = 0x28,
        IRL_KEYCODE_YELLOW          = 0x29,
        IRL_KEYCODE_BLUE            = 0x2A,

        IRL_KEYCODE_PREV            = 0x30,
        IRL_KEYCODE_PLAY            = 0x31,
        IRL_KEYCODE_STOP            = 0x32,
        IRL_KEYCODE_NEXT            = 0x33,
        IRL_KEYCODE_USB             = 0x34,
        IRL_KEYCODE_PAUSE           = 0x35,
        IRL_KEYCODE_REC             = 0x36,
        IRL_KEYCODE_LIVE            = 0x37,
    };
}
