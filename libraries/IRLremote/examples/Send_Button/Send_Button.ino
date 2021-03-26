/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Send Button
 Sends IR signals on any pin. This uses Bitbanging.
 
 Press the button to send data.
 Turn interrupts off to get a better result if needed
 */

#include "IRLremote.h"

// choose any pin to send IR signals
const int pinSendIR = 3;

// choose any pin to trigger IR sending
const int pinButton = 8;

void setup() {
  // setup for the button
  pinMode(pinButton, INPUT_PULLUP);
}

void loop() {
  if (!digitalRead(pinButton)) {
    // send the data, no pin setting to OUTPUT needed
    uint16_t address = 0x6361;
    uint32_t command = 0xFE01;
    IRLwrite<IR_NEC>(pinSendIR, address, command);

    // simple debounce
    delay(300);
  }
}
