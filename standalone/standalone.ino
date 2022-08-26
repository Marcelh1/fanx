#include <Arduino.h>
#include "cc1101.h"
#define REQUEST_RATE                2500  // ms
#define ONBOARD_LED                 7
#define LED_FLASH_TIME              50  // ms

enum module_states
{
  JUST_BOOTED,
  RF15_PAIRINGSMODE,
  NORMAL_MODE
};

CC1101 radio;

//******************************************************************************************//
//                                                                                          //
//                        Led flash                                                         //
//                                                                                          //
//******************************************************************************************//

void led_flash_once_ms(int blink_time)
{
  digitalWrite(ONBOARD_LED, LOW);  // RADIO LED ON
  delay(blink_time);
  digitalWrite(ONBOARD_LED, HIGH); // RADIO LED OFF   
}

//******************************************************************************************//
//                                                                                          //
//                        Init                                                              //
//                                                                                          //
//******************************************************************************************//
void setup()
{
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, HIGH);

  Serial.begin(9600); // Virtual comport via USB
  while(!Serial);
  
  Serial1.begin(38400); // used for transmitting data to Orcon  
  while(!Serial1);
}

//******************************************************************************************//
//                                                                                          //
//                        MAIN                                                              //
//                                                                                          //
//******************************************************************************************//
void loop()
{
  module_states dongle_state = JUST_BOOTED;
  int prev_speed_lvl = 0;
  
  while (1)
  {
    switch (dongle_state)
    {
      case (JUST_BOOTED):
        radio.init();

        if ((radio.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) &0x1f) == 1)
        {
          Serial.println("> CC1101 radio initialized");
          dongle_state = RF15_PAIRINGSMODE;
          //dongle_state = NORMAL_MODE; //skip clone mode
        }
        else
        {
          Serial.println("> Something went wrong with radio init, will try again");
          delay(1000);
        }
        break;

      case (RF15_PAIRINGSMODE):

        Serial.println("> Enter cloning mode ");
        Serial.println("> Press button on RF15");
        
        digitalWrite(ONBOARD_LED, LOW);  // RADIO LED ON
        radio.set_rx_mode();

        if (radio.clone_mode()) // try to clone system, timeouts after 5sec 
          Serial.println("> Clone succesfull!\n");
        else
          Serial.println("> No RF15 messages received!\n");
        
        digitalWrite(ONBOARD_LED, HIGH); // RADIO LED OFF

        /*EEPROM ADDRESS
         *0 - 2 => RF15 address
         *3 - 5 => ORCON broadcast address
         */
        Serial.print("> Using source device id: ");
        for (int i = 0; i < 3; i++)
        {
          Serial.print(EEPROM.read(i), HEX);
          Serial.print(" ");
        }

        Serial.println("");
        Serial.print("> Using target device id: ");
        for (int i = 3; i < 6; i++)
        {
          Serial.print(EEPROM.read(i), HEX);
          Serial.print(" ");
        }

        Serial.println("");
        Serial.println("");
        Serial.println("> Enter idle mode");

        dongle_state = NORMAL_MODE;

        break;

      case (NORMAL_MODE):

        if(radio.request_orcon_state())
        {
          
          if(prev_speed_lvl != radio.orcon_state.fan_speed)
          {
            Serial.print("FAN Speed changed: ");
            Serial.println(radio.orcon_state.fan_speed);
            prev_speed_lvl = radio.orcon_state.fan_speed;
          }
          led_flash_once_ms(LED_FLASH_TIME);
        }
        else
          Serial.println("Error requesting fan speed...");
          
        delay(REQUEST_RATE);

        break;
    }
  }
}
