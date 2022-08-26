#include <Firmata.h>
#include <Arduino.h>
#include "cc1101.h"

// Note! Patch in Firmata.cpp to wait for Serial ready: TBD
// while(!Serial);

// the minimum interval for sampling analog input
#define MINIMUM_SAMPLING_INTERVAL   1

#define FAN_ANALOG_PIN              0     // used to report current fan speed
#define FAN_PWM_PIN                 3     // used to control fan speed
#define ORCON_INTERVAL              5000  // RF link interval
#define ONBOARD_LED                 7

#define LED_FLASH_TIME              25  // ms

#define TX_RETRY_CNT                5  // x

enum module_states
{
  JUST_BOOTED,
  RF15_PAIRINGSMODE,
  FIRMATA_INIT,
  NORMAL_MODE
};

CC1101 radio;

/*==============================================================================
   GLOBAL VARIABLES
  ============================================================================*/

/* analog inputs */
int analogInputsToReport = 0; // bitwise array to store pin reporting

/* digital input ports */
byte reportPINs[TOTAL_PORTS];       // 1 = report this port, 0 = silence
byte previousPINs[TOTAL_PORTS];     // previous 8 bits sent

/* pins configuration */
byte portConfigInputs[TOTAL_PORTS]; // each bit: 1 = pin in INPUT, 0 = anything else

/* timer variables */
unsigned long currentMillis;        // store the current value from millis()
unsigned long previousMillis;       // for comparison with currentMillis
unsigned int samplingInterval = 19; // how often to run the main loop (in ms)

boolean isResetting = false;

// Forward declare a few functions to avoid compiler errors with older versions
// of the Arduino IDE.
void setPinModeCallback(byte, int);
void reportAnalogCallback(byte analogPin, int value);
void sysexCallback(byte, byte, byte*);

volatile int fan_speed_req = 0;

/*==============================================================================
   FUNCTIONS
  ============================================================================*/

void outputPort(byte portNumber, byte portValue, byte forceSend)
{
  // pins not configured as INPUT are cleared to zeros
  portValue = portValue & portConfigInputs[portNumber];
  // only send if the value is different than previously sent
  if (forceSend || previousPINs[portNumber] != portValue) {
    Firmata.sendDigitalPort(portNumber, portValue);
    previousPINs[portNumber] = portValue;
  }
}

/* -----------------------------------------------------------------------------
   check all the active digital inputs for change of state, then add any events
   to the Serial output queue using Serial.print() */
void checkDigitalInputs(void)
{
  /* Using non-looping code allows constants to be given to readPort().
     The compiler will apply substantial optimizations if the inputs
     to readPort() are compile-time constants. */
  if (TOTAL_PORTS > 0 && reportPINs[0]) outputPort(0, readPort(0, portConfigInputs[0]), false);
  if (TOTAL_PORTS > 1 && reportPINs[1]) outputPort(1, readPort(1, portConfigInputs[1]), false);
  if (TOTAL_PORTS > 2 && reportPINs[2]) outputPort(2, readPort(2, portConfigInputs[2]), false);
  if (TOTAL_PORTS > 3 && reportPINs[3]) outputPort(3, readPort(3, portConfigInputs[3]), false);
  if (TOTAL_PORTS > 4 && reportPINs[4]) outputPort(4, readPort(4, portConfigInputs[4]), false);
  if (TOTAL_PORTS > 5 && reportPINs[5]) outputPort(5, readPort(5, portConfigInputs[5]), false);
  if (TOTAL_PORTS > 6 && reportPINs[6]) outputPort(6, readPort(6, portConfigInputs[6]), false);
  if (TOTAL_PORTS > 7 && reportPINs[7]) outputPort(7, readPort(7, portConfigInputs[7]), false);
  if (TOTAL_PORTS > 8 && reportPINs[8]) outputPort(8, readPort(8, portConfigInputs[8]), false);
  if (TOTAL_PORTS > 9 && reportPINs[9]) outputPort(9, readPort(9, portConfigInputs[9]), false);
  if (TOTAL_PORTS > 10 && reportPINs[10]) outputPort(10, readPort(10, portConfigInputs[10]), false);
  if (TOTAL_PORTS > 11 && reportPINs[11]) outputPort(11, readPort(11, portConfigInputs[11]), false);
  if (TOTAL_PORTS > 12 && reportPINs[12]) outputPort(12, readPort(12, portConfigInputs[12]), false);
  if (TOTAL_PORTS > 13 && reportPINs[13]) outputPort(13, readPort(13, portConfigInputs[13]), false);
  if (TOTAL_PORTS > 14 && reportPINs[14]) outputPort(14, readPort(14, portConfigInputs[14]), false);
  if (TOTAL_PORTS > 15 && reportPINs[15]) outputPort(15, readPort(15, portConfigInputs[15]), false);
}

