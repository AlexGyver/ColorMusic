/*
Copyright (c) 2014-2015 NicoHood
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

#include "IRLremote.h"

//================================================================================
// Static Data
//================================================================================

// Protocol temporary data
uint8_t Nec::countNec = 0;
uint8_t Nec::dataNec[NEC_BLOCKS] = { 0 };
volatile uint8_t Nec::protocol = IR_NO_PROTOCOL;
uint32_t Nec::lastTime = 0;
volatile uint32_t Nec::lastEvent = 0;


uint8_t Panasonic::countPanasonic = 0;
uint8_t Panasonic::dataPanasonic[PANASONIC_BLOCKS] = { 0 };
uint8_t Sony::countSony = 0;
uint8_t Sony::dataSony[SONY_BLOCKS_12] = { 0 };
volatile RAWIR_DATA_T RawIR::countRawIR = 0;
volatile uint16_t RawIR::dataRawIR[RAWIR_BLOCKS] = { 0 };
uint16_t HashIR::durationHashIR = 0;
uint16_t HashIR::countHashIR = 0;
uint32_t HashIR::commandHashIR = FNV_BASIS_32;

// Main/shared remote data
volatile uint8_t CIRLData::IRLProtocol = IR_NO_PROTOCOL;
uint32_t CIRLData::IRLLastTime = 0;
volatile uint32_t CIRLData::IRLLastEvent = 0;
