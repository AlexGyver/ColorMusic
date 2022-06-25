/*
  Скетч к проекту "Светомузыка на Arduino"
  Страница проекта (схемы, описания): https://alexgyver.ru/colormusic/
  Исходники на GitHub: https://github.com/AlexGyver/ColorMusic
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2018
  https://AlexGyver.ru/

  Как откалибровать уровень шума и как пользоваться пультом
  расписано на странице проекта! https://alexgyver.ru/colormusic/
*/

/*
  Основано на версии AlexGyver Technologies 2.10
  Версия 1.1.1
*/
#define VERSION "1.1.1"
// ***************************** SETTINGS *****************************

// ----- IR remote settings
#define REMOTE_TYPE 0			// 0 - без пульта, 1 - пульт от WAVGAT, 2 - пульт от KEYES, 3 - кастомный пульт
// система может работать С ЛЮБЫМ ИК ПУЛЬТОМ (практически). Коды для своего пульта можно задать начиная со строки 160 в прошивке. Коды пультов определяются скетчем IRtest_2.0, читай инструкцию

// ----- parameters
#define KEEP_SETTINGS 1			// save all settings in EEPROM
#define KEEP_STATE 1			// save on/off state in EEPROM (on/off feature is available only via IR remote)
#define RESET_SETTINGS 0		// to reset settings in EEPROM (set to 1, upload firmware, set back to 0 and upload firmware once again)
#define SETTINGS_LOG 1			// print to Serial all settings in EEPROM at start up

// ----- LED strip
#define NUM_LEDS 120			// LEDs quantity (max 410)
#define COLOR_ORDER GRB			// LEDs color order in strip. If colors are wrong - change the order. Try RGB, RBG, GRB, GBR, BRG, BGR
#define CURRENT_LIMIT 2000		// current limit in mA, "FastLED" library automatically controls the brightness. 0 - turn the limit off
byte BRIGHTNESS = 200;			// BRIGHTNESS (0 - 255)
byte EMPTY_BRIGHT = 40;			// brightness of empty (not flashing) LEDs (0 - 255)

// ----- пины подключения
#define SOUND_R A2				// analog audio in, right channel
#define SOUND_L A1				// analog audio in, left channel
#define SOUND_R_FREQ A3			// analog audio in for modes with frequencies (modes 2, 3, 4, 7, 8)
#define BTN_PIN 3				// button (PIN --- КНОПКА --- GND)
#define BTN_IS_TOUCH 1			// 0 - usual tactile button, 1 - touch button, e. g. TTP223 - in this case 'A' jumper should be opened - button outputs logical "1" when touched

#if defined(__AVR_ATmega32U4__)	// pins for Arduino Pro Micro (смотри схему для Pro Micro на странице проекта!!!)
#define MLED_PIN 17				// пин светодиода режимов на ProMicro, т.к. обычный не выведен.
#define MLED_ON LOW
#define LED_PIN 9				// пин DI светодиодной ленты на ProMicro, т.к. обычный не выведен.
#else							// Пины для других плат Arduino (по умолчанию)
#define MLED_PIN 13				// пин светодиода режимов
#define MLED_ON HIGH
#define LED_PIN 12				// пин DI светодиодной ленты
#endif

#define POT_GND A0				// ground pin for potentiometer
#define IR_PIN 2				// pin for IR Receiver

// ----- mode 1 - rainbow
float RAINBOW_STEP = 5.00;		// rainbow color change step - width of rainbow

// ----- animation
#define MODE 0					// initial mode
#define MAIN_LOOP 5				// period of main draw cycle ("animation" rootine)

// ----- audio signal
#define MONO 1					// 1 - only right channel (SOUND_R), 0 - two channels
#define EXP 1.4					// signal gain (for more strident animation) (default 1.4)
#define POTENT 1				// 1 - use potentiometer, 0 - use intrenal reference of 1.1 V
#define EMPTY_COLOR HUE_PURPLE	// color of empty (not flashing) LEDs. Black if EMPTY_BRIGHT = 0

// ----- lower noise threshold
uint16_t LOW_PASS = 100;		// lower noise threshold in VU modes (modes 0 and 1), manual
uint16_t SPEKTR_LOW_PASS = 40;	// lower noise threshold in frequencies modes (modes 2, 3, 4, 7, 8), manual
#define AUTO_LOW_PASS 0			// no EEPROM, do automatic setting of lower noise threshold every time at start up (default 0), so the colormusic should be turned on without music
#define EEPROM_LOW_PASS 1		// allow use EEPROM to save and load lower noise threshold (default 1)
#define LOW_PASS_ADD 13			// additional value to the lower threshold, for reliability (VU modes)
#define LOW_PASS_FREQ_ADD 3		// additional value to the lower threshold, for reliability (frequencies modes)

// ----- VU modes (#0 and 1)
float SMOOTH = 0.2;				// VU animation smoothness coefficient (default 0.5)
#define MAX_COEF 1.8			// loudness coefficient (max loudness = average loudness * MAX_COEF) (default 1.8)

// ----- frequencies modes (#2, 3, 4, 7, 8)
float SMOOTH_FREQ = 0.3;		// frequencies animation smoothness coefficient (default 0.8)
float MAX_COEF_FREQ = 1.2;		// threshold coefficient for colormusic to generate the flash (default 1.5)
#define SMOOTH_STEP 20			// step of brightness decreasing (more value - faster attenuation of light), i.e. fade rate
#define LOW_COLOR HUE_RED		// color of low frequencies
#define MID_COLOR HUE_YELLOW	// color of middle frequencies
#define HIGH_COLOR HUE_AQUA		// color of middle frequencies

// ----- stroboscope mode (#5)
uint16_t STROBE_PERIOD = 40;	// period of stroboscope flashes, ms //Trance: 140-145 BPM, dance: 120 BPM
uint8_t STROBE_BPM = 150;		// beats per minute. Further, STROBE_PERIOD is calculated using this value
#define STROBE_DUTY 20			// flash duty ratio (1 - 99) - ratio of "LED on" time to "LED off" time
#define STROBE_COLOR HUE_RED	// stroboscope color
#define STROBE_SAT 0			// saturation. If 0 - the color will be WHITE for any STROBE_COLOR (0 - 255)
byte STROBE_SMOOTH = 200;		// flash rise/fade rate (0 - 255). Should be enough for high frequenies

