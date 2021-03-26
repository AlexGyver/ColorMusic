/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 IRL Convert Old Nec
 Converts Nec Signals from the old format that Ken used
 with his library to the new bit order. Add your codes
 to the array, upload and open the Serial monitor.
*/

void setup() {
  // Serial setup
  while (!Serial);
  Serial.begin(115200);
  Serial.println(F("Startup"));

  // add your old code here
  uint32_t oldcode[] = {
    0x20DF0FF0,
    0x20DF9E61,
    0x20DFD02F,
    0x20DFA956,
    // ...
  };

  // go thorugh all entries
  for (int i = 0; i < (sizeof(oldcode) / sizeof(uint32_t)); i++) {
    uint32_t newcode = 0;
    uint16_t command = 0;
    uint16_t address = 0;

    // print old code
    Serial.print(oldcode[i], HEX);
    Serial.print(F(" --> 0x"));

    // LSB to MSB
    for (uint8_t j = 0; j < 32; j++) {
      newcode <<= 1;
      newcode |= (oldcode[i] & 0x00000001);
      oldcode[i] >>= 1;
    }

    // new code (address + command)
    address = newcode & 0xFFFF;
    command = (newcode >> 16);

    // print new code
    //Serial.print(newcode, HEX);
    Serial.print(address, HEX);
    Serial.print(F(", 0x"));
    Serial.println(command, HEX);
  }
}

void loop() {
  // empty
}