// -----------------------------------------------------------------------------
/* sets the pin mode to the correct state and sets the relevant bits in the
   two bit-arrays that track Digital I/O and PWM status
*/
void setPinModeCallback(byte pin, int mode)
{
  if (Firmata.getPinMode(pin) == PIN_MODE_IGNORE)
    return;

  if (IS_PIN_ANALOG(pin)) {
    reportAnalogCallback(PIN_TO_ANALOG(pin), mode == PIN_MODE_ANALOG ? 1 : 0); // turn on/off reporting
  }
  if (IS_PIN_DIGITAL(pin)) {
    if (mode == INPUT || mode == PIN_MODE_PULLUP) {
      portConfigInputs[pin / 8] |= (1 << (pin & 7));
    } else {
      portConfigInputs[pin / 8] &= ~(1 << (pin & 7));
    }
  }
  Firmata.setPinState(pin, 0);
  switch (mode) {
    case PIN_MODE_ANALOG:
      if (IS_PIN_ANALOG(pin)) {
        if (IS_PIN_DIGITAL(pin)) {
          //pinMode(PIN_TO_DIGITAL(pin), INPUT);    // disable output driver
#if ARDUINO <= 100
          // deprecated since Arduino 1.0.1 - TODO: drop support in Firmata 2.6
          //digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable internal pull-ups
#endif
        }
        Firmata.setPinMode(pin, PIN_MODE_ANALOG);
      }
      break;
    case INPUT:
      if (IS_PIN_DIGITAL(pin)) {
        //pinMode(PIN_TO_DIGITAL(pin), INPUT);    // disable output driver
#if ARDUINO <= 100
        // deprecated since Arduino 1.0.1 - TODO: drop support in Firmata 2.6
        //digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable internal pull-ups
#endif
        Firmata.setPinMode(pin, INPUT);
      }
      break;
    case PIN_MODE_PULLUP:
      if (IS_PIN_DIGITAL(pin)) {
        //pinMode(PIN_TO_DIGITAL(pin), INPUT_PULLUP);
        Firmata.setPinMode(pin, PIN_MODE_PULLUP);
        Firmata.setPinState(pin, 1);
      }
      break;
    case OUTPUT:
      if (IS_PIN_DIGITAL(pin)) {
        if (Firmata.getPinMode(pin) == PIN_MODE_PWM) {
          // Disable PWM if pin mode was previously set to PWM.
          //digitalWrite(PIN_TO_DIGITAL(pin), LOW);
        }
        //pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
        Firmata.setPinMode(pin, OUTPUT);
      }
      break;
    case PIN_MODE_PWM:
      if (IS_PIN_PWM(pin)) {
        //pinMode(PIN_TO_PWM(pin), OUTPUT);
        //analogWrite(PIN_TO_PWM(pin), 0);
        Firmata.setPinMode(pin, PIN_MODE_PWM);
      }
      break;
    default:
      Firmata.sendString("Unknown pin mode"); // TODO: put error msgs in EEPROM
  }
  // TODO: save status to EEPROM here, if changed
}

/*
   Sets the value of an individual pin. Useful if you want to set a pin value but
   are not tracking the digital port state.
   Can only be used on pins configured as OUTPUT.
   Cannot be used to enable pull-ups on Digital INPUT pins.
*/
void setPinValueCallback(byte pin, int value)
{
  if (pin < TOTAL_PINS && IS_PIN_DIGITAL(pin)) {
    if (Firmata.getPinMode(pin) == OUTPUT) {
      Firmata.setPinState(pin, value);
      //digitalWrite(PIN_TO_DIGITAL(pin), value);
    }
  }
}

