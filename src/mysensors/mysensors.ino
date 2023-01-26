#include <Arduino.h>
#include <avr/wdt.h>
#include "cc1101.h"
#define ONBOARD_LED                 7
#define ORCON_INTERVAL              5000  // RF link interval
#define LED_FLASH_TIME              25  // ms
#define TX_RETRY_CNT                5  // x

enum module_states
{
  JUST_BOOTED,
  RF15_PAIRINGSMODE,
  PAIRING_FAIL,
  NORMAL_MODE
};

CC1101 radio;

// Enable serial gateway
#define MY_GATEWAY_SERIAL
#define MY_BAUD_RATE                38400

//#define MY_DEBUG
#include <MySensors.h>
#define CHILD_ID_FAN                1
#define CHILD_ID_CLONE              2
#define CHILD_ID_TARGET_ADDRESS     3
#define CHILD_ID_SOURCE_ADDRESS     4

#define SN                          "FanX"
#define SV                          "1.1"

MyMessage msgSourceAddress(CHILD_ID_TARGET_ADDRESS, V_VAR1);
MyMessage msgTargetAddress(CHILD_ID_SOURCE_ADDRESS, V_VAR1);

MyMessage msgCloneSwitch(CHILD_ID_CLONE, V_STATUS);

MyMessage msgFANspeed(CHILD_ID_FAN, V_PERCENTAGE);
MyMessage msgFANstate(CHILD_ID_FAN, V_STATUS);

volatile int fan_speed_req = 1;
volatile int fan_speed = 1;
volatile int prev_fan_speed = 0;

module_states dongle_state = JUST_BOOTED;

//******************************************************************************************//
//                                                                                          //
//                        Init                                                              //
//                                                                                          //
//******************************************************************************************//

void setup()
{
  wdt_disable();
  // Setup locally attached sensors
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, HIGH);

  Serial1.begin(38400); // used for transmitting data to Orcon
  while (!Serial1);

  wdt_enable(WDTO_8S);
}

void before()
{
  radio.init();
}

void presentation()
{
  // Present locally attached sensors
  sendSketchInfo(SN, SV);
  present(CHILD_ID_FAN, S_DIMMER, "FAN speed");
  present(CHILD_ID_CLONE, S_BINARY, "Clone switch");
  present(CHILD_ID_TARGET_ADDRESS, S_CUSTOM, "Target address");
  present(CHILD_ID_SOURCE_ADDRESS, S_CUSTOM, "Source address");
}

//******************************************************************************************//
//                                                                                          //
//                        Led flash                                                         //
//                                                                                          //
//******************************************************************************************//
void led_flash(uint8_t flash_cnt)
{
  for (int i = 0; i < flash_cnt; i++)
  {
    digitalWrite(ONBOARD_LED, LOW);  // RADIO LED ON
    wait(LED_FLASH_TIME * 2);
    digitalWrite(ONBOARD_LED, HIGH); // RADIO LED OFF
    wait(LED_FLASH_TIME * 2);
  }
}

//******************************************************************************************//
//                                                                                          //
//                        MAIN                                                              //
//                                                                                          //
//******************************************************************************************//

void loop()
{

  static uint8_t tx_retry_cntr = 0;
  static unsigned long req_current_millis = 0;
  static unsigned long prev_req_current_millis = 0;
  static bool first_run_flag = true;

  switch (dongle_state)
  {
    case (JUST_BOOTED):

      led_flash(1);

      // Inform MySensors once
      send(msgSourceAddress.set(0));
      send(msgTargetAddress.set(0));
      send(msgCloneSwitch.set(0));
      send(msgFANspeed.set(0));
      send(msgFANstate.set(0));

      // Check communication with Radio chip
      if ((radio.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f) == 1)
      {
        // Read EEPROM address to check if there is an address known
        bool empty_eeprom = true;

        for (int i = 0; i < 6; i++)
        {
          radio.orcon_state.address[i] = loadState(i);
          if (radio.orcon_state.address[i] != 0xFF) empty_eeprom = false;
        }
                
        if (empty_eeprom)
          dongle_state = PAIRING_FAIL;
        else
        {
          sendNewSourceAddressToGateway();
          sendNewTargetAddressToGateway();
          dongle_state = NORMAL_MODE;
        }

      }
      else
        wait(1000);

      break;

    case (RF15_PAIRINGSMODE):

      digitalWrite(ONBOARD_LED, LOW);  // RADIO LED ON

      radio.set_rx_mode();
      if (radio.clone_mode())
      {
        // Store in EEPROM via MySensors function saveState
        for (int i = 0; i < 6; i++)
          saveState(i, radio.orcon_state.address[i]);

        // Update Gateway
        sendNewSourceAddressToGateway();
        sendNewTargetAddressToGateway();

        prev_req_current_millis = millis(); // prevent burst of transmissions
        dongle_state = NORMAL_MODE;
      }
      else
        dongle_state = PAIRING_FAIL;

      digitalWrite(ONBOARD_LED, HIGH); // RADIO LED OFF

      clone_mode_ended();

      break;

    case (PAIRING_FAIL):

      // LED error pattern
      led_flash(2);
      wait(750);

      break;

    case (NORMAL_MODE):

      if (fan_speed != fan_speed_req)                          // Set new data or fan speed request?
      {
        if (tx_retry_cntr < TX_RETRY_CNT)
        {
          if (radio.tx_orcon(fan_speed_req))                    // Succes, blink led
          {
            led_flash(1);
            fan_speed = radio.orcon_state.fan_speed;            // If ok, current fan speed should match requested one
          }
          tx_retry_cntr++;
        }
        else
          fan_speed = fan_speed_req;                            // Forget about this session, maybe more succes next time?
      }
      else
      {
        tx_retry_cntr = 0; // reset cntr

        // Request fan speed on regular interval ORCON_INTERVAL
        req_current_millis = millis();
        if((req_current_millis-prev_req_current_millis) > ORCON_INTERVAL)
        {
          prev_req_current_millis += ORCON_INTERVAL;

          if(radio.request_orcon_state())
          {
            led_flash(1);

            if(first_run_flag)                                  // Update once on first boot
            {
              fan_speed = radio.orcon_state.fan_speed;
              fan_speed_req = fan_speed;                        // Prevent RF update next cycle
              
              if(fan_speed == 0)
                update_fan_state(0);
              else
                update_fan_state(1);
              
              update_fan_speed(fan_speed);                
              
              first_run_flag = false;              
            }
            else if(fan_speed != radio.orcon_state.fan_speed)   // FAN SPEED CHANGED
            {
              if(radio.orcon_state.fan_speed == 0)              // DETERMINE TO REPORT FAN STATE CHANGE OF SPEED CHANGE TO CONTROLLER
              {
                prev_fan_speed = fan_speed;
                fan_speed = radio.orcon_state.fan_speed;
                fan_speed_req = fan_speed;                      // Prevent RF update next cycle              
                update_fan_state(0);
              }
              else
              {
                if(fan_speed == 0)                              // Fan is off, new value != 0, therefore: send once V_STATUS = TRUE
                  update_fan_state(1);                
  
                fan_speed = radio.orcon_state.fan_speed;
                fan_speed_req = fan_speed;                      // Prevent RF update next cycle
                update_fan_speed(fan_speed);
              }
            }
          }
        }
      }

      break;
  }

  wdt_reset();

}

