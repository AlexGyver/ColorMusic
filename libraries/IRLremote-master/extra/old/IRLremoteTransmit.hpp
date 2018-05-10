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

// include guard
#pragma once

//================================================================================
// Definitions
//================================================================================

// definition for sending to determine what should be send first
#define IR_ADDRESS_FIRST true
#define IR_COMMAND_FIRST false

// definition to convert an uint8_t array to an uint16_t/uint32_t at any position (thx timeage!)
#define UINT16_AT_OFFSET(p_to_8, offset)    ((uint16_t)*((const uint16_t *)((p_to_8)+(offset))))
#define UINT32_AT_OFFSET(p_to_8, offset)    ((uint32_t)*((const uint32_t *)((p_to_8)+(offset))))

// definition to get the higher value
#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

// definition to get the lower value
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


//================================================================================
// Inline Implementations (send)
//================================================================================

template <IRType irType>
void IRLwrite(const uint8_t pin, uint16_t address, uint32_t command)
{
	// get the port mask and the pointers to the out/mode registers for faster access
	uint8_t bitMask = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);
	volatile uint8_t * outPort = portOutputRegister(port);
	volatile uint8_t * modePort = portModeRegister(port);

	// set pin to OUTPUT and LOW
	*modePort |= bitMask;
	*outPort &= ~bitMask;

	// disable interrupts
	uint8_t oldSREG = SREG;
	cli();

	switch (irType) {

	case IR_NEC:
		// NEC only sends the data once
		if (command == 0xFFF)
			// send holding indicator
			IRLsend<0, 0, NEC_HZ, 0, NEC_MARK_LEAD, NEC_SPACE_HOLDING,
			        0, 0, 0, 0>
			        (outPort, bitMask, address, command);
		else
			// send data
			IRLsend<NEC_ADDRESS_LENGTH, NEC_COMMAND_LENGTH, NEC_HZ, IR_ADDRESS_FIRST, NEC_MARK_LEAD, NEC_SPACE_LEAD,
			        NEC_MARK_ZERO, NEC_MARK_ONE, NEC_SPACE_ZERO, NEC_SPACE_ONE>
			        (outPort, bitMask, address, command);
		break;

	case IR_PANASONIC: //TODO test
		// send data
		IRLsend<PANASONIC_ADDRESS_LENGTH, PANASONIC_COMMAND_LENGTH, PANASONIC_HZ, IR_ADDRESS_FIRST, PANASONIC_MARK_LEAD, PANASONIC_SPACE_LEAD,
		        PANASONIC_MARK_ZERO, PANASONIC_MARK_ONE, PANASONIC_SPACE_ZERO, PANASONIC_SPACE_ONE>
		        (outPort, bitMask, address, command);
		break;

	case IR_SONY12: //TODO test, address -1?
		// repeat 3 times
		for (uint8_t i = 0; i < 3; i++)
			// send data
			IRLsend<SONY_ADDRESS_LENGTH_12, SONY_COMMAND_LENGTH_12, SONY_HZ, IR_COMMAND_FIRST, SONY_MARK_LEAD, SONY_SPACE_LEAD,
			        SONY_MARK_ZERO, SONY_MARK_ONE, SONY_SPACE_ZERO, SONY_SPACE_ONE>
			        (outPort, bitMask, address, command);
		break;
	}

	// enable interrupts
	SREG = oldSREG;

	// set pin to INPUT again to be save
	*modePort &= ~bitMask;
}

// multifunctional template for sending
template <uint8_t addressLength, uint8_t commandLength,
          uint16_t Hz, bool addressFirst,
          uint16_t markLead, uint16_t spaceLead,
          uint16_t markZero, uint16_t markOne,
          uint16_t spaceZero, uint16_t spaceOne>