void analogWriteCallback(byte pin, int value)
{
  if (pin < TOTAL_PINS) {
    switch (Firmata.getPinMode(pin)) {
      case PIN_MODE_PWM:
        if (IS_PIN_PWM(pin))
        {
          if (pin == FAN_PWM_PIN)
          {
            fan_speed_req = value;
            fan_speed_req = min(fan_speed_req, 4);      // Limit to 4
            fan_speed_req = max(fan_speed_req, 0);      // Not negative
          }
          //else
          //analogWrite(PIN_TO_PWM(pin), value);
        }
        Firmata.setPinState(pin, value);
        break;
    }
  }
}

void digitalWriteCallback(byte port, int value)
{
  byte pin, lastPin, pinValue, mask = 1, pinWriteMask = 0;

  if (port < TOTAL_PORTS) {
    // create a mask of the pins on this port that are writable.
    lastPin = port * 8 + 8;
    if (lastPin > TOTAL_PINS) lastPin = TOTAL_PINS;
    for (pin = port * 8; pin < lastPin; pin++) {
      // do not disturb non-digital pins (eg, Rx & Tx)
      if (IS_PIN_DIGITAL(pin)) {
        // do not touch pins in PWM, ANALOG, SERVO or other modes
        if (Firmata.getPinMode(pin) == OUTPUT || Firmata.getPinMode(pin) == INPUT) {
          pinValue = ((byte)value & mask) ? 1 : 0;
          if (Firmata.getPinMode(pin) == OUTPUT) {
            pinWriteMask |= mask;
          } else if (Firmata.getPinMode(pin) == INPUT && pinValue == 1 && Firmata.getPinState(pin) != 1) {
            // only handle INPUT here for backwards compatibility
#if ARDUINO > 100
            //pinMode(pin, INPUT_PULLUP);
#else
            // only write to the INPUT pin to enable pullups if Arduino v1.0.0 or earlier
            pinWriteMask |= mask;
#endif
          }
          Firmata.setPinState(pin, pinValue);
        }
      }
      mask = mask << 1;
    }
    writePort(port, (byte)value, pinWriteMask);
  }
}


// -----------------------------------------------------------------------------
/* sets bits in a bit array (int) to toggle the reporting of the analogIns
*/
//void FirmataClass::setAnalogPinReporting(byte pin, byte state) {
//}
void reportAnalogCallback(byte analogPin, int value)
{
  if (analogPin < TOTAL_ANALOG_PINS) {
    if (value == 0) {
      analogInputsToReport = analogInputsToReport & ~ (1 << analogPin);
    } else {
      analogInputsToReport = analogInputsToReport | (1 << analogPin);
      // prevent during system reset or all analog pin values will be reported
      // which may report noise for unconnected analog pins
      if (!isResetting) {
        // Send pin value immediately. This is helpful when connected via
        // ethernet, wi-fi or bluetooth so pin states can be known upon
        // reconnecting.
        Firmata.sendAnalog(analogPin, radio.orcon_state.fan_speed);
      }
    }
  }
  // TODO: save status to EEPROM here, if changed
}

void reportDigitalCallback(byte port, int value)
{
  if (port < TOTAL_PORTS) {
    reportPINs[port] = (byte)value;
    // Send port value immediately. This is helpful when connected via
    // ethernet, wi-fi or bluetooth so pin states can be known upon
    // reconnecting.
    if (value) outputPort(port, readPort(port, portConfigInputs[port]), true);
  }
  // do not disable analog reporting on these 8 pins, to allow some
  // pins used for digital, others analog.  Instead, allow both types
  // of reporting to be enabled, but check if the pin is configured
  // as analog when sampling the analog inputs.  Likewise, while
  // scanning digital pins, portConfigInputs will mask off values from any
  // pins configured as analog
}

/*==============================================================================
   SYSEX-BASED commands
  ============================================================================*/

