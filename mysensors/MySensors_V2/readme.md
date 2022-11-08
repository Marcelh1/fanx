# This is a beta version for testing purposes

The low level eeprom read/write for storing the ventilation address was removed, this version makes use of MySensors eeprom write/read functions to prevent conflicts:
- loadState
- SaveState

The dongle will not go into clone mode each restart anymore, it will check for a stored address in eeprom, if so, it will immediately go to normal operation and skip clone mode. The dongle can be put in clone mode via Home Assistant, an additional binary switch will be created by the dongle. If the switch is activated in Home Assistant, the dongle will enter "clone mode" and stay into this mode for 5 seconds, if the timer expires, the switch in Home Assistant will be switched off by the dongle.

![usb dongle](https://github.com/Marcelh1/fanx/main/mysensors/MySensors_V2/HA2.png)

If cloning was succesful, the dongle will go into normal operation, the led will blink each 5 seconds to notify the user of active communication. If the cloning was not succesful, the led will constantly show a doubleblink pattern and the dongle will do nothing else. 

There are two ways to exit the doubleblink pattern mode:
1. Power cycle the dongle, it will read (previous) stored address in eeprom at start
2. Activate the clone switch in Home Assistant again to retry