void IRLsend(volatile uint8_t * outPort, uint8_t bitmask, uint16_t address, uint32_t command) {

	// send lead mark
	if (markLead)
		IRLmark(Hz, outPort, bitmask, markLead);

	// send lead space
	if (spaceLead)
		IRLspace(outPort, bitmask, spaceLead);

	// abort if input values are both the same (sending a holding signal for example)
	if (markOne != markZero || spaceOne != spaceZero)
	{
		// go through all bits
		for (uint8_t i = 0; i < (addressLength + commandLength); i++) {
			// determine if its a logical one or zero, starting with address or command
			bool bitToSend;
			if (addressFirst && i < addressLength || !addressFirst && i >= commandLength) {
				bitToSend = address & 0x01;
				address >>= 1;
			}
			else {
				bitToSend = command & 0x01;
				command >>= 1;
			}

			// send logic mark bits if needed
			if (markOne != markZero) {
				// modulate if spaces dont have logic, else only every even number
				if (spaceOne == spaceZero || spaceOne != spaceZero && i % 2 == 0)
				{
					if (bitToSend)
						IRLmark(Hz, outPort, bitmask, markOne);
					else
						IRLmark(Hz, outPort, bitmask, markZero);
				}
			}
			else
				IRLmark(Hz, outPort, bitmask, markOne);

			// send logic space bits if needed
			if (spaceOne != spaceZero) {
				// modulate if marks dont have logic, else only every odd number
				if (markOne == markZero || markOne != markZero && i % 2 == 1) {
					if (bitToSend)
						IRLspace(outPort, bitmask, spaceOne);
					else
						IRLspace(outPort, bitmask, spaceZero);
				}
			}
			else
				IRLspace(outPort, bitmask, spaceOne);
		}
	}

	// finish mark
	IRLmark(Hz, outPort, bitmask, markZero); //TODO zero or one for sony?
	IRLspace(outPort, bitmask, 0);
}