// ----- backlight mode (#6 - is divided into 3 sub modes)
byte LIGHT_COLOR = 0;			// mode #6-1: backlight color
byte LIGHT_SAT = 255;			// mode #6-1: backlight color saturation
byte COLOR_SPEED = 100;			// mode #6-2: effect speed
int RAINBOW_PERIOD = 1;			// mode #6-3: rainbow speed
float RAINBOW_STEP_2 = 0.5;		// mode #6-3: rainbow color change step - width of rainbow

// ----- traveling frequencies mode (#7)
byte RUNNING_SPEED = 11;

// ----- spectrum analyzer mode (#8)
byte HUE_START = 0;
byte HUE_STEP = 5;
#define LIGHT_SMOOTH 2

/*
  Colors in HSV
  HUE_RED
  HUE_ORANGE
  HUE_YELLOW
  HUE_GREEN
  HUE_AQUA
  HUE_BLUE
  HUE_PURPLE
  HUE_PINK
*/

// ----- codes of WAVGAT IR remote buttons -----
#if REMOTE_TYPE == 1
#define BUTT_UP     0xF39EEBAD
#define BUTT_DOWN   0xC089F6AD
#define BUTT_LEFT   0xE25410AD
#define BUTT_RIGHT  0x14CE54AD
#define BUTT_OK     0x297C76AD
#define BUTT_1      0x4E5BA3AD
#define BUTT_2      0xE51CA6AD
#define BUTT_3      0xE207E1AD
#define BUTT_4      0x517068AD
#define BUTT_5      0x1B92DDAD
#define BUTT_6      0xAC2A56AD
#define BUTT_7      0x5484B6AD
#define BUTT_8      0xD22353AD
#define BUTT_9      0xDF3F4BAD
#define BUTT_0      0xF08A26AD
#define BUTT_STAR   0x68E456AD
#define BUTT_HASH   0x151CD6AD
#endif

// ----- codes of KEYES IR remote buttons  -----
#if REMOTE_TYPE == 2
#define BUTT_UP     0xE51CA6AD
#define BUTT_DOWN   0xD22353AD
#define BUTT_LEFT   0x517068AD
#define BUTT_RIGHT  0xAC2A56AD
#define BUTT_OK     0x1B92DDAD
#define BUTT_1      0x68E456AD
#define BUTT_2      0xF08A26AD
#define BUTT_3      0x151CD6AD
#define BUTT_4      0x18319BAD
#define BUTT_5      0xF39EEBAD
#define BUTT_6      0x4AABDFAD
#define BUTT_7      0xE25410AD
#define BUTT_8      0x297C76AD
#define BUTT_9      0x14CE54AD
#define BUTT_0      0xC089F6AD
#define BUTT_STAR   0xAF3F1BAD
#define BUTT_HASH   0x38379AD
#endif

// ----- codes of yours IR remote buttons -----
#if REMOTE_TYPE == 3
#define BUTT_UP     0xE51CA6AD
#define BUTT_DOWN   0xD22353AD
#define BUTT_LEFT   0x517068AD
#define BUTT_RIGHT  0xAC2A56AD
#define BUTT_OK     0x1B92DDAD
#define BUTT_1      0x68E456AD
#define BUTT_2      0xF08A26AD
#define BUTT_3      0x151CD6AD
#define BUTT_4      0x18319BAD
#define BUTT_5      0xF39EEBAD
#define BUTT_6      0x4AABDFAD
#define BUTT_7      0xE25410AD
#define BUTT_8      0x297C76AD
#define BUTT_9      0x14CE54AD
#define BUTT_0      0xC089F6AD
#define BUTT_STAR   0xAF3F1BAD  // *
#define BUTT_HASH   0x38379AD   // #
#endif


// ------------------------------ FOR DEVELOPERS --------------------------------
#define MODE_AMOUNT 9		// modes quantity

#define STRIP NUM_LEDS / 5
float freq_to_strip = NUM_LEDS / 40; // /2 because of symmetry, and /20 because of 20 frequencies

#define FHT_N 64			// spectrum width х2
#define LOG_OUT 1
#include <FHT.h>			// Hartley transform

#include <EEPROMex.h>

#define FASTLED_ALLOW_INTERRUPTS 1
#include "FastLED.h"
CRGB leds[NUM_LEDS];

#include "GyverButton.h"
#if BTN_IS_TOUCH == 0	// usual tact button //D3 - butt1 - GND
  GButton butt1(BTN_PIN);
#else // e.g. TTP223 - 'A' jumper is opened - outputs logical "1" when pressed
  GButton butt1(BTN_PIN, LOW_PULL, NORM_OPEN);
#endif

#include "IRLremote.h"
CHashIR IRLremote;
uint32_t IRdata;

// градиент-палитра от зелёного к красному
DEFINE_GRADIENT_PALETTE(soundlevel_gp) {
  0,    0,    255,  0,  // green
  100,  255,  255,  0,  // yellow
  150,  255,  100,  0,  // orange
  200,  255,  50,   0,  // red
  255,  255,  0,    0   // red
};
CRGBPalette32 myPal = soundlevel_gp;

int Rlenght, Llenght;
float RsoundLevel, RsoundLevel_f;
float LsoundLevel, LsoundLevel_f;

float averageLevel = 50;
int maxLevel = 100;
int MAX_CH = NUM_LEDS / 2;
int hue;
unsigned long main_timer, hue_timer, strobe_timer, running_timer, color_timer, rainbow_timer, eeprom_timer;
float averK = 0.006;
byte count;
float index = (float)255 / MAX_CH;   // коэффициент перевода для палитры
boolean lowFlag;
byte low_pass;
int RcurrentLevel, LcurrentLevel;
int colorMusic[3];
float colorMusic_f[3], colorMusic_aver[3];
boolean colorMusicFlash[3], strobeUp_flag, strobeDwn_flag;
byte this_mode = MODE;
int thisBright[3], strobe_bright = 0;
//unsigned int light_time = STROBE_PERIOD * STROBE_DUTY / 100;
volatile boolean ir_flag;
boolean settings_mode, ONstate = true;
int8_t freq_strobe_mode = 0, light_mode = 2; //rainbow
int freq_max;
float freq_max_f, rainbow_steps;
int freq_f[32];
int this_color;
boolean running_flag[3], eeprom_flag;

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
// ------------------------------ ДЛЯ РАЗРАБОТЧИКОВ --------------------------------

