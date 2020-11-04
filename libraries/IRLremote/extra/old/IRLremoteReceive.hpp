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
// User Functions
//================================================================================

template<typename protocol, typename ...protocols>
CIRLremote<protocol, protocols...>::
CIRLremote(void) {
	// Empty
}


template<typename protocol, typename ...protocols>
bool CIRLremote<protocol, protocols...>::
begin(uint8_t pin)
{
	// Get pin ready for reading.
	// This is normally not required with a static setup
	// But for dynamic plugging it is.
	pinMode(pin, INPUT_PULLUP);

	// For single protocols use a different flag
	uint8_t flag = CHANGE;
	if(sizeof...(protocols) == 0){
		flag = protocol::getSingleFlag();
	}

	// Try to attach PinInterrupt first
	if (digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT){
		attachInterrupt(digitalPinToInterrupt(pin), interrupt, flag);
		return true;
	}

	// If PinChangeInterrupt library is used, try to attach it
#ifdef PCINT_VERSION
	else if (digitalPinToPCINT(pin) != NOT_AN_INTERRUPT){
		attachPCINT(digitalPinToPCINT(pin), interrupt, flag);
		return true;
	}
#endif

	// Return an error if none of them work (pin has no Pin(Change)Interrupt)
	return false;
}


template<typename protocol, typename ...protocols>
bool CIRLremote<protocol, protocols...>::
end(uint8_t pin)
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


template<typename protocol, typename ...protocols>
bool CIRLremote<protocol, protocols...>::
available(void)
{
	// Only add this overhead if we have multiple protocols
	// Or the protocol requires a timeout check.
	if(sizeof...(protocols) != 0 || protocol::requiresCheckTimeout())
	{
		// Disable interrupts when checking for new input
		uint8_t oldSREG = SREG;
		cli();

		// Let each protocol check if their timeout expired. Not all protocols use this.
		if(!(IRLProtocol & IR_NEW_PROTOCOL)){
			protocol::checkTimeout();
			nop((protocols::checkTimeout(), 0)...);
		}
		SREG = oldSREG;
	}

	// This if construct saves flash
	if(IRLProtocol & IR_NEW_PROTOCOL)
		return true;
	else
		return false;
}


template<typename protocol, typename ...protocols>
IR_data_t CIRLremote<protocol, protocols...>::
read(void)
{
	// If nothing was received return an empty struct
	IR_data_t data = IR_data_t();

	// Only the received protocol will write data into the struct
	uint8_t oldSREG = SREG;
	cli();

	// Get new data, if any
	if(IRLProtocol & IR_NEW_PROTOCOL)
	{
		// Check if we actually have new data and save the protocol as well
		data.address = ((uint16_t)dataNec[1] << 8) | ((uint16_t)dataNec[0]);
		data.command = ((uint16_t)dataNec[3] << 8) | ((uint16_t)dataNec[2]);
		data.protocol = IRLProtocol;

		// Remove new protocol flag
		IRLProtocol &= ~IR_NEW_PROTOCOL;

		// Set last ISR to current time.
		// This is required to not trigger a timeout afterwards
		// and read corrupted data. This might happen
		// if the reading loop is too slow.
		IRLLastTime = micros();
	}
	SREG = oldSREG;

	// Process API TODO

	// Return the new protocol information to the user
	return data;
}


template<typename protocol, typename ...protocols>
void CIRLremote<protocol, protocols...>::
reset(void)
{
	// Call all protocol reset functions
	protocol::reset();
	nop((protocols::reset(), 0)...);
}


//================================================================================
// Interrupt Function
//================================================================================

template<typename protocol, typename ...protocols>
void CIRLremote<protocol, protocols...>::
interrupt(void)
{
	// Block if the protocol is already recognized
	if(IRLProtocol & IR_NEW_PROTOCOL)
		return;

	// Save the duration between the last reading
	uint32_t time = micros();
	uint32_t duration_32 = time - IRLLastTime;
	IRLLastTime = time;

	// Calculate 16 bit duration. On overflow sets duration to a clear timeout
	uint16_t duration = 0xFFFF;
	if (duration_32 <= 0xFFFF)
		duration = duration_32;

		// For a single protocol use a simpler decode function
		// to get maximum speed + recognition and minimum flash size
		protocol::decodeSingle(duration);

	// New valid signal, save new time
	if(IRLProtocol & IR_NEW_PROTOCOL) {
		IRLLastEvent = IRLLastTime;
	}
}
