## Limits and any interupts

You may to read about how the switchs has connected on [Wiring Limit Switches](https://github.com/gnea/grbl/wiki/Wiring-Limit-Switches).

They need will connect to ground for triggered.
If use normally-open (NO) switches (default), they will trigger when to connect to ground.
If use normally-close (NC) switches, they will trigger when lost connect with ground.

### Limit

- INVERT_LIMIT_PIN_MASK - if he is set, then use NC switches. 
- setting.flag.BITFLAG_INVERT_LIMIT_PINS - not support

### Control

- INVERT_CONTROL_PIN_MASK - don't support. Instead you need use CONTROL_PIN_USE_NC.
