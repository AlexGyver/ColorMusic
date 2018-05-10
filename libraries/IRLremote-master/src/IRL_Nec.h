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

#include "IRL_Receive.h"
#include "IRL_Time.h"
#include "IRL_Protocol.h"
#include "IRL_Decode.h"

//==============================================================================
// Protocol Definitions
//==============================================================================

// NEC
// IRP notation:
// {38.4k,564}<1,-1|1,-3>(16,-8,D:8,S:8,F:8,~F:8,1,-78,(16,-4,1,-173)*)
// Lead + Space logic
#define NEC_HZ                38000UL
#define NEC_PULSE             564UL
#define NEC_ADDRESS_LENGTH    16
#define NEC_COMMAND_LENGTH    16
#define NEC_DATA_LENGTH       (NEC_ADDRESS_LENGTH + NEC_COMMAND_LENGTH)
#define NEC_BLOCKS            (NEC_DATA_LENGTH / 8)
// 2 for lead + space, each block has mark and space
#define NEC_LENGTH            (2 + NEC_DATA_LENGTH * 2)
#define NEC_TIMEOUT           (NEC_PULSE * 78UL)
#define NEC_TIMEOUT_HOLDING   (NEC_PULSE * 173UL)
#define NEC_TIMESPAN_HOLDING  (NEC_TIMEOUT_HOLDING + NEC_LOGICAL_HOLDING)
#define NEC_MARK_LEAD         (NEC_PULSE * 16UL)
#define NEC_MARK_HOLDING      (NEC_PULSE * 16UL)
#define NEC_SPACE_LEAD        (NEC_PULSE * 8UL)
#define NEC_SPACE_HOLDING     (NEC_PULSE * 4UL)
#define NEC_LOGICAL_LEAD      (NEC_MARK_LEAD + NEC_SPACE_LEAD)
#define NEC_LOGICAL_HOLDING   (NEC_MARK_LEAD + NEC_SPACE_HOLDING)
#define NEC_MARK_ZERO         (NEC_PULSE * 1UL)
#define NEC_MARK_ONE          (NEC_PULSE * 1UL)
#define NEC_SPACE_ZERO        (NEC_PULSE * 1UL)
#define NEC_SPACE_ONE         (NEC_PULSE * 3UL)
#define NEC_LOGICAL_ZERO      (NEC_MARK_ZERO + NEC_SPACE_ZERO)
#define NEC_LOGICAL_ONE       (NEC_MARK_ONE + NEC_SPACE_ONE)

// Decoding limits
#define NEC_LIMIT_LOGIC       ((NEC_LOGICAL_ONE + NEC_LOGICAL_ZERO) / 2)
#define NEC_LIMIT_HOLDING     ((NEC_LOGICAL_HOLDING + NEC_LOGICAL_ONE) / 2)
#define NEC_LIMIT_LEAD        ((NEC_LOGICAL_LEAD + NEC_LOGICAL_HOLDING) / 2)
#define NEC_LIMIT_TIMEOUT     ((NEC_TIMEOUT + NEC_LOGICAL_LEAD) / 2)
#define NEC_LIMIT_REPEAT      (NEC_TIMESPAN_HOLDING * 3 / 2)

/*
 * Nec pulse demonstration:
 *
 *---|                |--------| |---| |-|   ... -| |----------/ ~ /----------|
 *   |                |        | |   | | |   ...  | |                         |
 *   |                |        | |   | | |   ...  | |                         |
 *   |----------------|        |-|   |-| |-  ...  |-|                         |
 *   |          Lead           |Log 1|Lg0|  Data  |E|         Timeout         |-

 *---|                |----| |---------------------/ ~ /----------------------|
 *   |                |    | |                                                |
 *   |                |    | |                                                |
 *   |----------------|    |-|                                                |
 *   |      Holding        |E|                Timeout Holding                 |-
 *   |                            Timespan Holding                            |
 *
 *  2 Pulses: |-+-| Logical 0
 *  3 Pulses: |----| limitLogic
 *  4 Pulses: |-+---| Logical 1
 * 12 Pulses: |-------------| limitHolding
 * 20 Pulses: |----------------+----| Holding
 * 22 Pulses: |-----------------------| limitLead
 * 24 Pulses: |----------------+--------| Lead
 * 51 Pulses: |-----------------/ ~ /-----------------| limitTimeout
 * 78 Pulses: |-------------------------/ ~ /-------------------------| Timeout
 */

typedef uint16_t Nec_address_t;
typedef uint8_t Nec_command_t;

// Struct that is returned by the read() function
struct Nec_data_t
{
    Nec_address_t address;
    Nec_command_t command;
};

//==============================================================================
// Nec Decoding Class
//==============================================================================

class CNec : public CIRL_Receive<CNec>,
             public CIRL_Time<CNec>,
             public CIRL_Protocol<CNec, Nec_data_t>,
             public CIRL_DecodeSpaces<CNec, NEC_BLOCKS>
{
protected:
    static constexpr uint32_t timespanEvent = NEC_TIMESPAN_HOLDING;
    static constexpr uint32_t limitTimeout = NEC_LIMIT_TIMEOUT;
    static constexpr uint32_t limitLead = NEC_LIMIT_LEAD;
    static constexpr uint32_t limitHolding = NEC_LIMIT_HOLDING;
    static constexpr uint32_t limitLogic = NEC_LIMIT_LOGIC;
    static constexpr uint32_t limitRepeat = NEC_LIMIT_REPEAT;
    static constexpr uint8_t irLength = NEC_LENGTH;

    friend CIRL_Receive<CNec>;
    friend CIRL_Protocol<CNec, Nec_data_t>;
    friend CIRL_DecodeSpaces<CNec, NEC_BLOCKS>;

    // Protocol interface functions
    inline Nec_data_t getData(void);
    static inline bool checksum(void);
    static inline void holding(void);
};


//==============================================================================
// Nec Decoding Implementation
//==============================================================================

Nec_data_t CNec::getData(void){
    Nec_data_t retdata;
    retdata.address = ((uint16_t)data[1] << 8) | ((uint16_t)data[0]);
    retdata.command = data[2];
    return retdata;
}


bool CNec::checksum(void) {
    return uint8_t((data[2] ^ (~data[3]))) == 0x00;
}


void CNec::holding(void) {
    // Flag repeat signal via "invalid" address and empty command
    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0x00;
}