//******************************************************************************************//
//                                                                                          //
//                        Receive from MySensors                                            //
//                                                                                          //
//******************************************************************************************//

void receive(const MyMessage &message)
{
  if(message.isAck())
  {
    //Serial.println("This is an ack from gateway");
    return;
  }

  if( (message.getType() == V_STATUS) && (message.getSensor() == CHILD_ID_CLONE) )
  {
    if(message.getBool())
    {
      dongle_state = RF15_PAIRINGSMODE;
      clone_mode_started();
    }
  }

  if(message.getSensor() == CHILD_ID_FAN)  // FAN
  {
    if(message.getType() == V_STATUS)      // ON, OFF SWITCH
    {
      if(!message.getBool())               // FAN OFF
      {
        prev_fan_speed = fan_speed;        // STORE FAN SPEED (FOR ONLY RECEIVING "V_STATUS = TRUE" NEXT TIME)
        fan_speed_req = 0;                 // SEND "0" TO ORCON
        update_fan_state(0);               // REPORT STATE BACK TO CONTROLLER        
      }
      else
      {
        fan_speed_req = prev_fan_speed;    // SET BACK FAN SPEED TO ORCON ONLY
        update_fan_state(1);               // REPORT BACK TO CONTROLLER
      }   
      
    }

    if (message.getType() == V_PERCENTAGE)
    {
      fan_speed_req = constrain( message.getInt(), 0, 100 );
      fan_speed_req = min(fan_speed_req, 4);      // Limit to 4
      fan_speed_req = max(fan_speed_req, 0);      // >= 0
      
      update_fan_speed(fan_speed_req);
    }

  }
  

}

//******************************************************************************************//
//                                                                                          //
//                        Transmit new state                                                //
//                                                                                          //
//******************************************************************************************//
void sendNewSourceAddressToGateway()
{
  // Ramses II format
  uint32_t source_address = ((uint32_t)radio.orcon_state.address[3]<<16) | ((uint32_t)radio.orcon_state.address[4]<<8) | ((uint32_t)radio.orcon_state.address[5]<<0);
  uint8_t device_id = ((source_address & 0xFC0000)>>18);
  uint32_t address_id = source_address & 0x03FFFF;  
  String result_string = String(device_id) + ":" + String(address_id);
  
  send(msgSourceAddress.set(result_string.c_str()));
}

void sendNewTargetAddressToGateway()
{
  // Ramses II format
  uint32_t target_address = ((uint32_t)radio.orcon_state.address[0]<<16) | ((uint32_t)radio.orcon_state.address[1]<<8) | ((uint32_t)radio.orcon_state.address[2]<<0);
  uint8_t device_id = ((target_address & 0xFC0000)>>18);
  uint32_t address_id = target_address & 0x03FFFF; 
  String result_string = String(device_id) + ":" + String(address_id);
    
  send(msgTargetAddress.set(result_string.c_str()));
}

void update_fan_speed(int16_t speed_level)
{
  send(msgFANspeed.set(speed_level));
}

void update_fan_state(int16_t fan_state)
{
  send(msgFANstate.set(fan_state));
}

void clone_mode_ended(void)
{
  send(msgCloneSwitch.set(0));
}

void clone_mode_started(void)
{
  send(msgCloneSwitch.set(1));
}
