/*
  Copyright (c) 2014-2017 NicoHood
  See the readme for credit to other people.

  IRL NecAPI

  Receives IR NEC signals and decodes them with the API for better processing.

  The following pins are usable for PinInterrupt or PinChangeInterrupt*:
  Arduino Uno/Nano/Mini: All pins are usable
  Arduino Mega: 10, 11, 12, 13, 50, 51, 52, 53, A8 (62), A9 (63), A10 (64),
              A11 (65), A12 (66), A13 (67), A14 (68), A15 (69)
  Arduino Leonardo/Micro: 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI)
  HoodLoader2: All (broken out 1-7) pins are usable
  Attiny 24/44/84: All pins are usable
  Attiny 25/45/85: All pins are usable
  Attiny 13: All pins are usable
  Attiny 441/841: All pins are usable
  ATmega644P/ATmega1284P: All pins are usable

  PinChangeInterrupts* requires a special library which can be downloaded here:
  https://github.com/NicoHood/PinChangeInterrupt
*/

// Use HID Project for more keys and options
#include "HID-Project.h"

#define USBKEYBOARD BootKeyboard
#define USBMOUSE BootMouse

// include PinChangeInterrupt library* BEFORE IRLremote to acces more pins if needed
//#include "PinChangeInterrupt.h"

#include "IRLremote.h"

// Choose a valid PinInterrupt or PinChangeInterrupt* pin of your Arduino board
#define pinIR 2
using namespace IRL_Protek_Remote;

// Choose the IR protocol of your remote. See the other example for this.
void NecEvent(void);
CNecAPI<NecEvent, IRL_ADDRESS> IRLremote;

#define pinLed LED_BUILTIN

void setup()
{
  // Start serial debug output
  while (!Serial);
  Serial.begin(115200);
  Serial.println(F("Startup"));

  // Set LED to output
  pinMode(pinLed, OUTPUT);

  // Start reading the remote. PinInterrupt or PinChangeInterrupt* will automatically be selected
  if (!IRLremote.begin(pinIR))
    Serial.println(F("You did not choose a valid pin."));

  // Start HID devices
  USBKEYBOARD.begin();
  USBMOUSE.begin();
}

void loop()
{
  // Get the new data from the remote
  // Always call this to check for timeouts
  IRLremote.read();
}