void setup() {
  Serial.begin(115200);
  if (RESET_SETTINGS) EEPROM.write(1, 0);        // сброс флага настроек
  // в 1 ячейке хранится число 100. Если нет - значит это первый запуск системы
  if (KEEP_SETTINGS) {
    eeprom_timer = millis();
    eeprom_flag = false;
    if (EEPROM.read(1) != 100) {
      //Serial.println(F("First start"));
      EEPROM.write(1, 100);
      updateEEPROM();
    } else {
      readEEPROM();
    }
  }
  FastLED.addLeds<WS2811, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(BRIGHTNESS);

#if defined(__AVR_ATmega32U4__)   //Выключение светодиодов на Pro Micro
  TXLED1;                           //на ProMicro выключим и TXLED
  delay (1000);                     //При питании по usb от компьютера нужна задержка перед выключением RXLED. Если питать от БП, то можно убрать эту строку.
#endif
  pinMode(MLED_PIN, OUTPUT);        //Режим пина для светодиода режима на выход
  digitalWrite(MLED_PIN, !MLED_ON); //Выключение светодиода режима

  pinMode(POT_GND, OUTPUT);
  digitalWrite(POT_GND, LOW);
  butt1.setTimeout(800);
  butt1.setStepTimeout(100);

#if REMOTE_TYPE != 0
  IRLremote.begin(IR_PIN);
#endif

  // для увеличения точности уменьшаем опорное напряжение,
  // выставив EXTERNAL и подключив Aref к выходу 3.3V на плате через делитель
  // GND ---[10-20 кОм] --- REF --- [10 кОм] --- 3V3
  // в данной схеме GND берётся из А0 для удобства подключения
  if (POTENT) analogReference(EXTERNAL);
  else
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    analogReference(INTERNAL1V1);
#else
    analogReference(INTERNAL);
#endif

  // жуткая магия, меняем частоту оцифровки до 18 кГц
  // команды на *** ассемблере, даже не спрашивайте, как это работает
  // поднимаем частоту опроса аналогового порта до 38.4 кГц, по теореме
  // Котельникова (Найквиста) частота дискретизации будет 19.2 кГц
  // http://yaab-arduino.blogspot.ru/2015/02/fast-sampling-from-analog-input.html
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  if (AUTO_LOW_PASS && !EEPROM_LOW_PASS) {         // если разрешена автонастройка нижнего порога шумов
    autoLowPass();
  }

Serial.print(F("VERSION = ")); Serial.println(VERSION);
#if (SETTINGS_LOG == 1)
  Serial.print(F("this_mode = ")); Serial.println(this_mode);
  Serial.print(F("freq_strobe_mode = ")); Serial.println(freq_strobe_mode);
  Serial.print(F("light_mode = ")); Serial.println(light_mode);
  Serial.print(F("RAINBOW_STEP = ")); Serial.println(RAINBOW_STEP);
  Serial.print(F("MAX_COEF_FREQ = ")); Serial.println(MAX_COEF_FREQ);
  Serial.print(F("STROBE_PERIOD = ")); Serial.println(STROBE_PERIOD);
  Serial.print(F("LIGHT_SAT = ")); Serial.println(LIGHT_SAT);
  Serial.print(F("RAINBOW_STEP_2 = ")); Serial.println(RAINBOW_STEP_2);
  Serial.print(F("HUE_START = ")); Serial.println(HUE_START);
  Serial.print(F("SMOOTH = ")); Serial.println(SMOOTH);
  Serial.print(F("SMOOTH_FREQ = ")); Serial.println(SMOOTH_FREQ);
  Serial.print(F("STROBE_SMOOTH = ")); Serial.println(STROBE_SMOOTH);
  Serial.print(F("LIGHT_COLOR = ")); Serial.println(LIGHT_COLOR);
  Serial.print(F("COLOR_SPEED = ")); Serial.println(COLOR_SPEED);
  Serial.print(F("RAINBOW_PERIOD = ")); Serial.println(RAINBOW_PERIOD);
  Serial.print(F("RUNNING_SPEED = ")); Serial.println(RUNNING_SPEED);
  Serial.print(F("HUE_STEP = ")); Serial.println(HUE_STEP);
  Serial.print(F("BRIGHTNESS = ")); Serial.println(BRIGHTNESS);
  Serial.print(F("EMPTY_BRIGHT = ")); Serial.println(EMPTY_BRIGHT);
  Serial.print(F("ONstate = ")); Serial.println(ONstate);
#endif
}

unsigned int light_time = STROBE_PERIOD * STROBE_DUTY / 100; //should be defined only after STROBE_PERIOD has been initialised

void loop() {
  buttonTick();     // опрос и обработка кнопки
#if REMOTE_TYPE != 0
  remoteTick();     // опрос ИК пульта
#endif

  mainLoop();       // главный цикл обработки и отрисовки

#if KEEP_SETTINGS
  eepromTick();     // проверка не пора ли сохранить настройки
#endif
}

