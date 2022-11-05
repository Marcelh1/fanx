#include <Arduino.h>
#include "cc1101.h"
#define ONBOARD_LED                 7
#define LED_FLASH_TIME              50  // ms
#define ORCON_INTERVAL              5000  // RF link interval
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

// Enable serial gateway
#define MY_GATEWAY_SERIAL
#define MY_BAUD_RATE                38400

#include <MySensors.h>
#define CHILD_ID                    1

#define SN                          "FanX"
#define SV                          "1.0"

MyMessage msgState(CHILD_ID, V_LIGHT);
MyMessage msgDimmer(CHILD_ID, V_DIMMER);

volatile int fan_speed_req = 1;
volatile int fan_speed = 1;

module_states dongle_state = JUST_BOOTED;

//******************************************************************************************//
//                                                                                          //
//                        Init                                                              //
//                                                                                          //
//******************************************************************************************//

void setup()
{
  // Setup locally attached sensors
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, HIGH);
  
  Serial1.begin(38400); // used for transmitting data to Orcon  
  while(!Serial1);
}

void presentation()
{
  // Present locally attached sensors
  sendSketchInfo(SN, SV);
  present(CHILD_ID, S_DIMMER, "FanX");  
}

//******************************************************************************************//
//                                                                                          //
//                        Led flash                                                         //
//                                                                                          //
//******************************************************************************************//
void led_flash_once_ms(int blink_time)
{
  digitalWrite(ONBOARD_LED, LOW);  // RADIO LED ON
  wait(blink_time);
  digitalWrite(ONBOARD_LED, HIGH); // RADIO LED OFF
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
  static int temp_cntr = 1;
  
  switch (dongle_state)
  {
    case (JUST_BOOTED):
      
      // Inform MySensors once
      send(msgState.set(1));
      send(msgDimmer.set(1));

      radio.init();

      if ((radio.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) &0x1f) == 1)
        dongle_state = RF15_PAIRINGSMODE;
      else
        wait(1000);
     
      break;
      
    case (RF15_PAIRINGSMODE):
     
      digitalWrite(ONBOARD_LED, LOW);  // RADIO LED ON
      radio.set_rx_mode();
      radio.clone_mode();
      digitalWrite(ONBOARD_LED, HIGH); // RADIO LED OFF 

      dongle_state = NORMAL_MODE;

      break;

    case (NORMAL_MODE):
/*
        if (fan_speed != fan_speed_req)                          // Set new data or fan speed request?
        {
          
          if (tx_retry_cntr < TX_RETRY_CNT)
          {
            if (radio.tx_orcon(fan_speed_req))                    // Succes, blink led
            {
              led_flash_once_ms(LED_FLASH_TIME);
              fan_speed = radio.orcon_state.fan_speed;            // If ok, current fan speed should match requested one
            }
            tx_retry_cntr++;
          }
          else
            fan_speed = fan_speed_req;                            // Forget about this session, maybe more succes next time?
        }
        else
        {
        */
          tx_retry_cntr = 0; // reset cntr

          // Request fan speed on regular interval ORCON_INTERVAL
          req_current_millis = millis();
          if ( (req_current_millis - prev_req_current_millis) > ORCON_INTERVAL)
          {
            prev_req_current_millis += ORCON_INTERVAL;
           
            if (radio.request_orcon_state())
            {
              led_flash_once_ms(LED_FLASH_TIME);
            }

            // Debugging
            led_flash_once_ms(LED_FLASH_TIME);
            if(temp_cntr < 4)
              temp_cntr++;
            else
              temp_cntr = 1;          
            sendNewStateToGateway(temp_cntr);
            
          }
        //}
     
      break;
  }  
 
}

//******************************************************************************************//
//                                                                                          //
//                        Receive from MySensors                                            //
//                                                                                          //
//******************************************************************************************//

void receive(const MyMessage &message) 
{
  if (message.isAck()) 
  {
     //Serial.println("This is an ack from gateway");
     return;
  }

  if (message.getType() == V_LIGHT) 
  {
    
  }
  
  if (message.getType() == V_DIMMER)
  {
      fan_speed_req = constrain( message.getInt(), 0, 100 );     
      fan_speed_req = min(fan_speed_req, 4);      // Limit to 4
      fan_speed_req = max(fan_speed_req, 0);      // Not negative
      
      sendNewStateToGateway(fan_speed_req);
  }  
  
}

//******************************************************************************************//
//                                                                                          //
//                        Transmit new state                                                //
//                                                                                          //
//******************************************************************************************//

void sendNewStateToGateway(int speed_level) 
{   
  send(msgDimmer.set(speed_level));
}
