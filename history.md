**2020.11.05**

- version: 1.1.1
- add: BRIGHTNESS is stored in EEPROM now and loaded at colormusic start up (if `EEPROM_LOW_PASS 1`);
- upd: EEPROM optimised - correct data types are stored and read;
- fix: `LOW_PASS` and `SPEKTR_LOW_PASS` had wrong sizes in EEPROM. Now they occupy 4 bytes each, so are compatible with SAMD chips integer data type size.

**2020.11.04**

- version: 1.1.0
- fix: variable `light_time` is initialised properly - after `STROBE_PERIOD` initialisation.
- add: now settings could be changed with the only one button (it's a madness). No need for a remote control. Variables' limits and changing steps are different for IR remote and button. It is made for convenient usage.
- upd: button behaviour
- upd: variables' limits and changing steps have been changed so that it is more convenient to adjust them with one button.
- upd: stroboscope now has a specific set of frequencies:
	25 Hz (1500 BPM - trance music x10), 20 Hz (1200 BPM - dance music x10), 15 Hz (900 BPM), 10 Hz (600 BPM) and 5 Hz (300 BPM).
- upd: some comments were translated.

**2020.06.18**

Regarding the Gyver's version 2.10
- version: 1.0.0
- fix: despite `define KEEP_SETTINGS 0`, EEPROM was updated from procedure `loop()`.
- fix: variables eeprom_flag and eeprom_timer were not defined. With no remote controls (#if REMOTE_TYPE != 0) they remained undefined.
- add: update `this_mode` in EEPROM after changing it by the button, not only IR remote.