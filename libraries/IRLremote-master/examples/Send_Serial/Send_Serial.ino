/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Send Serial
 Sends IR signals on any pin. This uses Bitbanging.
 
 Write anything to the Serial port and hit enter to send Data.
 Turn interrupts off to get a better result if needed
 */

#include "IRLremote.h"

// choose any pin to send IR signals
const int pinSendIR = 3;

void setup() {
  // start serial debug in/output
  Serial.begin(115200);
  Serial.println("Startup");
}

void loop() {
  if (Serial.available()) {
    // discard all Serial bytes to avoid multiple sendings
    delay(10);
    while (Serial.available())
      Serial.read();
      
    // send the data, no pin setting to OUTPUT needed
    Serial.println("Sending...");
    uint16_t address = 0x6361;
    uint32_t command = 0xFE01;

    IRLwrite<IR_NEC>(pinSendIR, address, command);
  }
}