void mainLoop() {
  // главный цикл отрисовки
  if (ONstate) {
    if (millis() - main_timer > MAIN_LOOP) {
      // сбрасываем значения
      RsoundLevel = 0;
      LsoundLevel = 0;

      // перваые два режима - громкость (VU meter)
      if (this_mode == 0 || this_mode == 1) {
        for (byte i = 0; i < 100; i ++) {                                 // делаем 100 измерений
          RcurrentLevel = analogRead(SOUND_R);                            // с правого
          if (!MONO) LcurrentLevel = analogRead(SOUND_L);                 // и левого каналов

          if (RsoundLevel < RcurrentLevel) RsoundLevel = RcurrentLevel;   // ищем максимальное
          if (!MONO) if (LsoundLevel < LcurrentLevel) LsoundLevel = LcurrentLevel;   // ищем максимальное
        }

        // фильтруем по нижнему порогу шумов
        RsoundLevel = map(RsoundLevel, LOW_PASS, 1023, 0, 500);
        if (!MONO)LsoundLevel = map(LsoundLevel, LOW_PASS, 1023, 0, 500);

        // ограничиваем диапазон
        RsoundLevel = constrain(RsoundLevel, 0, 500);
        if (!MONO)LsoundLevel = constrain(LsoundLevel, 0, 500);

        // возводим в степень (для большей чёткости работы)
        RsoundLevel = pow(RsoundLevel, EXP);
        if (!MONO)LsoundLevel = pow(LsoundLevel, EXP);

        // фильтр
        RsoundLevel_f = RsoundLevel * SMOOTH + RsoundLevel_f * (1 - SMOOTH);
        if (!MONO)LsoundLevel_f = LsoundLevel * SMOOTH + LsoundLevel_f * (1 - SMOOTH);

        if (MONO) LsoundLevel_f = RsoundLevel_f;  // если моно, то левый = правому

        // заливаем "подложку", если яркость достаточная
        if (EMPTY_BRIGHT > 5) {
          for (int i = 0; i < NUM_LEDS; i++)
            leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
        }

        // если значение выше порога - начинаем самое интересное
        if (RsoundLevel_f > 15 && LsoundLevel_f > 15) {

          // расчёт общей средней громкости с обоих каналов, фильтрация.
          // Фильтр очень медленный, сделано специально для автогромкости
          averageLevel = (float)(RsoundLevel_f + LsoundLevel_f) / 2 * averK + averageLevel * (1 - averK);

          // принимаем максимальную громкость шкалы как среднюю, умноженную на некоторый коэффициент MAX_COEF
          maxLevel = (float)averageLevel * MAX_COEF;

          // преобразуем сигнал в длину ленты (где MAX_CH это половина количества светодиодов)
          Rlenght = map(RsoundLevel_f, 0, maxLevel, 0, MAX_CH);
          Llenght = map(LsoundLevel_f, 0, maxLevel, 0, MAX_CH);

          // ограничиваем до макс. числа светодиодов
          Rlenght = constrain(Rlenght, 0, MAX_CH);
          Llenght = constrain(Llenght, 0, MAX_CH);

          animation();       // отрисовать
        }
      }

      // 3-5 режим - цветомузыка
      if (this_mode == 2 || this_mode == 3 || this_mode == 4 || this_mode == 7 || this_mode == 8) {
        analyzeAudio();
        colorMusic[0] = 0;
        colorMusic[1] = 0;
        colorMusic[2] = 0;
        for (int i = 0 ; i < 32 ; i++) {
          if (fht_log_out[i] < SPEKTR_LOW_PASS) fht_log_out[i] = 0;
        }
        // низкие частоты, выборка со 2 по 5 тон (0 и 1 зашумленные!)
        for (byte i = 2; i < 6; i++) {
          if (fht_log_out[i] > colorMusic[0]) colorMusic[0] = fht_log_out[i];
        }
        // средние частоты, выборка с 6 по 10 тон
        for (byte i = 6; i < 11; i++) {
          if (fht_log_out[i] > colorMusic[1]) colorMusic[1] = fht_log_out[i];
        }
        // высокие частоты, выборка с 11 по 31 тон
        for (byte i = 11; i < 32; i++) {
          if (fht_log_out[i] > colorMusic[2]) colorMusic[2] = fht_log_out[i];
        }
        freq_max = 0;
        for (byte i = 0; i < 30; i++) {
          if (fht_log_out[i + 2] > freq_max) freq_max = fht_log_out[i + 2];
          if (freq_max < 5) freq_max = 5;

          if (freq_f[i] < fht_log_out[i + 2]) freq_f[i] = fht_log_out[i + 2];
          if (freq_f[i] > 0) freq_f[i] -= LIGHT_SMOOTH;
          else freq_f[i] = 0;
        }
        freq_max_f = freq_max * averK + freq_max_f * (1 - averK);
        for (byte i = 0; i < 3; i++) {
          colorMusic_aver[i] = colorMusic[i] * averK + colorMusic_aver[i] * (1 - averK);  // общая фильтрация
          colorMusic_f[i] = colorMusic[i] * SMOOTH_FREQ + colorMusic_f[i] * (1 - SMOOTH_FREQ);      // локальная
          if (colorMusic_f[i] > ((float)colorMusic_aver[i] * MAX_COEF_FREQ)) {
            thisBright[i] = 255;
            colorMusicFlash[i] = true;
            running_flag[i] = true;
          } else colorMusicFlash[i] = false;
          if (thisBright[i] >= 0) thisBright[i] -= SMOOTH_STEP;
          if (thisBright[i] < EMPTY_BRIGHT) {
            thisBright[i] = EMPTY_BRIGHT;
            running_flag[i] = false;
          }
        }
        animation();
      }
      if (this_mode == 5) {				//STROBE
        if ((long)millis() - strobe_timer > STROBE_PERIOD) {
          strobe_timer = millis();
          strobeUp_flag = true;
          strobeDwn_flag = false;
        }
        if ((long)millis() - strobe_timer > light_time) {
          strobeDwn_flag = true;
        }
        if (strobeUp_flag) {                    // если настало время пыхнуть
          if (strobe_bright < 255)              // если яркость не максимальная
            strobe_bright += STROBE_SMOOTH;     // увелчить
          if (strobe_bright > 255) {            // если пробили макс. яркость
            strobe_bright = 255;                // оставить максимум
            strobeUp_flag = false;              // флаг опустить
          }
        }

        if (strobeDwn_flag) {                   // гаснем
          if (strobe_bright > 0)                // если яркость не минимальная
            strobe_bright -= STROBE_SMOOTH;     // уменьшить
          if (strobe_bright < 0) {              // если пробили мин. яркость
            strobeDwn_flag = false;
            strobe_bright = 0;                  // оставить 0
          }
        }
        animation();
      }
      if (this_mode == 6) animation();	//light independent from music

#if REMOTE_TYPE != 0
      if (!IRLremote.receiving())		// если на ИК приёмник не приходит сигнал (без этого НЕ РАБОТАЕТ!)
        FastLED.show();					// отправить значения на ленту
#else
      FastLED.show();					// отправить значения на ленту
#endif

      if (this_mode != 7)				// 7 режиму не нужна очистка!!!
        FastLED.clear();				// очистить массив пикселей
      
	  main_timer = millis();			// сбросить таймер
    }
  }
}

