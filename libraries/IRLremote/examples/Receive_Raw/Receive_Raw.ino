/*
  Copyright (c) 2014-2015 NicoHood
  See the readme for credit to other people.

  IRL Receive_Raw

  Receives IR signals and prints raw values to the Serial.
  Please read the notes below, this example is used for developing/debugging only.

  The following pins are usable for PinInterrupt or PinChangeInterrupt*:
  Arduino Uno/Nano/Mini: 2, 3, All pins* are usable
  Arduino Mega: 2, 3, 18, 19, 20, 21,
               10*, 11*, 12*, 13*, 50*, 51*, 52*, 53*, A8 (62)*, A9 (63)*, A10 (64)*,
               A11 (65)*, A12 (66)*, A13 (67)*, A14 (68)*, A15 (69)*
  Arduino Leonardo/Micro: 0, 1, 2, 3, 7, 8*, 9*, 10*, 11*, 14 (MISO)*, 15 (SCK)*, 16 (MOSI)*
  HoodLoader2: All (broken out 1-7*) pins are usable
  Attiny 24/44/84: 8, All pins* are usable
  Attiny 25/45/85: 2, All pins* are usable
  Attiny 13: All pins* are usable
  ATmega644P/ATmega1284P: 10, 11, All pins* are usable

  PinChangeInterrupts* requires a special library which can be downloaded here:
  https://github.com/NicoHood/PinChangeInterrupt
*/

// include PinChangeInterrupt library* BEFORE IRLremote to acces more pins if needed
//#include "PinChangeInterrupt.h"

#include "IRLremote.h"
// choose a valid PinInterrupt or PinChangeInterrupt* pin of your Arduino board
#define pinIR 2
CIRLremote<RawIR> IRLremote;

#define pinLed LED_BUILTIN

void setup() {
  // Start serial debug output
  while (!Serial);
  Serial.begin(115200);
  Serial.println(F("Startup"));

  // Set LED to output
  pinMode(pinLed, OUTPUT);

  // Start reading the remote. PinInterrupt or PinChangeInterrupt* will automatically be selected
  if (!IRLremote.begin(pinIR))
    Serial.println(F("You did not choose a valid pin."));
}

void loop() {
  if (IRLremote.available()) {
    // Light Led
    digitalWrite(pinLed, HIGH);

    // Important note:
    // Printing the values might cause you to miss
    // the first byte of the next sequence.
    // You might want to increase the buffer size
    // and the timeout as well in the settings.
    // You could also copy the buffer, call read()
    // and print afterwards if enough ram is available.
    // Do not use RawIR as "all day" protocol.
    // But you can use the HashIR protocol instead.
    const bool printRaw = true;
    if (printRaw) {

      // Print a mark
      Serial.println(F("=========="));

      // Go through the whole buffer and print values
      for (typeof(RawIR::countRawIR) i = 0; i < RawIR::countRawIR; i++) {
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println(RawIR::dataRawIR[i], DEC);
      }

      // Each buffer should end with the timeout value
      if (RawIR::countRawIR == RAWIR_BLOCKS) {
        Serial.println(F("Buffer was full!"));
      }
      Serial.println();
    }

    // Get the new data from the remote
    IR_data_t data = IRLremote.read();

    // Print the protocol data
    // For RawIR this is just an approximate calculation.
    // You should ignore addresses/length < 6 to avoid noise.
    Serial.print(F("Buffer length: "));
    Serial.println(data.address, DEC);
    Serial.print(F("Command: 0x"));
    Serial.println(data.command, HEX);
    Serial.println();

    // Turn Led off after printing the data
    digitalWrite(pinLed, LOW);
  }
}