void sysexCallback(byte command, byte argc, byte *argv)
{
  switch (command) {
    case SAMPLING_INTERVAL:
      if (argc > 1) {
        samplingInterval = argv[0] + (argv[1] << 7);
        if (samplingInterval < MINIMUM_SAMPLING_INTERVAL) {
          samplingInterval = MINIMUM_SAMPLING_INTERVAL;
        }
      } else {
        //Firmata.sendString("Not enough data");
      }
      break;
    case EXTENDED_ANALOG:
      if (argc > 1) {
        int val = argv[1];
        if (argc > 2) val |= (argv[2] << 7);
        if (argc > 3) val |= (argv[3] << 14);
        analogWriteCallback(argv[0], val);
      }
      break;
    case CAPABILITY_QUERY:
      Firmata.write(START_SYSEX);
      Firmata.write(CAPABILITY_RESPONSE);
      for (byte pin = 0; pin < TOTAL_PINS; pin++) {
        if (IS_PIN_DIGITAL(pin)) {
          Firmata.write((byte)INPUT);
          Firmata.write(1);
          Firmata.write((byte)PIN_MODE_PULLUP);
          Firmata.write(1);
          Firmata.write((byte)OUTPUT);
          Firmata.write(1);
        }
        if (IS_PIN_ANALOG(pin)) {
          Firmata.write(PIN_MODE_ANALOG);
          Firmata.write(10); // 10 = 10-bit resolution
        }
        if (IS_PIN_PWM(pin)) {
          Firmata.write(PIN_MODE_PWM);
          Firmata.write(DEFAULT_PWM_RESOLUTION);
        }
        if (IS_PIN_DIGITAL(pin)) {
          Firmata.write(PIN_MODE_SERVO);
          Firmata.write(14);
        }
        Firmata.write(127);
      }
      Firmata.write(END_SYSEX);
      break;
    case PIN_STATE_QUERY:
      if (argc > 0) {
        byte pin = argv[0];
        Firmata.write(START_SYSEX);
        Firmata.write(PIN_STATE_RESPONSE);
        Firmata.write(pin);
        if (pin < TOTAL_PINS) {
          Firmata.write(Firmata.getPinMode(pin));
          Firmata.write((byte)Firmata.getPinState(pin) & 0x7F);
          if (Firmata.getPinState(pin) & 0xFF80) Firmata.write((byte)(Firmata.getPinState(pin) >> 7) & 0x7F);
          if (Firmata.getPinState(pin) & 0xC000) Firmata.write((byte)(Firmata.getPinState(pin) >> 14) & 0x7F);
        }
        Firmata.write(END_SYSEX);
      }
      break;
    case ANALOG_MAPPING_QUERY:
      Firmata.write(START_SYSEX);
      Firmata.write(ANALOG_MAPPING_RESPONSE);
      for (byte pin = 0; pin < TOTAL_PINS; pin++) {
        Firmata.write(IS_PIN_ANALOG(pin) ? PIN_TO_ANALOG(pin) : 127);
      }
      Firmata.write(END_SYSEX);
      break;

  }
}

/*==============================================================================
   SETUP()
  ============================================================================*/

void systemResetCallback()
{
  isResetting = true;

  // initialize a defalt state
  // TODO: option to load config from EEPROM instead of default

  for (byte i = 0; i < TOTAL_PORTS; i++) {
    reportPINs[i] = false;    // by default, reporting off
    portConfigInputs[i] = 0;  // until activated
    previousPINs[i] = 0;
  }

  for (byte i = 0; i < TOTAL_PINS; i++) {
    // pins with analog capability default to analog input
    // otherwise, pins default to digital output
    if (IS_PIN_ANALOG(i)) {
      // turns off pullup, configures everything
      setPinModeCallback(i, PIN_MODE_ANALOG);
    } else if (IS_PIN_DIGITAL(i)) {
      // sets the output to 0, configures portConfigInputs
      setPinModeCallback(i, OUTPUT);
    }

  }
  // by default, do not report any analog inputs
  analogInputsToReport = 0;


  /* send digital inputs to set the initial state on the host computer,
     since once in the loop(), this firmware will only send on change */
  /*
    TODO: this can never execute, since no pins default to digital input
        but it will be needed when/if we support EEPROM stored config
    for (byte i=0; i < TOTAL_PORTS; i++) {
    outputPort(i, readPort(i, portConfigInputs[i]), true);
    }
  */
  isResetting = false;
}

void setup()
{

  Serial1.begin(38400); // used for transmitting data to Orcon

  while (!Serial1) {
    ; // wait for serial1 port to connect. Needed for native USB port only
  }
  
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, HIGH);

}

void led_flash_once_ms(int blink_time)
{
  digitalWrite(ONBOARD_LED, LOW);  // RADIO LED ON
  delay(blink_time);
  digitalWrite(ONBOARD_LED, HIGH); // RADIO LED OFF
}

