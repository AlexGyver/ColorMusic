#define COMMAND_BUFFER_LENGTH 100 // Максикмальная длина команды. MAX: 255

#define CR '\r'
#define LF '\n'
#define BS '\b'
#define NULLCHAR '\0'
#define SPACE ' '

const char *delimiters = ", \n"; // Разделитель команд
char commandLine[COMMAND_BUFFER_LENGTH + 1]; // Буфер для команды

/**
 * hasCommand()
 * Считывает данные с порта в буфер
 * @return bool true - поступила новая команда, false - в буфере пусто
 */
bool hasCommand(){
  static uint8_t charsRead = 0;             

  while (Serial.available()) {
    char c;

    if(Serial.available()){
      c = Serial.read();
    }

    switch (c){
      case CR:      // команда заканчивается символом CR и/или LS
      case LF:
        commandLine[charsRead] = NULLCHAR;  
        if (charsRead > 0)  {
          charsRead = 0;                         
          Serial.println(commandLine);
          return true;
        }
        break;
      default:
        c = tolower(c);
        if (charsRead < COMMAND_BUFFER_LENGTH) {
          commandLine[charsRead++] = c;
        }
        commandLine[charsRead] = NULLCHAR;     //just in case
        break;
    }
  }

  return false;
}

/**
 * getCommand()
 * Получить текст команды
 */
char * getCommand() {
  char * word = strtok(commandLine, delimiters);
  return word;
}

/**
 * getArg()
 * Получить следующий аргумент
 */
char * getArg() {
  char * word = strtok(NULL, delimiters);
  return word;
}

/**
 * getArgInt()
 * Получить следующий числовой аргумент
 */
long getArgInt() {
  char * numTextPtr = strtok(NULL, delimiters);
  return atol(numTextPtr); 
}