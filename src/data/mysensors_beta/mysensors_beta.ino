#include <Arduino.h>
#include <avr/wdt.h>
#include "cc1101.h"
#define ONBOARD_LED               7
#define FAN_INTERVAL              5000  // RF link interval
#define LED_FLASH_TIME            25  // ms
#define TX_RETRY_CNT              5  // x

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

#define CHILD_ID_INDOOR_HUM         5
#define CHILD_ID_OUTDOOR_HUM        6
#define CHILD_ID_EXHAUST_TEMP       7
#define CHILD_ID_SUPPLY_TEMP        8
#define CHILD_ID_BYPASS_POS         9
#define CHILD_ID_EXHAUST_FANSPEED   10
#define CHILD_ID_SUPPLY_FANSPEED    11
#define CHILD_ID_SUPPLY_FLOW        12
#define CHILD_ID_EXHAUST_FLOW       13
#define CHILD_ID_INDOOR_TEMP        14
#define CHILD_ID_OUTDOOR_TEMP       15

#define CHILD_ID_BYPASS_MODE        16

#define SN                          "FanX"
#define SV                          "1.5"

MyMessage msgSourceAddress(CHILD_ID_TARGET_ADDRESS, V_VAR1);
MyMessage msgTargetAddress(CHILD_ID_SOURCE_ADDRESS, V_VAR1);
MyMessage msgCloneSwitch(CHILD_ID_CLONE, V_STATUS);
MyMessage msgFANspeed(CHILD_ID_FAN, V_PERCENTAGE);
MyMessage msgFANstate(CHILD_ID_FAN, V_STATUS);

MyMessage msgIndoorHUM(CHILD_ID_INDOOR_HUM, V_HUM);
MyMessage msgOutdoorHUM(CHILD_ID_OUTDOOR_HUM, V_HUM);

MyMessage msgExhaustTEMP(CHILD_ID_EXHAUST_TEMP, V_TEMP);
MyMessage msgSupplyTEMP(CHILD_ID_SUPPLY_TEMP, V_TEMP);

MyMessage msgIndoorTEMP(CHILD_ID_INDOOR_TEMP, V_TEMP);
MyMessage msgOutdoorTEMP(CHILD_ID_OUTDOOR_TEMP, V_TEMP);

MyMessage msgBypassPOS(CHILD_ID_BYPASS_POS, V_VAR1);

MyMessage msgExhaustFAN(CHILD_ID_EXHAUST_FANSPEED, V_VAR1);
MyMessage msgSupplyFAN(CHILD_ID_SUPPLY_FANSPEED, V_VAR1);

MyMessage msgSupplyFLOW(CHILD_ID_SUPPLY_FLOW, V_VAR1);
MyMessage msgExhaustFLOW(CHILD_ID_EXHAUST_FLOW, V_VAR1);

MyMessage msgBypassMODE(CHILD_ID_BYPASS_MODE, V_VAR1);

volatile uint8_t fan_bypass_mode_req = 0;