void animation() {
  // согласно режиму
  switch (this_mode) {
    case 0:		//VU meter (столбик громкости): от зелёного к красному
      count = 0;
      for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
        leds[i] = ColorFromPalette(myPal, (count * index));   // заливка по палитре " от зелёного к красному"
        count++;
      }
      count = 0;
      for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) {
        leds[i] = ColorFromPalette(myPal, (count * index));   // заливка по палитре " от зелёного к красному"
        count++;
      }
      if (EMPTY_BRIGHT > 0) {
        CHSV this_dark = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
        for (int i = ((MAX_CH - 1) - Rlenght); i > 0; i--)
          leds[i] = this_dark;
        for (int i = MAX_CH + Llenght; i < NUM_LEDS; i++)
          leds[i] = this_dark;
      }
      break;
    case 1:		//VU meter (столбик громкости): плавно бегущая радуга
      if (millis() - rainbow_timer > 30) {
        rainbow_timer = millis();
        hue = floor((float)hue + RAINBOW_STEP);
      }
      count = 0;
      for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue);  // заливка по палитре радуга
        count++;
      }
      count = 0;
      for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue); // заливка по палитре радуга
        count++;
      }
      if (EMPTY_BRIGHT > 0) {
        CHSV this_dark = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
        for (int i = ((MAX_CH - 1) - Rlenght); i > 0; i--)
          leds[i] = this_dark;
        for (int i = MAX_CH + Llenght; i < NUM_LEDS; i++)
          leds[i] = this_dark;
      }
      break;
    case 2:		//Светомузыка по частотам: 5 полос
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i < STRIP)          leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
        else if (i < STRIP * 2) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < STRIP * 3) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
        else if (i < STRIP * 4) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < STRIP * 5) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
      }
      break;
    case 3:		//Светомузыка по частотам: 3 полосы
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i < NUM_LEDS / 3)          leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
        else if (i < NUM_LEDS * 2 / 3) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < NUM_LEDS)         leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
      }
      break;
    case 4:		//Светомузыка по частотам: 1 полоса, цвета накладываются друг на друга: бас, сверху, если есть, средние, затем высокие при наличии
      switch (freq_strobe_mode) {
        case 0:
          if (colorMusicFlash[2]) HIGHS();
          else if (colorMusicFlash[1]) MIDS();
          else if (colorMusicFlash[0]) LOWS();
          else SILENCE();
          break;
        case 1:
          if (colorMusicFlash[2]) HIGHS();
          else SILENCE();
          break;
        case 2:
          if (colorMusicFlash[1]) MIDS();
          else SILENCE();
          break;
        case 3:
          if (colorMusicFlash[0]) LOWS();
          else SILENCE();
          break;
      }
      break;
    case 5:		//strobe
      if (strobe_bright > 0)
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(STROBE_COLOR, STROBE_SAT, strobe_bright);
      else
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
      break;
    case 6: 	//light independent from music
      switch (light_mode) {	//constant light
        case 0: for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(LIGHT_COLOR, LIGHT_SAT, 255);
          break;
        case 1:		//smooth color changing
          if (millis() - color_timer > COLOR_SPEED) {
            color_timer = millis();
            if (++this_color > 255) this_color = 0;
          }
          for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(this_color, LIGHT_SAT, 255);
          break;
        case 2:		//rainbow
          if (millis() - rainbow_timer > 30) {
            rainbow_timer = millis();
            this_color += RAINBOW_PERIOD;
            if (this_color > 255) this_color = 0;
            if (this_color < 0) this_color = 255;
          }
          rainbow_steps = this_color;
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CHSV((int)floor(rainbow_steps), 255, 255);
            rainbow_steps += RAINBOW_STEP_2;
            if (rainbow_steps > 255) rainbow_steps = 0;
            if (rainbow_steps < 0) rainbow_steps = 255;
          }
          break;
      }
      break;
    case 7:
      switch (freq_strobe_mode) {
        case 0:
          if (running_flag[2]) leds[NUM_LEDS / 2] = CHSV(HIGH_COLOR, 255, thisBright[2]);
          else if (running_flag[1]) leds[NUM_LEDS / 2] = CHSV(MID_COLOR, 255, thisBright[1]);
          else if (running_flag[0]) leds[NUM_LEDS / 2] = CHSV(LOW_COLOR, 255, thisBright[0]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
        case 1:
          if (running_flag[2]) leds[NUM_LEDS / 2] = CHSV(HIGH_COLOR, 255, thisBright[2]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
        case 2:
          if (running_flag[1]) leds[NUM_LEDS / 2] = CHSV(MID_COLOR, 255, thisBright[1]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
        case 3:
          if (running_flag[0]) leds[NUM_LEDS / 2] = CHSV(LOW_COLOR, 255, thisBright[0]);
          else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
          break;
      }
      leds[(NUM_LEDS / 2) - 1] = leds[NUM_LEDS / 2];
      if (millis() - running_timer > RUNNING_SPEED) {
        running_timer = millis();
        for (int i = 0; i < NUM_LEDS / 2 - 1; i++) {
          leds[i] = leds[i + 1];
          leds[NUM_LEDS - i - 1] = leds[i];
        }
      }
      break;
    case 8:
      byte HUEindex = HUE_START;
      for (int i = 0; i < NUM_LEDS / 2; i++) {
        byte this_bright = map(freq_f[(int)floor((NUM_LEDS / 2 - i) / freq_to_strip)], 0, freq_max_f, 0, 255);
        this_bright = constrain(this_bright, 0, 255);
        leds[i] = CHSV(HUEindex, 255, this_bright);
        leds[NUM_LEDS - i - 1] = leds[i];
        HUEindex += HUE_STEP;
        if (HUEindex > 255) HUEindex = 0;
      }
      break;
  }
}

void HIGHS() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
}
void MIDS() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
}
void LOWS() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
}
void SILENCE() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
}

// вспомогательная функция, изменяет величину value на шаг incr в пределах minimum.. maximum
int smartIncr(int value, int incr_step, int mininmum, int maximum) {
  int val_buf = value + incr_step;
  val_buf = constrain(val_buf, mininmum, maximum);
  return val_buf;
}

float smartIncrFloat(float value, float incr_step, float mininmum, float maximum) {
  float val_buf = value + incr_step;
  val_buf = constrain(val_buf, mininmum, maximum);
  return val_buf;
}