/*==============================================================================
   LOOP()
  ============================================================================*/
void loop()
{
  byte pin, analogPin;
  module_states dongle_state = JUST_BOOTED;

  unsigned long req_current_millis = 0;
  unsigned long prev_req_current_millis = 0;
  int prev_fan_speed_req = 0;
  uint8_t tx_retry_cntr = 0;
  bool sync_flag = false;

  while (1)
  {
    switch (dongle_state)
    {
      case (JUST_BOOTED):
        radio.init();

        if ((radio.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f) == 1)
          dongle_state = RF15_PAIRINGSMODE;
        else
          delay(1000);
        break;

      case (RF15_PAIRINGSMODE):
        digitalWrite(ONBOARD_LED, LOW);  // RADIO LED ON
        radio.set_rx_mode();
        radio.clone_mode();
        digitalWrite(ONBOARD_LED, HIGH); // RADIO LED OFF
        dongle_state = FIRMATA_INIT;

        break;

      case (FIRMATA_INIT):
        Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);

        Firmata.attach(ANALOG_MESSAGE, analogWriteCallback);
        Firmata.attach(DIGITAL_MESSAGE, digitalWriteCallback);
        Firmata.attach(REPORT_ANALOG, reportAnalogCallback);
        Firmata.attach(REPORT_DIGITAL, reportDigitalCallback);
        Firmata.attach(SET_PIN_MODE, setPinModeCallback);
        Firmata.attach(SET_DIGITAL_PIN_VALUE, setPinValueCallback);
        Firmata.attach(START_SYSEX, sysexCallback);
        Firmata.attach(SYSTEM_RESET, systemResetCallback);

        // to use a port other than Serial, such as Serial1 on an Arduino Leonardo or Mega,
        // Call begin(baud) on the alternate serial port and pass it to Firmata to begin like this:
        // Serial1.begin(57600);
        // Firmata.begin(Serial1);
        // However do not do this if you are using SERIAL_MESSAGE
        
        Serial.begin(57600);
        while (!Serial) {
          ; // wait for serial port to connect. Needed for ATmega32u4-based boards and Arduino 101
        }
        Firmata.begin(Serial);

        systemResetCallback();  // reset to default config

        dongle_state = NORMAL_MODE;

        break;

      case (NORMAL_MODE):
       
          if (prev_fan_speed_req != fan_speed_req)                  // Set new data or fan speed request?
          {
            if (tx_retry_cntr < TX_RETRY_CNT)
            {
              if (radio.tx_orcon(fan_speed_req))                    // Succes, blink led
              {
                led_flash_once_ms(LED_FLASH_TIME);
                prev_fan_speed_req = radio.orcon_state.fan_speed;   // If ok, current fan speed should match requested one
              }
              tx_retry_cntr++;
            }
            else
              prev_fan_speed_req = fan_speed_req;                   // Forget about this session, maybe more succes next time?
          }
          else
          {
            tx_retry_cntr = 0; // reset cntr
  
            // Request fan speed on regular interval ORCON_INTERVAL
            req_current_millis = millis();
            if ( (req_current_millis - prev_req_current_millis) > ORCON_INTERVAL)
            {
              prev_req_current_millis += ORCON_INTERVAL;
              
              if (radio.request_orcon_state())
                led_flash_once_ms(LED_FLASH_TIME);
            }
          }

        checkDigitalInputs();

        while (Firmata.available())
          Firmata.processInput();

        // TODO - ensure that Stream buffer doesn't go over 60 bytes
        currentMillis = millis();        
        if (currentMillis - previousMillis > samplingInterval)
        {
          previousMillis += samplingInterval;

          /* ANALOGREAD - do all analogReads() at the configured sampling interval */
          for (pin = 0; pin < TOTAL_PINS; pin++) {
            if (IS_PIN_ANALOG(pin) && Firmata.getPinMode(pin) == PIN_MODE_ANALOG)
            {
              analogPin = PIN_TO_ANALOG(pin);
              if (analogInputsToReport & (1 << analogPin))
              {
                if (analogPin == FAN_ANALOG_PIN)
                  Firmata.sendAnalog(analogPin, radio.orcon_state.fan_speed);
                else
                  Firmata.sendAnalog(analogPin, 0);
              }
            }
          }        
        }

        break;
    }
  }

}
