# This is beta version for testing purposes

The low level eeprom read/write for ventilation address was removed, this version makes use of MySensors eeprom write/read functions to prevent conflicts:
- loadState
- SaveState

The dongle will not go into clone mode each restart, it will check if there is a address known, if so, it will immediately go to normal operation and skip clone mode. The dongle can be put in clone mode via Home Assistant, an additional binary switch will be created by the Dongle. If the switch is activated in Home Assistant, the dongle will enter "clone mode" and stay into this mode for 5 seconds, if the timer expires, the switch in Home Assistant will be switched off by the dongle.

If cloning was succesful, the Dongle will go into normal operation, the led will blink each 5 seconds to notify the user of communication. If the cloning was not succesful, the led will show a doubleblink pattern and the dongle will do nothing else. If the dongle is power cycled, it will always start with known address in eeprom.