#if REMOTE_TYPE != 0
void remoteTick() {
  if (IRLremote.available())  {
    auto data = IRLremote.read();
    IRdata = data.command;
    ir_flag = true;
  }
  if (ir_flag) { // если данные пришли
    eeprom_timer = millis();
    eeprom_flag = true;
    switch (IRdata) {
      // режимы
      case BUTT_1: this_mode = 0;
        break;
      case BUTT_2: this_mode = 1;
        break;
      case BUTT_3: this_mode = 2;
        break;
      case BUTT_4: this_mode = 3;
        break;
      case BUTT_5: this_mode = 4;
        break;
      case BUTT_6: this_mode = 5;
        break;
      case BUTT_7: this_mode = 6;
        break;
      case BUTT_8: this_mode = 7;
        break;
      case BUTT_9: this_mode = 8;
        break;
      case BUTT_0: fullLowPass();
        break;
      case BUTT_STAR: ONstate = !ONstate; FastLED.clear(); FastLED.show(); updateEEPROM();
        break;
      case BUTT_HASH:
        switch (this_mode) {
          case 4:
          case 7: if (++freq_strobe_mode > 3) freq_strobe_mode = 0;
            break;
          case 6: if (++light_mode > 2) light_mode = 0;
            break;
        }
        break;
      case BUTT_OK: digitalWrite(MLED_PIN, settings_mode ^ MLED_ON); settings_mode = !settings_mode;
        break;
      case BUTT_UP:
        if (settings_mode) {
          // ВВЕРХ общие настройки
          EMPTY_BRIGHT = smartIncr(EMPTY_BRIGHT, 5, 0, 255);
        } else {
          switch (this_mode) {
            case 0:
              break;
            case 1: RAINBOW_STEP = smartIncrFloat(RAINBOW_STEP, 0.5, 0.5, 20);
              break;
            case 2:
            case 3:
            case 4: MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, 0.1, 0, 5);
              break;
            case 5: STROBE_PERIOD = smartIncr(STROBE_PERIOD, 20, 1, 1000);
              break;
            case 6:
              switch (light_mode) {
                case 0: LIGHT_SAT = smartIncr(LIGHT_SAT, 20, 0, 255);
                  break;
                case 1: LIGHT_SAT = smartIncr(LIGHT_SAT, 20, 0, 255);
                  break;
                case 2: RAINBOW_STEP_2 = smartIncrFloat(RAINBOW_STEP_2, 0.5, 0.5, 10);
                  break;
              }
              break;
            case 7: MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, 0.1, 0.0, 10);
              break;
            case 8: HUE_START = smartIncr(HUE_START, 10, 0, 255);
              break;
          }
        }
        break;
      case BUTT_DOWN:
        if (settings_mode) {
          // ВНИЗ общие настройки
          EMPTY_BRIGHT = smartIncr(EMPTY_BRIGHT, -5, 0, 255);
        } else {
          switch (this_mode) {
            case 0:
              break;
            case 1: RAINBOW_STEP = smartIncrFloat(RAINBOW_STEP, -0.5, 0.5, 20);
              break;
            case 2:
            case 3:
            case 4: MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, -0.1, 0, 5);
              break;
            case 5: STROBE_PERIOD = smartIncr(STROBE_PERIOD, -20, 1, 1000);
              break;
            case 6:
              switch (light_mode) {
                case 0: LIGHT_SAT = smartIncr(LIGHT_SAT, -20, 0, 255);
                  break;
                case 1: LIGHT_SAT = smartIncr(LIGHT_SAT, -20, 0, 255);
                  break;
                case 2: RAINBOW_STEP_2 = smartIncrFloat(RAINBOW_STEP_2, -0.5, 0.5, 10);
                  break;
              }
              break;
            case 7: MAX_COEF_FREQ = smartIncrFloat(MAX_COEF_FREQ, -0.1, 0.0, 10);
              break;
            case 8: HUE_START = smartIncr(HUE_START, -10, 0, 255);
              break;
          }
        }
        break;
      case BUTT_LEFT:
        if (settings_mode) {
          // ВЛЕВО общие настройки
          BRIGHTNESS = smartIncr(BRIGHTNESS, -20, 0, 255);
          FastLED.setBrightness(BRIGHTNESS);
        } else {
          switch (this_mode) {
            case 0:
            case 1: SMOOTH = smartIncrFloat(SMOOTH, -0.05, 0.05, 1);
              break;
            case 2:
            case 3:
            case 4: SMOOTH_FREQ = smartIncrFloat(SMOOTH_FREQ, -0.05, 0.05, 1);
              break;
            case 5: STROBE_SMOOTH = smartIncr(STROBE_SMOOTH, -20, 0, 255);
              break;
            case 6:
              switch (light_mode) {
                case 0: LIGHT_COLOR = smartIncr(LIGHT_COLOR, -10, 0, 255);
                  break;
                case 1: COLOR_SPEED = smartIncr(COLOR_SPEED, -10, 0, 255);
                  break;
                case 2: RAINBOW_PERIOD = smartIncr(RAINBOW_PERIOD, -1, -20, 20);
                  break;
              }
              break;
            case 7: RUNNING_SPEED = smartIncr(RUNNING_SPEED, -10, 1, 255);
              break;
            case 8: HUE_STEP = smartIncr(HUE_STEP, -1, 1, 255);
              break;
          }
        }
        break;
      case BUTT_RIGHT:
        if (settings_mode) {
          // ВПРАВО общие настройки
          BRIGHTNESS = smartIncr(BRIGHTNESS, 20, 0, 255);
          FastLED.setBrightness(BRIGHTNESS);
        } else {
          switch (this_mode) {
            case 0:
            case 1: SMOOTH = smartIncrFloat(SMOOTH, 0.05, 0.05, 1);
              break;
            case 2:
            case 3:
            case 4: SMOOTH_FREQ = smartIncrFloat(SMOOTH_FREQ, 0.05, 0.05, 1);
              break;
            case 5: STROBE_SMOOTH = smartIncr(STROBE_SMOOTH, 20, 0, 255);
              break;
            case 6:
              switch (light_mode) {
                case 0: LIGHT_COLOR = smartIncr(LIGHT_COLOR, 10, 0, 255);
                  break;
                case 1: COLOR_SPEED = smartIncr(COLOR_SPEED, 10, 0, 255);
                  break;
                case 2: RAINBOW_PERIOD = smartIncr(RAINBOW_PERIOD, 1, -20, 20);
                  break;
              }
              break;
            case 7: RUNNING_SPEED = smartIncr(RUNNING_SPEED, 10, 1, 255);
              break;
            case 8: HUE_STEP = smartIncr(HUE_STEP, 1, 1, 255);
              break;
          }
        }
        break;
      default: eeprom_flag = false;   // если не распознали кнопку, не обновляем настройки!
        break;
    }
    ir_flag = false;
  }
}
#endif