void NecEvent(void)
{
  // Get the current button (re)press count
  auto pressCount = IRLremote.pressCount();

  // Tells how many times the button was pressed including the first time.
  // Add a debounce if the button is held down, only accept every xth key.
  // 1, 2, 4, 8, 16, 32, 64, 128 works best
  auto holdDebounce = IRLremote.holdCount(4);
  auto holdCount = IRLremote.holdCount(0);

  // Button holding/pressing multiple times timed out.
  // Use this to differentiate between 1,2,3,...,n button presses.
  // or release holding keys.
  auto pressTimeout = IRLremote.pressTimeout();
  auto releaseButton = IRLremote.releaseButton();

  // Get the actual command we are processing
  auto command = IRLremote.command();


  // Print data to the serial for debugging
  //  Serial.println(F("----------"));
  //  Serial.print(F("Key: "));
  //  Serial.println(command);
  //  Serial.print(F("Keypresses: "));
  //  Serial.println(pressCount);
  //  Serial.print(F("holdDebounce: "));
  //  Serial.println(holdDebounce);
  //  Serial.print(F("pressTimeout: "));
  //  Serial.println(pressTimeout);
  //  Serial.println();


  // Differenciate between 4 modes
  enum IRModes : uint8_t
  {
    PC_Mode,
    Mouse_Mode,
    Led_Mode,
    Kodi_Mode,
  };
  static IRModes IRMode = PC_Mode;

  // Press HID keys with a protek remote
  switch (command)
  {
    // Red mode
    case IRL_KEYCODE_RED:
      if (holdCount == 1) {
        IRMode = PC_Mode;
      }
      break;

    // Green mode
    case IRL_KEYCODE_GREEN:
      if (holdCount == 1) {
        IRMode = Mouse_Mode;
      }
      break;

    // Yellow mode
    case IRL_KEYCODE_YELLOW:
      if (holdCount == 1) {
        // Not implemented
        IRMode = Led_Mode;
      }
      break;

    // Blue mode
    case IRL_KEYCODE_BLUE:
      if (holdCount == 1) {
        // Kodi remote
        // http://kodi.wiki/view/Keyboard_controls
        // https://github.com/xbmc/xbmc/blob/master/system/keymaps/keyboard.xml
        IRMode = Kodi_Mode;
      }
      break;


    // Shutdown menu
    case IRL_KEYCODE_POWER:
      if (IRMode == Kodi_Mode) {
        // Hold the button some time (around 3 sec) to trigger shutdown button
        if (holdDebounce == 4) {
          USBKEYBOARD.press('s');
          USBKEYBOARD.releaseAll();
        }
      }
      else {
        // Hold the button some time (around 3 sec) to trigger shutdown button
        if (pressCount == 1 && holdDebounce == 4) {
          if (IRMode == PC_Mode) {
            USBKEYBOARD.press(KEY_POWER);
            USBKEYBOARD.releaseAll();
          }
          IRLremote.reset();
        }
        // For two short keypress just exit the current program
        else if (pressCount == 2 && holdDebounce == 4)
        {
          IRLremote.reset();
        }
      }
      break;

    // Global system mute
    case IRL_KEYCODE_MUTE:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press(KEY_F8);
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode) {
          USBKEYBOARD.press(KEY_VOLUME_MUTE);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Fullscreen/Window mode
    case IRL_KEYCODE_SCREEN:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('\\');
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode || IRMode == Led_Mode) {
          // Turn off screens
          // xset dpms force off
          USBKEYBOARD.write(CONSUMER_SLEEP);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Live TV channels window
    case IRL_KEYCODE_SATELLITE:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('h');
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Open videos
    case IRL_KEYCODE_TV_RADIO:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press(KEY_LEFT_CTRL);
          USBKEYBOARD.press('e');
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Open music
    case IRL_KEYCODE_TV_MUSIC:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press(KEY_LEFT_CTRL);
          USBKEYBOARD.press('m');
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // 1
    case IRL_KEYCODE_1:
      if (holdDebounce == 1) {
        USBKEYBOARD.press('1');
        USBKEYBOARD.releaseAll();
      }
      break;

    // Letters are used to qickly jump between movies in the library
    // A, B, C, 2
    case IRL_KEYCODE_2:
      if (pressTimeout == 1) {
        USBKEYBOARD.press('A');
      }
      else if (pressTimeout == 2) {
        USBKEYBOARD.press('B');
      }
      else if (pressTimeout == 3) {
        USBKEYBOARD.press('C');
      }
      else if (pressTimeout == 4) {
        USBKEYBOARD.press('2');
      }
      USBKEYBOARD.releaseAll();
      break;

    // D, E, F, 3
    case IRL_KEYCODE_3:
      if (pressTimeout == 1) {
        USBKEYBOARD.press('D');
      }
      else if (pressTimeout == 2) {
        USBKEYBOARD.press('E');
      }
      else if (pressTimeout == 3) {
        USBKEYBOARD.press('F');
      }
      else if (pressTimeout == 4) {
        USBKEYBOARD.press('3');
      }
      USBKEYBOARD.releaseAll();
      break;

    // G, H, I, 4
    case IRL_KEYCODE_4:
      if (pressTimeout == 1) {
        USBKEYBOARD.press('G');
      }
      else if (pressTimeout == 2) {
        USBKEYBOARD.press('H');
      }
      else if (pressTimeout == 3) {
        USBKEYBOARD.press('I');
      }
      else if (pressTimeout == 4) {
        USBKEYBOARD.press('4');
      }
      USBKEYBOARD.releaseAll();
      break;

    // J, K, L, 5
    case IRL_KEYCODE_5:
      if (pressTimeout == 1) {
        USBKEYBOARD.press('J');
      }
      else if (pressTimeout == 2) {
        USBKEYBOARD.press('K');
      }
      else if (pressTimeout == 3) {
        USBKEYBOARD.press('L');
      }
      else if (pressTimeout == 4) {
        USBKEYBOARD.press('5');
      }
      USBKEYBOARD.releaseAll();
      break;

    // M, N, O, 6
    case IRL_KEYCODE_6:
      if (pressTimeout == 1) {
        USBKEYBOARD.press('M');
      }
      else if (pressTimeout == 2) {
        USBKEYBOARD.press('N');
      }
      else if (pressTimeout == 3) {
        USBKEYBOARD.press('O');
      }
      else if (pressTimeout == 4) {
        USBKEYBOARD.press('6');
      }
      USBKEYBOARD.releaseAll();
      break;

    // P, Q, R, S, 7
    case IRL_KEYCODE_7:
      if (pressTimeout == 1) {
        USBKEYBOARD.press('P');
      }
      else if (pressTimeout == 2) {
        USBKEYBOARD.press('Q');
      }
      else if (pressTimeout == 3) {
        USBKEYBOARD.press('R');
      }
      else if (pressTimeout == 4) {
        USBKEYBOARD.press('S');
      }
      else if (pressTimeout == 5) {
        USBKEYBOARD.press('7');
      }
      USBKEYBOARD.releaseAll();
      break;

    // T, U, V, 8
    case IRL_KEYCODE_8:
      if (pressTimeout == 1) {
        USBKEYBOARD.press('T');
      }
      else if (pressTimeout == 2) {
        USBKEYBOARD.press('U');
      }
      else if (pressTimeout == 3) {
        USBKEYBOARD.press('V');
      }
      else if (pressTimeout == 4) {
        USBKEYBOARD.press('8');
      }
      USBKEYBOARD.releaseAll();
      break;

    // W, X, Y, Z, 9
    case IRL_KEYCODE_9:
      if (pressTimeout == 1) {
        USBKEYBOARD.press('W');
      }
      else if (pressTimeout == 2) {
        USBKEYBOARD.press('X');
      }
      else if (pressTimeout == 3) {
        USBKEYBOARD.press('Y');
      }
      else if (pressTimeout == 4) {
        USBKEYBOARD.press('Z');
      }
      else if (pressTimeout == 5) {
        USBKEYBOARD.press('9');
      }
      USBKEYBOARD.releaseAll();
      break;

    // Space, 0
    case IRL_KEYCODE_0:
      if (pressTimeout == 1) {
        USBKEYBOARD.press(' ');
      }
      else if (pressTimeout == 2) {
        USBKEYBOARD.press('0');
      }
      USBKEYBOARD.releaseAll();
      break;

    case IRL_KEYCODE_BACK:
      if (holdDebounce == 1) {
        // Back
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press(KEY_BACKSPACE);
          USBKEYBOARD.releaseAll();
        }
        // Backspace
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(KEY_BACKSPACE);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    case IRL_KEYCODE_FAVORITE:
      if (holdDebounce == 1) {
        // Mark as watched/unwatched
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('w');
          USBKEYBOARD.releaseAll();
        }
        // Delete
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(KEY_DELETE);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Volume up
    case IRL_KEYCODE_VOL_UP:
      if (holdDebounce) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('+'); // F10, =
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(KEY_VOLUME_UP);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Volume down
    case IRL_KEYCODE_VOL_DOWN:
      if (holdDebounce) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('-'); // F9
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(KEY_VOLUME_DOWN);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Live TV EPG/TV guide
    case IRL_KEYCODE_EPG:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('e');
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(KEY_TAB);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Info
    case IRL_KEYCODE_INFO:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('i');
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(KEY_F1);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Increase rating
    case IRL_KEYCODE_CHANNEL_UP:
      if (IRMode == Kodi_Mode || IRMode == PC_Mode) {
        // Decrease rating
        if (holdDebounce) {
          USBKEYBOARD.press(KEY_PAGE_UP);
          USBKEYBOARD.releaseAll();
        }
      }
      else if (IRMode == Mouse_Mode) {
        // Scroll
        if (holdCount) {
          USBMOUSE.move(0, 0, min(holdCount, 10));
        }
      }
      break;

    // Decrease rating
    case IRL_KEYCODE_CHANNEL_DOWN:
      if (IRMode == Kodi_Mode || IRMode == PC_Mode) {
        if (holdDebounce) {
          USBKEYBOARD.press(KEY_PAGE_DOWN);
          USBKEYBOARD.releaseAll();
        }
      }
      else if (IRMode == Mouse_Mode) {
        // Scroll
        if (holdCount) {
          USBMOUSE.move(0, 0, -1 * min(holdCount, 10));
        }
      }
      break;

    // Navigation
    case IRL_KEYCODE_RIGHT:
      if (IRMode == Kodi_Mode || IRMode == PC_Mode) {
        if (holdDebounce) {
          USBKEYBOARD.press(KEY_RIGHT_ARROW);
          USBKEYBOARD.releaseAll();
        }
      }
      else if (IRMode == Mouse_Mode) {
        if (holdCount) {
          USBMOUSE.move(min(holdCount * holdCount, 127), 0);
        }
      }
      break;
    case IRL_KEYCODE_LEFT:
      if (IRMode == Kodi_Mode || IRMode == PC_Mode) {
        if (holdDebounce) {
          USBKEYBOARD.press(KEY_LEFT_ARROW);
          USBKEYBOARD.releaseAll();
        }
      }
      else if (IRMode == Mouse_Mode) {
        if (holdCount) {
          USBMOUSE.move(-1 * min(holdCount * holdCount, 127), 0);
        }
      }
      break;
    case IRL_KEYCODE_UP:
      if (IRMode == Kodi_Mode || IRMode == PC_Mode) {
        if (holdDebounce) {
          USBKEYBOARD.press(KEY_UP_ARROW);
          USBKEYBOARD.releaseAll();
        }
      }
      else if (IRMode == Mouse_Mode) {
        if (holdCount) {
          USBMOUSE.move(0, -1 * min(holdCount * holdCount, 127));
        }
      }
      break;
    case IRL_KEYCODE_DOWN:
      if (IRMode == Kodi_Mode || IRMode == PC_Mode) {
        if (holdDebounce) {
          USBKEYBOARD.press(KEY_DOWN_ARROW);
          USBKEYBOARD.releaseAll();
        }
      }
      else if (IRMode == Mouse_Mode) {
        if (holdCount) {
          USBMOUSE.move(0, min(holdCount * holdCount, 127));
        }
      }
      break;

    // Enter menu, play/pause, general okay button
    case IRL_KEYCODE_OK:
      if (IRMode == Mouse_Mode) {
        if (holdDebounce == 1) {
          USBMOUSE.press();
        }
        else if (releaseButton) {
          USBMOUSE.releaseAll();
        }
      }
      else if (IRMode == Kodi_Mode || IRMode == PC_Mode) {
        if (holdDebounce == 1) {
          USBKEYBOARD.press(KEY_RETURN);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Exit full screeen
    case IRL_KEYCODE_EXIT:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode || IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(KEY_ESC);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Context menu/Playlist
    case IRL_KEYCODE_MENU:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('c');
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode) {
          USBKEYBOARD.press(KEY_MENU);
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == Mouse_Mode) {
          USBMOUSE.press(MOUSE_RIGHT);
        }
      }
      else if (releaseButton) {
        USBMOUSE.releaseAll();
      }
      break;

    // Show debug information
    case IRL_KEYCODE_I_II:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press(KEY_LEFT_CTRL);
          USBKEYBOARD.press('D');
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode) {
          USBKEYBOARD.press(KEY_LEFT_GUI);
          USBKEYBOARD.press('p');
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Teletext/Visualization settings
    case IRL_KEYCODE_TELETEXT:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('v');
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode) {
          // Linux is very picky here, 'e' needs to be released first.
          USBKEYBOARD.press(KEY_LEFT_GUI);
          USBKEYBOARD.write('e');
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Next subtitle
    case IRL_KEYCODE_SUBTITLE:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('l');
          USBKEYBOARD.releaseAll();
        }
        // Open Switchboard
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(KEY_LEFT_GUI);
          USBKEYBOARD.press(KEY_SPACE);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Fullscreen play
    case IRL_KEYCODE_ADD:
      if (IRMode == Kodi_Mode) {
        if (holdDebounce == 1) {
          USBKEYBOARD.press(KEY_TAB);
          USBKEYBOARD.releaseAll();
        }
      }
      // Switch application
      else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
        // Hold down to keep the overview, press again to switch application
        if (holdDebounce == 1) {
          USBKEYBOARD.press(KEY_LEFT_ALT);
          USBKEYBOARD.write(KEY_TAB);
        }
        if (pressTimeout) {
          USBKEYBOARD.releaseAll();
        }
      }

      break;

    // Play/Pause
    case IRL_KEYCODE_PLAY:
    case IRL_KEYCODE_PAUSE:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press(KEY_SPACE);
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(MEDIA_PLAY_PAUSE);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Skip forward
    case IRL_KEYCODE_NEXT:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('.');
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(MEDIA_NEXT);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Skip backward
    case IRL_KEYCODE_PREV:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press(',');
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(MEDIA_PREVIOUS);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Stop
    case IRL_KEYCODE_STOP:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press('x');
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(MEDIA_STOP);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Change keyboard layout
    case IRL_KEYCODE_USB:
      if (IRMode == PC_Mode || IRMode == Mouse_Mode || IRMode == Kodi_Mode) {
        if (holdDebounce == 1) {
          USBKEYBOARD.press(KEY_LEFT_SHIFT);
          USBKEYBOARD.press(KEY_LEFT_ALT);
          USBKEYBOARD.press(KEY_SPACE);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // Screenshot
    case IRL_KEYCODE_REC:
      if (holdDebounce == 1) {
        if (IRMode == Kodi_Mode) {
          USBKEYBOARD.press(KEY_LEFT_CTRL);
          USBKEYBOARD.press('s');
          USBKEYBOARD.releaseAll();
        }
        else if (IRMode == PC_Mode || IRMode == Mouse_Mode) {
          USBKEYBOARD.press(KEY_PRINT);
          USBKEYBOARD.releaseAll();
        }
      }
      break;

    // TODO
    case IRL_KEYCODE_LIVE:

      break;
  }
}

