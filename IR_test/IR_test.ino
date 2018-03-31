#include "IRremote.h"
IRrecv irrecv(2); // указываем вывод, к которому подключен приемник
decode_results results;

void setup() {
  Serial.begin(9600); // выставляем скорость COM порта
  irrecv.enableIRIn(); // запускаем прием
}

void loop() {
  if ( irrecv.decode( &results )) {   // если данные пришли
    Serial.print("0x");
    Serial.println(results.value, HEX);
    irrecv.resume();  // принимаем следующую команду
  }
}