void autoLowPass() {
  // для режима VU
  delay(10);                                // ждём инициализации АЦП
  int thisMax = 0;                          // максимум
  int thisLevel;
  for (byte i = 0; i < 200; i++) {
    thisLevel = analogRead(SOUND_R);        // делаем 200 измерений
    if (thisLevel > thisMax)                // ищем максимумы
      thisMax = thisLevel;                  // запоминаем
    delay(4);                               // ждём 4мс
  }
  LOW_PASS = thisMax + LOW_PASS_ADD;        // нижний порог как максимум тишины + некая величина

  // для режима спектра
  thisMax = 0;
  for (byte i = 0; i < 100; i++) {          // делаем 100 измерений
    analyzeAudio();                         // разбить в спектр
    for (byte j = 2; j < 32; j++) {         // первые 2 канала - хлам
      thisLevel = fht_log_out[j];
      if (thisLevel > thisMax)              // ищем максимумы
        thisMax = thisLevel;                // запоминаем
    }
    delay(4);                               // ждём 4мс
  }
  SPEKTR_LOW_PASS = thisMax + LOW_PASS_FREQ_ADD;  // нижний порог как максимум тишины
  if (EEPROM_LOW_PASS && !AUTO_LOW_PASS) {
    EEPROM.updateInt(70, LOW_PASS);
    EEPROM.updateInt(74, SPEKTR_LOW_PASS);
  }
}

void analyzeAudio() {
  for (int i = 0 ; i < FHT_N ; i++) {
    int sample = analogRead(SOUND_R_FREQ);
    fht_input[i] = sample; // put real data into bins
  }
  fht_window();  // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run();     // process the data in the fht
  fht_mag_log(); // take the output of the fht
}

