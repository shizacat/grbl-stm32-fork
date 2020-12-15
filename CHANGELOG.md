# Changelog

## [Unreleased]

- To base code GRBL version 1.1h
- Enable/Disable spindle Pin works now
- Change pinout. Direction XYZ to GPIOA4,5,6. Enable Stepper to GPIOB0.
- Change pinout. Limit XYZ to GPIOB12,13,14. Spindle En and Dir to GPIOB1,2.
- Replace SPL on STM32Cube.
- Control pins have work now. Change logick work.
  Add new config flag - CONTROL_PIN_USE_NC. Mask for invert don't support.
- Update code for settings and work with eeprom.

## [0.3]

- Update Full Speed device library to 4.1.0
- Update Virtual Com Port. Misc.h - add type bool.
- Looked at [issue#46](https://github.com/usbcnc/grbl/issues/46) from usbcnc.
  Minor change.

## [0.2]

- uint16_t probe_invert_mask
- [issue#36](https://github.com/usbcnc/grbl/issues/36) from usbcnc
- [issue#38](https://github.com/usbcnc/grbl/issues/38) from usbcnc
- [issue#40](https://github.com/usbcnc/grbl/issues/40) from usbcnc
- [issue#41](https://github.com/usbcnc/grbl/issues/41) from usbcnc
- [issue#49](https://github.com/usbcnc/grbl/issues/49) from usbcnc

## [0.1]

- To PlatformIO