volatile int fan_speed_req = 1;
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
  Serial1.begin(38400); // used for transmitting data to FAN
  while (!Serial1);
  radio.current_fan_state.fan_speed = 1;
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

  present(CHILD_ID_INDOOR_HUM, S_HUM, "Indoor humidity (%)");
  present(CHILD_ID_OUTDOOR_HUM, S_HUM, "Outdoor humidity (%)");  
  present(CHILD_ID_EXHAUST_TEMP, S_TEMP, "Exhaust temperature (ºC)");
  present(CHILD_ID_SUPPLY_TEMP, S_TEMP, "Supply temperature (ºC)");

  present(CHILD_ID_BYPASS_POS, S_CUSTOM, "Bypass position (%)");
  present(CHILD_ID_EXHAUST_FANSPEED, S_CUSTOM, "Exhaust FAN (%)");
  present(CHILD_ID_SUPPLY_FANSPEED, S_CUSTOM, "Supply FAN (%)");  
  
  present(CHILD_ID_SUPPLY_FLOW, S_CUSTOM, "Supply flow (m3/h)");
  present(CHILD_ID_EXHAUST_FLOW, S_CUSTOM, "Exhaust flow (m3/h)");

  present(CHILD_ID_INDOOR_TEMP, S_TEMP, "Indoor temperature (ºC)");
  present(CHILD_ID_OUTDOOR_TEMP, S_TEMP, "Outdoor temperature (ºC)");  

  present(CHILD_ID_BYPASS_MODE, S_CUSTOM, "Bypass mode (0, 1, 2)");
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
//                        Update new params to controller                                   //
//                                                                                          //
//******************************************************************************************//
void update_new_params()
{
  float temperature, flow;
  
  if(radio.current_fan_state.indoor_humidity != radio.new_fan_state.indoor_humidity)
  {
    send(msgIndoorHUM.set(radio.new_fan_state.indoor_humidity));
    radio.current_fan_state.indoor_humidity = radio.new_fan_state.indoor_humidity;                
  }
  if(radio.current_fan_state.outdoor_humidity != radio.new_fan_state.outdoor_humidity)
  {
    send(msgOutdoorHUM.set(radio.new_fan_state.outdoor_humidity));
    radio.current_fan_state.outdoor_humidity = radio.new_fan_state.outdoor_humidity;                
  }
  if(radio.current_fan_state.exhaust_temperature != radio.new_fan_state.exhaust_temperature)
  {
    temperature = (radio.new_fan_state.exhaust_temperature/100.0);
    send(msgExhaustTEMP.set(temperature, 1));
    radio.current_fan_state.exhaust_temperature != radio.new_fan_state.exhaust_temperature;
  }
  if(radio.current_fan_state.supply_temperature != radio.new_fan_state.supply_temperature)
  {
    temperature = (radio.new_fan_state.supply_temperature/100.0);
    send(msgSupplyTEMP.set(temperature, 1));
    radio.current_fan_state.supply_temperature != radio.new_fan_state.supply_temperature;
  }
  if(radio.current_fan_state.indoor_temperature != radio.new_fan_state.indoor_temperature)
  {
    temperature = (radio.new_fan_state.indoor_temperature/100.0);
    send(msgIndoorTEMP.set(temperature, 1));
    radio.current_fan_state.indoor_temperature != radio.new_fan_state.indoor_temperature;
  }
  if(radio.current_fan_state.outdoor_temperature != radio.new_fan_state.outdoor_temperature)
  {
    temperature = (radio.new_fan_state.outdoor_temperature/100.0);
    send(msgOutdoorTEMP.set(temperature, 1));
    radio.current_fan_state.outdoor_temperature != radio.new_fan_state.outdoor_temperature;
  }  
  if(radio.current_fan_state.bypass_position != radio.new_fan_state.bypass_position)
  {
    send(msgBypassPOS.set(radio.new_fan_state.bypass_position));
    radio.current_fan_state.bypass_position != radio.new_fan_state.bypass_position;
  }  
  if(radio.current_fan_state.exhaust_fanspeed != radio.new_fan_state.exhaust_fanspeed)
  {
    send(msgExhaustFAN.set(radio.new_fan_state.exhaust_fanspeed));
    radio.current_fan_state.exhaust_fanspeed != radio.new_fan_state.exhaust_fanspeed;
  }    
  if(radio.current_fan_state.supply_fanspeed != radio.new_fan_state.supply_fanspeed)
  {
    send(msgSupplyFAN.set(radio.new_fan_state.supply_fanspeed));
    radio.current_fan_state.supply_fanspeed != radio.new_fan_state.supply_fanspeed;
  }   

  if(radio.current_fan_state.supply_flow != radio.new_fan_state.supply_flow)
  {
    flow = ((radio.new_fan_state.supply_flow/100.0)*3.6); // l/s => m3/h
    send(msgSupplyFLOW.set(flow, 1));
    radio.current_fan_state.supply_flow != radio.new_fan_state.supply_flow;
  } 

  if(radio.current_fan_state.exhaust_flow != radio.new_fan_state.exhaust_flow)
  {
    flow = ((radio.new_fan_state.exhaust_flow/100.0)*3.6); // l/s => m3/h
    send(msgExhaustFLOW.set(flow, 1));
    radio.current_fan_state.exhaust_flow != radio.new_fan_state.exhaust_flow;
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
      send(msgIndoorHUM.set(0));
      send(msgOutdoorHUM.set(0));
      send(msgExhaustTEMP.set(0));
      send(msgSupplyTEMP.set(0));
      send(msgIndoorTEMP.set(0));
      send(msgOutdoorTEMP.set(0));
      send(msgBypassPOS.set(0));
      send(msgExhaustFAN.set(0));
      send(msgSupplyFAN.set(0));
      send(msgSupplyFLOW.set(0));
      send(msgExhaustFLOW.set(0));
      send(msgBypassMODE.set(0));

      // Check communication with Radio chip
      if ((radio.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f) == 1)
      {
        // Read EEPROM address to check if there is an address known
        bool empty_eeprom = true;

        for (int i = 0; i < 6; i++)
        {
          radio.new_fan_state.address[i] = loadState(i);
          if (radio.new_fan_state.address[i] != 0xFF) empty_eeprom = false;
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
          saveState(i, radio.new_fan_state.address[i]);

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

      if (radio.current_fan_state.fan_speed != fan_speed_req)                          // Set new data or fan speed request?
      {
        if (tx_retry_cntr < TX_RETRY_CNT)
        {
          if (radio.tx_fanspeed(fan_speed_req))                    // Succes, blink led
          {
            led_flash(1);
            radio.current_fan_state.fan_speed = radio.new_fan_state.fan_speed;            // If ok, current fan speed should match requested one
          }
          tx_retry_cntr++;
        }
        else
          radio.current_fan_state.fan_speed = fan_speed_req;                            // Forget about this session, maybe more succes next time?
      }
      else if (radio.current_fan_state.bypass_mode != fan_bypass_mode_req)
      {
        if (tx_retry_cntr < TX_RETRY_CNT)
        {
          if (radio.tx_fan_bypass(fan_bypass_mode_req))                    // Succes, blink led
          {
            led_flash(1);
            radio.current_fan_state.bypass_mode = radio.new_fan_state.bypass_mode;            // If ok, current fan speed should match requested one
          }
          tx_retry_cntr++;
        }
        else
          radio.current_fan_state.bypass_mode = fan_bypass_mode_req;            // If ok, current fan speed should match requested one
      }
      else
      {
        tx_retry_cntr = 0; // reset cntr

        // Request fan speed on regular interval FAN_INTERVAL
        req_current_millis = millis();
        if((req_current_millis-prev_req_current_millis) > FAN_INTERVAL)
        {
          prev_req_current_millis += FAN_INTERVAL;

          if(radio.request_fan_state())
          {
            led_flash(1);

            if(first_run_flag)                                  // Update once on first boot
            {
              radio.current_fan_state.fan_speed = radio.new_fan_state.fan_speed;
              fan_speed_req = radio.current_fan_state.fan_speed;                        // Prevent RF update next cycle
              
              if(radio.current_fan_state.fan_speed == 0)
                update_fan_state(0);
              else
                update_fan_state(1);
              
              update_fan_speed(radio.current_fan_state.fan_speed);                
              
              first_run_flag = false;              
            }
            else 
            {
              update_new_params(); // update changed params to controller
             
              // FAN SPEED CHANGE
              if(radio.current_fan_state.fan_speed != radio.new_fan_state.fan_speed)   // FAN SPEED CHANGED
              {
                if(radio.new_fan_state.fan_speed == 0)              // DETERMINE TO REPORT FAN STATE CHANGE OF SPEED CHANGE TO CONTROLLER
                {
                  prev_fan_speed = radio.current_fan_state.fan_speed;
                  radio.current_fan_state.fan_speed = radio.new_fan_state.fan_speed;
                  fan_speed_req = radio.current_fan_state.fan_speed;                      // Prevent RF update next cycle              
                  update_fan_state(0);
                }
                else
                {
                  if(radio.current_fan_state.fan_speed == 0)                              // Fan is off, new value != 0, therefore: send once V_STATUS = TRUE
                    update_fan_state(1);                
    
                  radio.current_fan_state.fan_speed = radio.new_fan_state.fan_speed;
                  fan_speed_req = radio.current_fan_state.fan_speed;                      // Prevent RF update next cycle
                  update_fan_speed(radio.current_fan_state.fan_speed);
                }
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

  if(message.getSensor() == CHILD_ID_BYPASS_MODE)  // BYPASS MODE
  {
    if (message.getType() == V_VAR1)
    {
      fan_bypass_mode_req = constrain(message.getInt(), 0, 2);      
      update_bypass_mode(fan_bypass_mode_req);
    }
  }

  if(message.getSensor() == CHILD_ID_FAN)  // FAN
  {
    if(message.getType() == V_STATUS)      // ON, OFF SWITCH
    {
      if(!message.getBool())               // FAN OFF
      {
        prev_fan_speed = radio.current_fan_state.fan_speed;        // STORE FAN SPEED (FOR ONLY RECEIVING "V_STATUS = TRUE" NEXT TIME)
        fan_speed_req = 0;                 // SEND "0" TO FAN
        update_fan_state(0);               // REPORT STATE BACK TO CONTROLLER        
      }
      else
      {
        fan_speed_req = prev_fan_speed;    // SET BACK FAN SPEED TO FAN ONLY
        update_fan_state(1);               // REPORT BACK TO CONTROLLER
      }   
    }

    if (message.getType() == V_PERCENTAGE)
    {
      fan_speed_req = constrain(message.getInt(), 0, 4);     
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
  uint32_t source_address = ((uint32_t)radio.new_fan_state.address[3]<<16) | ((uint32_t)radio.new_fan_state.address[4]<<8) | ((uint32_t)radio.new_fan_state.address[5]<<0);
  uint8_t device_id = ((source_address & 0xFC0000)>>18);
  uint32_t address_id = source_address & 0x03FFFF;  
  String result_string = String(device_id) + ":" + String(address_id);
  
  send(msgSourceAddress.set(result_string.c_str()));
}

void sendNewTargetAddressToGateway()
{
  // Ramses II format
  uint32_t target_address = ((uint32_t)radio.new_fan_state.address[0]<<16) | ((uint32_t)radio.new_fan_state.address[1]<<8) | ((uint32_t)radio.new_fan_state.address[2]<<0);
  uint8_t device_id = ((target_address & 0xFC0000)>>18);
  uint32_t address_id = target_address & 0x03FFFF; 
  String result_string = String(device_id) + ":" + String(address_id);
    
  send(msgTargetAddress.set(result_string.c_str()));
}

void update_bypass_mode(int16_t bypass_mode)
{
  send(msgBypassMODE.set(bypass_mode));
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