void buttonTick() {
  butt1.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  if (butt1.isHolded()){				//кнопка удержана - изменить режим
    //Serial.println("HoldedPress");
    eeprom_timer = millis();
    eeprom_flag = true;
    if (++this_mode >= MODE_AMOUNT) this_mode = 0;
  }
  if (butt1.isSingle()){				//аналог нажатия на пульте кновки "вправо" - регулировка плавности или скорости анимации
    //Serial.println("SinglePress");
    eeprom_timer = millis();
    eeprom_flag = true;
    switch (this_mode) {
      case 0:
      case 1:
        if (SMOOTH+0.05 > 0.5) SMOOTH = 0.05;
		else SMOOTH += 0.05;
        Serial.print(F("SMOOTH = ")); Serial.println(SMOOTH);
        break;
      case 2:
      case 3:
      case 4:
        if (SMOOTH_FREQ+0.05 > 1) SMOOTH_FREQ = 0.05;
		else SMOOTH_FREQ += 0.05;
        Serial.print(F("SMOOTH_FREQ = ")); Serial.println(SMOOTH_FREQ);
        break;
      case 5:
        if (STROBE_SMOOTH+20 > 255) STROBE_SMOOTH = 0;
		else STROBE_SMOOTH += 20;
        Serial.print(F("STROBE_SMOOTH = ")); Serial.println(STROBE_SMOOTH);
        break;
      case 6:
        switch (light_mode) {
          case 0:
            if (LIGHT_COLOR+10 > 255) LIGHT_COLOR = 0;
			else LIGHT_COLOR += 10;
            Serial.print(F("LIGHT_COLOR = ")); Serial.println(LIGHT_COLOR);
            break;
          case 1:
            if (COLOR_SPEED+10 > 255) COLOR_SPEED = 0;
			else COLOR_SPEED += 10;
            Serial.print(F("COLOR_SPEED = ")); Serial.println(COLOR_SPEED);
            break;
          case 2:
            if (RAINBOW_PERIOD+2 > 14) RAINBOW_PERIOD = -14;
			else RAINBOW_PERIOD += 2;			// !!!!! БЫЛО 1 - ПРОВЕРИТЬ !!!!!
            Serial.print(F("RAINBOW_PERIOD = ")); Serial.println(RAINBOW_PERIOD);
            break;
        }
        break;
      case 7:
        if (RUNNING_SPEED*2 > 255) RUNNING_SPEED = 3;//1;
		else RUNNING_SPEED *=2;//+= 10;
        Serial.print(F("RUNNING_SPEED = ")); Serial.println(RUNNING_SPEED);
        break;
      case 8:
        if (HUE_STEP*2 > 255) HUE_STEP = 1;
		else HUE_STEP *= 2; //+20;			// !!!!! БЫЛО 1 - ПРОВЕРИТЬ !!!!!
        Serial.print(F("HUE_STEP = ")); Serial.println(HUE_STEP);
        break;
    }
    //break;
  }	//isSingle
  if (butt1.isDouble()){				//аналог нажатия на пульте кновки "вверх" - регулировка чувствительности, или насыщенности, или шага радуги
    //Serial.println("DoublePress");
    eeprom_timer = millis();
    eeprom_flag = true;
    switch (this_mode) {
      case 0:
        break;
      case 1:
        if (RAINBOW_STEP+0.5 > 20) RAINBOW_STEP = 0.5;
		else RAINBOW_STEP += 0.5;
        Serial.print(F("RAINBOW_STEP = ")); Serial.println(RAINBOW_STEP);
        break;
      case 2:
      case 3:
      case 4:
        if (MAX_COEF_FREQ+0.4 > 5) MAX_COEF_FREQ = 0;
		else MAX_COEF_FREQ += 0.2;
        Serial.print(F("MAX_COEF_FREQ = ")); Serial.println(MAX_COEF_FREQ);
        break;
      case 5: //1500BPM-25Hz-40ms 1200BPM-20Hz-50ms 900BPM-15Hz-67ms 600BPM-10Hz-100ms 300BPM-5Hz-200ms
        if (STROBE_BPM+30 > 150) STROBE_BPM = 30;
		else STROBE_BPM += 30;
        STROBE_PERIOD = 1./(STROBE_BPM/6)*1000;	// надо делить на 60, но частоту умножаю на 10, чтобы уменьшить операции делю на 6
        Serial.print(F("STROBE_BPM = ")); Serial.println(STROBE_BPM);
        Serial.print(F("STROBE_PERIOD = ")); Serial.println(STROBE_PERIOD);
        break;
      case 6:
        switch (light_mode) {
          case 0:
            //if (LIGHT_SAT+20 > 255) LIGHT_SAT = 0;
            //else LIGHT_SAT += 20;
            //break;
          case 1:
            if (LIGHT_SAT+20 > 255) LIGHT_SAT = 0;
			else LIGHT_SAT += 20;
            Serial.print(F("LIGHT_SAT = ")); Serial.println(LIGHT_SAT);
            break;
          case 2:
            if (RAINBOW_STEP_2+1 > 8) RAINBOW_STEP_2 = 0.5;
			else RAINBOW_STEP_2 += 1;
            Serial.print(F("RAINBOW_STEP_2 = ")); Serial.println(RAINBOW_STEP_2);
            break;
        }
        break;
      case 7:
        if (MAX_COEF_FREQ+0.4 > 5) MAX_COEF_FREQ = 0.0;
		else MAX_COEF_FREQ += 0.4;
        Serial.print(F("MAX_COEF_FREQ = ")); Serial.println(MAX_COEF_FREQ);
        break;
      case 8:
        if (HUE_START+20 > 255) HUE_START = 0;
		else HUE_START += 20;
        Serial.print(F("HUE_START = ")); Serial.println(HUE_START);
        break;
    }
  } //isDouble()
  if (butt1.isTriple()){
    //Serial.println("TripplePress");
    if (BRIGHTNESS+40 > 255) BRIGHTNESS = 40;
    else BRIGHTNESS += 40;
    FastLED.setBrightness(BRIGHTNESS);
    Serial.print(F("BRIGHTNESS = ")); Serial.println(BRIGHTNESS);
  }
  if (butt1.hasClicks() && butt1.getClicks() == 5) {	//пять нажатий для калибровки уровня шума. Обязательно в самом конце, т. к. 'getClicks()' method resets count of clicks
    //Serial.println("FifthPress");
    fullLowPass();
  }
}
void fullLowPass() {
  digitalWrite(MLED_PIN, MLED_ON);   // включить светодиод
  FastLED.setBrightness(0); // погасить ленту
  FastLED.clear();          // очистить массив пикселей
  FastLED.show();           // отправить значения на ленту
  delay(500);               // подождать чутка
  autoLowPass();            // измерить шумы
  delay(500);               // подождать
  FastLED.setBrightness(BRIGHTNESS);  // вернуть яркость
  digitalWrite(MLED_PIN, !MLED_ON);    // выключить светодиод
}
void updateEEPROM() {
  EEPROM.updateByte(2, this_mode);
  EEPROM.updateByte(3, freq_strobe_mode);
  EEPROM.updateByte(4, light_mode);
  EEPROM.updateFloat(5, RAINBOW_STEP);
  EEPROM.updateFloat(9, MAX_COEF_FREQ);
  EEPROM.updateInt(13, STROBE_BPM);
  EEPROM.updateByte(17, LIGHT_SAT);
  EEPROM.updateFloat(18, RAINBOW_STEP_2);
  EEPROM.updateByte(22, HUE_START);
  EEPROM.updateFloat(23, SMOOTH);
  EEPROM.updateFloat(27, SMOOTH_FREQ);
  EEPROM.updateByte(31, STROBE_SMOOTH);
  EEPROM.updateByte(32, LIGHT_COLOR);
  EEPROM.updateByte(33, COLOR_SPEED);
  EEPROM.updateInt(34, RAINBOW_PERIOD);
  EEPROM.updateByte(38, RUNNING_SPEED);
  EEPROM.updateByte(39, HUE_STEP);
  EEPROM.updateByte(40, BRIGHTNESS);
  EEPROM.updateByte(41, EMPTY_BRIGHT);
  if (KEEP_STATE) EEPROM.updateByte(42, ONstate);
}
void readEEPROM() {
  this_mode = EEPROM.readByte(2);
  freq_strobe_mode = EEPROM.readByte(3);
  light_mode = EEPROM.readByte(4);
  RAINBOW_STEP = EEPROM.readFloat(5);
  MAX_COEF_FREQ = EEPROM.readFloat(9);
  STROBE_BPM = EEPROM.readInt(13);
  STROBE_PERIOD = 1./(STROBE_BPM/6)*1000;
  LIGHT_SAT = EEPROM.readByte(17);
  RAINBOW_STEP_2 = EEPROM.readFloat(18);
  HUE_START = EEPROM.readByte(22);
  SMOOTH = EEPROM.readFloat(23);
  SMOOTH_FREQ = EEPROM.readFloat(27);
  STROBE_SMOOTH = EEPROM.readByte(31);
  LIGHT_COLOR = EEPROM.readByte(32);
  COLOR_SPEED = EEPROM.readByte(33);
  RAINBOW_PERIOD = EEPROM.readInt(34);
  RUNNING_SPEED = EEPROM.readByte(38);
  HUE_STEP = EEPROM.readByte(39);
  BRIGHTNESS = EEPROM.readByte(40);
  EMPTY_BRIGHT = EEPROM.readByte(41);
  if (KEEP_STATE) ONstate = EEPROM.readByte(42);
  if (EEPROM_LOW_PASS) {                // восстановить значения шумов из памяти
    LOW_PASS = EEPROM.readInt(70);
    SPEKTR_LOW_PASS = EEPROM.readInt(74);
  }
}
void eepromTick() {
  if ((eeprom_flag) && (millis() - eeprom_timer > 10000)) {  // 10 секунд после последнего нажатия с пульта
    eeprom_flag = false;
    eeprom_timer = millis();
    updateEEPROM();
  }
}