void IRLmark(const uint16_t Hz, volatile uint8_t * outPort, uint8_t bitMask, uint16_t time) {
	/*
	Bitbangs PWM in the given Hz number for the given time
	________________________________________________________________________________
	Delay calculation:
	F_CPU/1.000.000 to get number of cycles/uS
	/3 to get the number of loops needed for 1ms (1loop = 3 cycles)

	Multiply with the number of ms delay:
	1/kHz to get the seconds
	* 1.000.000 to get it in uS
	/2 to get half of a full pulse

	Substract the while, portmanipulation, loop overhead /3 loop cycles

	F_CPU(16.000.000)            1 * 1.000.000(pulse in uS)   12(overhead)
	========================== * ========================== - ==============
	1.000.000 * 3(loop cycles)   Hz * 2(half of a pulse)      3(loop cycles)

	<==>

	F_CPU(16.000.000) - (12(overhead) * Hz * 2(half of a pulse))
	===========================================================
	Hz * 2(half of a on/off pulse) * 3(loop cycles)

	________________________________________________________________________________
	Iterations calculation:
	Devide time with cycles in while loop
	Multiply this with the cycles per uS
	cycles per while loop: 3(loop cycles) * delay + overhead

	time * (F_CPU(16.000.000) / 1.000.000)
	======================================
	delay*3(loop cycles) + overhead
	*/

	const uint32_t loopCycles = 3;
	const uint32_t overHead = 12; // just a guess from try + error
	uint8_t delay = (F_CPU - (overHead * Hz * 2UL)) / (Hz * 2UL * loopCycles);
	uint16_t iterations = (time * (F_CPU / 1000000UL)) / (delay * loopCycles + overHead);

	// modulate IR signal
	while (iterations--) {
		// flip pin state and wait for the calculated time
		*outPort ^= bitMask;
#ifdef ARDUINO_ARCH_AVR
		// loop
		_delay_loop_1(delay);
#else
		// ARM TODO?
#endif
	}

	// register uint8_t temp;
	// asm volatile (
	//     // flip pin state
	//     ".L%=_iteration_loop:\n"
	//     "ld %[temp], %a[outPort]\n" // (2) read the pin (happens before the 2 cycles)
	//     "eor %[temp], %[bitMask]\n" // (1) XOR value
	//     "st %a[outPort],%[temp]\n"	// (2) flip pin state TODO happens before or after the 2 cycles

	//     // wait some time to modulate the Hz
	//     "mov %[temp],%[delay]\n"	// (1) load delay value into temp
	//     ".L%=_delay_loop:\n"
	//     "dec %[temp]\n"				// (1) decrement 1 from delay counter
	//     "brne .L%=_delay_loop\n"	// (1/2) wait until delay is over

	//     // check if all pulses have been sent

	//     //TODO set pin low
	//     "eor %[bitMask], %[bitMask]\n" // (1) XOR value
	//     "ld %[temp], %a[outPort]\n" // (2) read the pin (happens before the 2 cycles)
	//     "and %[temp], %[bitMask]\n" // (1) AND value
	//     "st %a[outPort],%[temp]\n"	// (2) set pin low TODO happens before or after the 2 cycles

	//     //MOVF    NumL, F; Test lo byte
	//     //BTFSC   STATUS, Z; Skip if not zero
	//     //DECF    NumH, F; Decrement hi byte
	//     //DECF    NumL, F; Decrement lo byte

	//     // decrement low byte
	//     // check if low byte is 255
	//     // if yes decrement high byte
	//     // check if high byte is zero, if yes and function
	//     // if no decrement
	//     // jump to loop above
	//     // end of loop

	//     // ----------
	//     // outputs:
	//     : [outPort] "+e" (outPort), // (read and write)
	//     [temp] "=&r" (temp), // (output only)

	//     // inputs:
	//     : [delay] "r" (delay),
	//     [bitMask] "r" (bitMask)

	//     // no clobbers
	// ); // end of asm volatile
}
//}
//
//	// abort if input values are both the same (sending a holding signal for example)
//	if (markOne != markZero || spaceOne != spaceZero)
//	{
//		// go through all bits
//		for (uint8_t i = 0; i < (addressLength + commandLength); i++){
//     5f4:	8c e2       	ldi	r24, 0x2C	; 44
//     5f6:	90 e0       	ldi	r25, 0x00	; 0
//     5f8:	01 97       	sbiw	r24, 0x01	; 1
//	const uint32_t overHead = 12; // just a guess from try + error
//	uint8_t delay = (F_CPU - (overHead * Hz * 2UL)) / (Hz * 2UL * loopCycles);
//	uint16_t iterations = (time*(F_CPU / 1000000UL)) / (delay * loopCycles + overHead);
//
//	// modulate IR signal
//	while (iterations--){
//     5fa:	39 f0       	breq	.+14     	; 0x60a <loop+0x204>
//		// flip pin state and wait for the calculated time
//		*outPort ^= bitMask;
//
//     //5fc:	28 81       	ld	r18, Y
//     //5fe:	27 25       	eor	r18, r7
//     //600:	28 83       	st	Y, r18
//
//     602:	25 2d       	mov	r18, r5
//     604:	2a 95       	dec	r18
//     606:	f1 f7       	brne	.-4      	; 0x604 <loop+0x1fe>
//     608:	f7 cf       	rjmp	.-18     	; 0x5f8 <loop+0x1f2>
//
//	 const uint32_t loopCycles = 3;
//	 const uint32_t overHead = 12; // just a guess from try + error
//	 uint8_t delay = (F_CPU - (overHead * Hz * 2UL)) / (Hz * 2UL * loopCycles);
//	 uint16_t iterations = (time*(F_CPU / 1000000UL)) / (delay * loopCycles + overHead);
//
//	 // modulate IR signal
//	 while (iterations--){
//		 // flip pin state and wait for the calculated time
//		 *outPort ^= bitMask;
//		 //5fc:	28 81       	ld	r18, Y
//		 //5fe:	27 25       	eor	r18, r7
//		 //600:	28 83       	st	Y, r18
//
//#ifdef ARDUINO_ARCH_AVR
//		 // loop
//		 _delay_loop_1(delay);
//#else
//		 // ARM TODO?
//#endif
//	 }
//	}

void IRLspace(volatile uint8_t * outPort, uint8_t bitMask, uint16_t time) {
	// Sends an IRLspace for the specified number of microseconds.
	// A IRLspace is no output, so the PWM output is disabled.
	*outPort &= ~bitMask; // write pin LOW
	delayMicroseconds(time);
	//_delay_loop_2(time*(F_CPU/1000000UL)/4UL);
}

//
//template<const uint16_t Hz>
//void IRLsend(volatile uint8_t* outPort, volatile uint8_t* inPort uint8_t bitMask, uint16_t timeMark, uint16_ timeSpace){
//
//	uint16_t iterations = timeMark;
//
//	while (iterations){
//		*inPort |= bitMask;
//	}
//}

