## ColorMusic

It is Alex Gyver project: [ColorMusic](https://github.com/AlexGyver/ColorMusic).
Almost everything is made by him. I just fixed some bugs and made little improvements.
Find differences in [history.md](https://github.com/x3mEr/ColorMusic/history.md)

*A lot of useful information (different schemes, components, explanation, videos) could be found [here](https://alexgyver.ru/colormusic/).*


## About

ColorMusic (individually addressable LED strip WS2812b) is driven by Arduino Nano v3.0 with ATmega328P.

**Features:**
- animation smoothness can be set up;
- auto or manual setting of lower noise threshold;
- auto volume adjustment;
- support of stereo and mono input;
- support of several popular IR remotes + one customizable;
- all settings are stored in EEPROM...
- 9 different effects:
	0. VU meter: classical volume bar;
	1. VU meter: smoothly running rainbow;
	2. Colormusic by frequencies: 5 symmetrical bars (high-middle-low-middle-high);
	3. Colormusic by frequencies: 3 bars (high-middle-low);
	4. Colormusic by frequencies: 1 bar (the colors of the frequencies overlap each other);
	5. Stroboscope with frequencies: 25 Hz (1500 BPM - trance music x10), 20 Hz (1200 BPM - dance music x10), 15 Hz (900 BPM), 10 Hz (600 BPM) and 5 Hz (300 BPM);
	6. Backlight. This mode has 3 sub modes (could be programmed in advance or switched with IR remote):
		1. constant color;
		2. changing color;
		3. running rainbow;
	7. Running frequencies;
	8. Spectrum Analyzer.


## Controls

Alex Gyver described IR remote control in detail.
Here I'll describe control with just one button.
Because we don't have `+` and `-` buttons, our single button changes setting cyclically: once the setting reaches upper limit it drops to the lower limit.

**Almighty button:**

	- Hold button: switch mode cyclically;
	- Single press: imitates IR remote button "up" (see table below);
	- Double press: imitates IR remote button "right" (see table below);
	- Triple press: increase brightness;
	- Press five times to calibrate the noise thresholds.

| Mode				    	| Single press 					| Double press	|
| --------------------------|-------------------------------|---------------|
| 0) classical VU meter		| animation smoothness			| -				|
| 1) rainbow VU meter		| animation smoothness			| rainbow running speed	|
| 2) 5 symmetrical bars		| animation smoothness			| sensitivity	|
| 3) 3 bars					| animation smoothness			| sensitivity	|
| 4) 1 bar					| animation smoothness			| sensitivity	|
| 5) stroboscope			| light flash smoothness		| frequency		|
| 6-1) backlight constant color	| color					| saturation	|
| 6-2) backlight changing color	| speed of color chenge	| saturation	|
| 6-3) backlight running rainbow	| speed of rainbow		| rainbow step (width)	|
| 7) Running frequencies	| speed	of running frequencies	| sensitivity	|
| 8) Spectrum Analyzer	| color step (width of one color cell)	| starting (central) color	|


## Setting of lower noise threshold
- Manual: `AUTO_LOW_PASS = 0`, `EEPROM_LOW_PASS = 0`, set up `LOW_PASS` and `SPEKTR_LOW_PASS`;
- Automatically: `AUTO_LOW_PASS = 1`. While turning the colormusic on, music should be silenced;
- Reset with button: press the button five times (music should be silenced);
- (the best choice) From EEPROM: `AUTO_LOW_PASS = 0` and `EEPROM_LOW_PASS = 1`. Then
  * turn the colormusic on;
  * silence the music;
  * press the button five times;
  * the noise values will be written to the EEPROM and automatically read at every colormusic start up.
