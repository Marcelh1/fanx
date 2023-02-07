#include "cc1101.h"

// Select (SPI) CC1101
#define cc1101_Select() digitalWrite(chipSelectPin, LOW)
// Deselect (SPI) CC1101
#define cc1101_Deselect() digitalWrite(chipSelectPin, HIGH)
// Wait until SPI MISO line goes low
#define wait_Miso() while (digitalRead(MISO) > 0)

CC1101::CC1101(void) {}

void CC1101::writeReg(byte regAddr, byte value)
{
  cc1101_Select();	// Select CC1101
  wait_Miso();	// Wait until MISO goes low
  SPI.transfer(regAddr);	// Send register address
  SPI.transfer(value);	// Send value
  cc1101_Deselect();	// Deselect CC1101
}

void CC1101::writeBurstReg(byte regAddr, byte *buffer, byte len)
{
  byte addr, i;

  addr = regAddr | WRITE_BURST;	// Enable burst transfer
  cc1101_Select();	// Select CC1101
  wait_Miso();	// Wait until MISO goes low
  SPI.transfer(addr);	// Send register address

  for (i = 0; i < len; i++)
    SPI.transfer(buffer[i]);	// Send value

  cc1101_Deselect();	// Deselect CC1101
}

void CC1101::cmdStrobe(byte cmd)
{
  cc1101_Select();	// Select CC1101
  wait_Miso();	// Wait until MISO goes low
  SPI.transfer(cmd);	// Send strobe command
  cc1101_Deselect();	// Deselect CC1101
}

byte CC1101::readReg(byte regAddr, byte regType)
{
  byte addr, val;

  addr = regAddr | regType;
  cc1101_Select();	// Select CC1101
  wait_Miso();	// Wait until MISO goes low
  SPI.transfer(addr);	// Send register address
  val = SPI.transfer(0x00);	// Read result
  cc1101_Deselect();	// Deselect CC1101

  return val;
}

void CC1101::readBurstReg(byte *buffer, byte regAddr, byte len)
{
  byte addr, i;

  addr = regAddr | READ_BURST;
  cc1101_Select();	// Select CC1101
  wait_Miso();	// Wait until MISO goes low
  SPI.transfer(addr);	// Send register address
  for (i = 0; i < len; i++)
    buffer[i] = SPI.transfer(0x00);	// Read result byte by byte
  cc1101_Deselect();	// Deselect CC1101
}

void CC1101::reset(void)
{
  cc1101_Deselect();	// Deselect CC1101
  delayMicroseconds(5);
  cc1101_Select();	// Select CC1101
  delayMicroseconds(10);
  cc1101_Deselect();	// Deselect CC1101
  delayMicroseconds(41);
  cc1101_Select();	// Select CC1101

  wait_Miso();	// Wait until MISO goes low
  SPI.transfer(CC1101_SRES);	// Send reset command strobe
  wait_Miso();	// Wait until MISO goes low

  cc1101_Deselect();	// Deselect CC1101
}

void CC1101::config_registers(void)
{
  reset();

  // ########## BEGIN FAN RF15 chip config ##########
  setCarrierFreq(CFREQ_868);
  writeReg(CC1101_IOCFG0, CC1101_DEFVAL_IOCFG0);	// High impdance 3-state
  writeReg(CC1101_IOCFG2, CC1101_DEFVAL_IOCFG2);	// Lock detector
  writeReg(CC1101_FSCTRL1, CC1101_DEFVAL_FSCTRL1);
  writeReg(CC1101_FSCTRL0, CC1101_DEFVAL_FSCTRL0);
  writeReg(CC1101_MDMCFG4, CC1101_DEFVAL_MDMCFG4);
  writeReg(CC1101_MDMCFG3, CC1101_DEFVAL_MDMCFG3);
  writeReg(CC1101_MDMCFG2, CC1101_DEFVAL_MDMCFG2);
  writeReg(CC1101_MDMCFG1, CC1101_DEFVAL_MDMCFG1);
  writeReg(CC1101_MDMCFG0, CC1101_DEFVAL_MDMCFG0);
  writeReg(CC1101_CHANNR, CC1101_DEFVAL_CHANNR);
  writeReg(CC1101_DEVIATN, CC1101_DEFVAL_DEVIATN);
  writeReg(CC1101_FREND1, CC1101_DEFVAL_FREND1);
  writeReg(CC1101_FREND0, CC1101_DEFVAL_FREND0);
  writeReg(CC1101_MCSM0, CC1101_DEFVAL_MCSM0);
  writeReg(CC1101_FOCCFG, CC1101_DEFVAL_FOCCFG);
  writeReg(CC1101_BSCFG, CC1101_DEFVAL_BSCFG);
  writeReg(CC1101_AGCCTRL2, CC1101_DEFVAL_AGCCTRL2);
  writeReg(CC1101_AGCCTRL1, CC1101_DEFVAL_AGCCTRL1);
  writeReg(CC1101_AGCCTRL0, CC1101_DEFVAL_AGCCTRL0);
  writeReg(CC1101_WORCTRL, CC1101_DEFVAL_WORCTRL);
  writeReg(CC1101_FSCAL3, CC1101_DEFVAL_FSCAL3);
  writeReg(CC1101_FSCAL2, CC1101_DEFVAL_FSCAL2);
  writeReg(CC1101_FSCAL1, CC1101_DEFVAL_FSCAL1);
  writeReg(CC1101_FSCAL0, CC1101_DEFVAL_FSCAL0);
  writeReg(CC1101_TEST0, CC1101_DEFVAL_TEST0);
  writeReg(CC1101_PKTCTRL1, CC1101_DEFVAL_PKTCTRL1);
  writeReg(CC1101_PKTCTRL0, CC1101_DEFVAL_PKTCTRL0);

  uint8_t tx_buf[8] = { 0x6F, 0x26, 0x2E, 0x7F, 0x8A, 0x84, 0xCA, 0xC4
                      };
  writeBurstReg(CC1101_UNKNOWNFIFO, tx_buf, 8);
  cmdStrobe(CC1101_SIDLE);
  cmdStrobe(CC1101_SIDLE);

}

void CC1101::init(void)
{
  pinMode(chipSelectPin, OUTPUT);	// Make sure that the SS Pin is declared as an Output
  SPI.begin();	// Initialize SPI interface
  pinMode(CC1101_GDO2, INPUT);	// Config GDO0 as input
  reset();	// Reset CC1101

  // Back to idle/power down
  setPowerDownState();

  // Constants
  msg_id[indoor_hum].code_id = 0x12A0;
  msg_id[fan_speed].code_id = 0x22F1;
  msg_id[fan_info].code_id = 0x31DA;

}

uint8_t CC1101::calc_crc(uint8_t dataframe[], uint8_t len)
{
  int crc_calc = 0;

  for (uint8_t i = 0; i < len - 1; i++)
    crc_calc += dataframe[i];

  while (crc_calc > 256)
    crc_calc -= 256;
  crc_calc = 256 - crc_calc;

  return crc_calc;
}

bool CC1101::clone_mode(void)
{
  const uint8_t buff_lenght = 75;
  uint8_t rx_buffer[buff_lenght];
  uint8_t rbuf[buff_lenght];
  int rlen = 0;  
  bool rf15_frame_valid = false;
  unsigned long previousMillis = 0;
  unsigned long currentMillis = 0;
  bool rx_abort_flag = false;
  bool header_detected_flag = false;

  Serial1.setTimeout(RX_TIME_OUT);

  // Clear input buffer
  uint8_t temp_var;
  while (Serial1.available())
    temp_var = Serial1.read();

  previousMillis = millis();

  // Process RX data
  while (!rx_abort_flag) // RX complete or timeout
  {
    if (!header_detected_flag)
    {
      while (Serial1.available() > 0)
      {
        for (uint8_t i = 0; i < 5; i++)
          rx_buffer[i] = rx_buffer[i + 1];
        rx_buffer[5] = Serial1.read();

        if ( (rx_buffer[5] == 0x53) && (rx_buffer[4] == 0x55) && (rx_buffer[3] == 0x33) && (rx_buffer[2] == 0x00) && (rx_buffer[0] == 0x55))
        {
          header_detected_flag = true;
          break;
        }
        else
        {
          currentMillis = millis();
          if (currentMillis - previousMillis > PAIR_TIME_OUT)
          {
            rx_abort_flag = true;
            break;
          }
        }
      }
    }
    else
    {
      rlen = Serial1.readBytesUntil(0x35, rbuf, sizeof(rbuf) - 1); // read until 0x35 received or timeout
      rx_abort_flag = true;
    }
  }

  // Process data
  if ( (rlen == 0) || (rlen == 100)) // No valid data
  {
	#ifdef DEBUG_MODE
		if (!header_detected_flag)
		  Serial.println("> Error: no header detected!");
		else
		  Serial.println("> header detected, but no valid data!");
	#endif

  }
  else
  {
	  // check if encoded number of bytes is even number
    if ((rlen % 2) != 0)
	{
	  #ifdef DEBUG_MODE
        Serial.println("> Error: not even number, problaby corrupted msg, problematic for decoding");
	  #endif
    }
	else
    {
      uint8_t dataframe_decoded[rlen / 2];
      manchester_decode(rbuf, rlen, dataframe_decoded);

      #ifdef DEBUG_MODE
		  Serial.print("> RX decoded data used for cloning: ");
		  for (int i = 0; i < rlen / 2; i++)
		  {
			Serial.print(dataframe_decoded[i], HEX);
			Serial.print(" ");
		  }
		  Serial.print(" | ");
	  #endif
	  
      // CRC check
      if (calc_crc(dataframe_decoded, rlen / 2) == dataframe_decoded[(rlen / 2) - 1])
      {
		  // Clone and store RF15 address in EEPROM
		  for (uint8_t i = 1; i < 7; i++)
			new_fan_state.address[i - 1] = dataframe_decoded[i];		  
		rf15_frame_valid = true;
	  }
	}
	
  }
  
  return rf15_frame_valid;
  
}

uint8_t *CC1101::manchester_decode(uint8_t rx_buff[], uint8_t len, uint8_t *rx_payload)
{
  uint8_t payload_cntr = 0;
  const uint8_t mch_lookup[16] = {0xAA, 0xA9, 0xA6, 0xA5, 0x9A, 0x99, 0x96, 0x95, 0x6A, 0x69, 0x66, 0x65, 0x5A, 0x59, 0x56, 0x55};

  for (uint8_t i = 0; i < len; i++)
  {
    for (uint8_t x = 0; x < 16; x++)
    {
      if (rx_buff[i + 1] == mch_lookup[x])
        rx_payload[payload_cntr] = x;
    }
    for (uint8_t x = 0; x < 16; x++)
    {
      if (rx_buff[i] == mch_lookup[x])
        rx_payload[payload_cntr] |= x << 4;
    }
    payload_cntr++;
    i++;
  }

  return rx_payload;
}

uint8_t *CC1101::manchester_encode(uint8_t tx_buff[], uint8_t len, uint8_t *payload_encoded)
{
  const uint8_t mch_lookup[16] = {0xAA, 0xA9, 0xA6, 0xA5, 0x9A, 0x99, 0x96, 0x95, 0x6A, 0x69, 0x66, 0x65, 0x5A, 0x59, 0x56, 0x55};

  // Manchester encode
  uint8_t tx_cntr = 0;
  uint8_t temp_var = 0;

  for (int i = 0; i < len; i++)
  {
    temp_var = tx_buff[i] & B11110000;
    payload_encoded[tx_cntr] = mch_lookup[temp_var >> 4];
    tx_cntr++;
    temp_var = tx_buff[i] & B00001111;
    payload_encoded[tx_cntr] = mch_lookup[temp_var];
    tx_cntr++;
  }

  return payload_encoded;
}

bool CC1101::transmit_data(uint8_t payload[], uint8_t len)
{
  bool fan_frame_valid = false;
  const uint8_t buff_lenght = 100;
  uint8_t rx_buffer[6];
  uint8_t rbuf[buff_lenght];
  int rlen = 0;
  bool header_detected_flag = false;
  uint8_t tx_frame_lenght = (len * 2) + 15;	// 15 bytes overhead, len*2 data
  uint8_t tx_buffer[tx_frame_lenght];
  uint8_t tx_payload_encoded[len * 2];

  unsigned long previousMillis = 0;
  unsigned long currentMillis = 0;
  bool rx_abort_flag = false;

  // Manchester encode payload
  manchester_encode(payload, len, tx_payload_encoded);

  //PREAMBLE
  for (int i = 0; i < 9; i++)
    tx_buffer[i] = 0x55;
  // BOF
  tx_buffer[9] = 0xFF;
  tx_buffer[10] = 0x00;
  tx_buffer[11] = 0x33;
  tx_buffer[12] = 0x55;
  tx_buffer[13] = 0x53;

  // Insert payload (manchester encoded)
  for (int i = 0; i < tx_frame_lenght - 1; i++)
    tx_buffer[i + 14] = tx_payload_encoded[i];

  // EOF
  tx_buffer[tx_frame_lenght - 1] = 0x35;

  Serial1.setTimeout(RX_TIME_OUT);

  config_registers();	// Reconfigure CC1101

  setTxState();
  Serial1.write(tx_buffer, tx_frame_lenght);
  Serial1.flush();	// wait for Serial TX data completed

  // Clear input buffer, might be filled with TX data, since RX and TX pin are connected
  uint8_t temp_var;
  while (Serial1.available())
    temp_var = Serial1.read();

  // Config GDO0 to ouput
  cmdStrobe(CC1101_SIDLE);
  setRxState();

  // Read serial data, variable frame lenght
  previousMillis = millis();

  // Process RX data
  while (!rx_abort_flag) // RX complete or timeout
  {
    if (!header_detected_flag)
    {
      while (Serial1.available() > 0)
      {
        for (uint8_t i = 0; i < 5; i++)
          rx_buffer[i] = rx_buffer[i + 1];
        rx_buffer[5] = Serial1.read();

        if ( (rx_buffer[5] == 0x53) && (rx_buffer[4] == 0x55) && (rx_buffer[3] == 0x33) && (rx_buffer[2] == 0x00) && (rx_buffer[0] == 0x55))
        {
          header_detected_flag = true;
          break;
        }
        else
        {
          currentMillis = millis();
          if (currentMillis - previousMillis > RX_TIME_OUT)
          {
            rx_abort_flag = true;
            break;
          }
        }
      }
    }
    else
    {
      rlen = Serial1.readBytesUntil(0x35, rbuf, sizeof(rbuf) - 1); // read until 0x35 received or timeout
      rx_abort_flag = true;
    }
  }

  // Process data
  if ( (rlen == 0) || (rlen == 100)) // No valid data
  {
	#ifdef DEBUG_MODE
	if (!header_detected_flag)
	  Serial.println("> Error: no header detected!");
	else
	  Serial.println("> header detected, but no valid data!");
	#endif
  }
  else
  {
    // check if encoded number of bytes is even number
    if ((rlen % 2) != 0)
	{
      #ifdef DEBUG_MODE
	  Serial.println("> Error: not even number, problaby corrupted msg, problematic for decoding");
	  #endif
    }
	else
    {
      uint8_t dataframe_decoded[rlen / 2];
      manchester_decode(rbuf, rlen, dataframe_decoded);

      #ifdef DEBUG_MODE
	  Serial.print("> RX decoded data: ");
	  for (int i = 0; i < rlen / 2; i++)
	  {
		Serial.print(dataframe_decoded[i], HEX);
		Serial.print(" ");
	  }
	  Serial.print(" | ");
	  #endif
	  
      // CRC check
      if (calc_crc(dataframe_decoded, rlen / 2) == dataframe_decoded[(rlen / 2) - 1])
      {
        // Check address!
        fan_frame_valid = true;
        for (uint8_t i = 1; i < 4; i++)
        {
          if (dataframe_decoded[i] != new_fan_state.address[i + 2])
            fan_frame_valid = false;
        }

        if (fan_frame_valid)	// Update states
        {
		  uint16_t param_type = ((uint16_t)dataframe_decoded[7]<<8) | (uint16_t)(dataframe_decoded[8]);
		  uint8_t param_lenght = dataframe_decoded[9];
			
          if ( (param_type == 0x22F1) && (param_lenght == 0x03) )	// FAN SPEED
          {
            new_fan_state.fan_speed = dataframe_decoded[11];
            msg_id[fan_speed].rx_flag = true;
          }

          if (param_type == 0x31D9)	// FAN SPEED, don't check for lenght, might differ from MVS and HRC
          {
            new_fan_state.fan_speed = dataframe_decoded[12];
            msg_id[fan_speed].rx_flag = true;
          }		
		  if ( (param_type == 0x12A0) && (param_lenght == 0x02) )	// Humidity
          {
            new_fan_state.indoor_humidity = dataframe_decoded[11];
            msg_id[indoor_hum].rx_flag = true;
          }
		  if ( (param_type == 0x31DA) && (param_lenght == 0x1E) )	// EXTENDED 31DA INFO
          {
            new_fan_state.indoor_humidity = dataframe_decoded[15];
			new_fan_state.outdoor_humidity = dataframe_decoded[16];			
			new_fan_state.exhaust_temperature = (dataframe_decoded[17]*256)+dataframe_decoded[18];			
			new_fan_state.supply_temperature = (dataframe_decoded[19]*256)+dataframe_decoded[20];
			new_fan_state.indoor_temperature = (dataframe_decoded[21]*256)+dataframe_decoded[22];			
			new_fan_state.outdoor_temperature = (dataframe_decoded[23]*256)+dataframe_decoded[24];			
			new_fan_state.bypass_position = dataframe_decoded[27];			
			new_fan_state.exhaust_fanspeed = dataframe_decoded[29];			
			new_fan_state.supply_fanspeed = dataframe_decoded[30];			
			new_fan_state.supply_flow = (dataframe_decoded[35]*256)+dataframe_decoded[36];
			new_fan_state.exhaust_flow = (dataframe_decoded[37]*256)+dataframe_decoded[38];
			
            msg_id[fan_info].rx_flag = true;
          }

          #ifdef DEBUG_MODE
			Serial.println("Data ok!");
		  #endif
        }
        else
		{
          #ifdef DEBUG_MODE
   		    Serial.println("Valid message, wrong address!");
		  #endif
			
		}
      }
      else
	  {
	    #ifdef DEBUG_MODE
          Serial.println("CRC ERROR!");
		#endif
	  }
    }
  }

  // Back to idle/power down
  cmdStrobe(CC1101_SIDLE);
  writeReg(CC1101_IOCFG0, CC1101_DEFVAL_IOCFG0);
  setPowerDownState();

  return fan_frame_valid;
}

bool CC1101::tx_fanspeed(uint8_t fan_speed)
{
  uint8_t payload[14];
  uint8_t ARR_SIZE = sizeof(payload) / sizeof(payload[0]);

  // header[RQ = 0x0C, W = 0x1C, I = 0x2C, RP = 3C]
  payload[0] = 0x1C;

  // Get souce and target address
  for (uint8_t i = 1; i < 7; i++)
    payload[i] = new_fan_state.address[i - 1];

  // Opcode[FAN speed status]
  payload[7] = 0x22;
  payload[8] = 0xF1;

  // Command lenght
  payload[9] = 0x03;

  // Payload
  payload[10] = 0x00;
  payload[11] = fan_speed;
  payload[12] = 0x04;

  payload[ARR_SIZE - 1] = calc_crc(payload, ARR_SIZE);

  // Returns bool
  return transmit_data(payload, ARR_SIZE);

}
/*
bool CC1101::tx_fan_bypass(uint8_t fan_bypass)
{
  uint8_t payload[14];
  uint8_t ARR_SIZE = sizeof(payload) / sizeof(payload[0]);

  // header[RQ = 0x0C, W = 0x1C, I = 0x2C, RP = 3C]
  payload[0] = 0x1C;

  // Get souce and target address
  for (uint8_t i = 1; i < 7; i++)
    payload[i] = new_fan_state.address[i - 1];

  // Opcode[FAN speed status]
  payload[7] = 0x22;
  payload[8] = 0xF7;

  // Command lenght
  payload[9] = 0x03;

  // Payload
  payload[10] = 0x00;
  payload[11] = fan_bypass;
  payload[12] = 0xEF;

  payload[ARR_SIZE - 1] = calc_crc(payload, ARR_SIZE);

  // Returns bool
  return transmit_data(payload, ARR_SIZE);

}
*/

uint8_t CC1101::request_fan_state(void)
{
  // header[RQ = 0x0C, W = 0x1C, I = 0x2C, RP = 3C]
  uint8_t payload[12] = {0x0C, new_fan_state.address[0], new_fan_state.address[1], new_fan_state.address[2], new_fan_state.address[3], new_fan_state.address[4], new_fan_state.address[5], 0x00, 0x00, 0x01, 0x00, 0x00};
  uint8_t ARR_SIZE = sizeof(payload) / sizeof(payload[0]);
  static uint8_t tx_cntr = 0;
  static uint8_t req_cntr = 0;

  static uint8_t req_pointer[3] = {fan_speed, indoor_hum, fan_info}; // Init request array

  payload[7] = (msg_id[req_pointer[tx_cntr]].code_id >> 8);
  payload[8] = (msg_id[req_pointer[tx_cntr]].code_id & 0xFF);

  payload[ARR_SIZE - 1] = calc_crc(payload, ARR_SIZE);

  // Determine next tx msg
  if (req_cntr < TX_REQ_CNTS)
  {
    if (tx_cntr < sizeof(enum codes_enum))
      tx_cntr++;
    else
      tx_cntr = 0;

    req_cntr++;
  }
  else
  {
	req_pointer[0] = fan_speed;	
	if(msg_id[fan_info].rx_flag)
		req_pointer[1] = fan_info;
	else
	  req_pointer[1] = indoor_hum;
	  
	if (tx_cntr < sizeof(enum codes_enum)-1)
	  tx_cntr++;
	else
	  tx_cntr = 0;    
  }

  // Returns bool
  return transmit_data(payload, ARR_SIZE);

}

void CC1101::set_rx_mode(void)
{
  config_registers();	// Reconfigure CC1101
  setRxState();
}

void CC1101::setCarrierFreq(byte freq)
{
  switch (freq)
  {
    case CFREQ_915:
      writeReg(CC1101_FREQ2, CC1101_DEFVAL_FREQ2_915);
      writeReg(CC1101_FREQ1, CC1101_DEFVAL_FREQ1_915);
      writeReg(CC1101_FREQ0, CC1101_DEFVAL_FREQ0_915);
      break;
    case CFREQ_433:
      writeReg(CC1101_FREQ2, CC1101_DEFVAL_FREQ2_433);
      writeReg(CC1101_FREQ1, CC1101_DEFVAL_FREQ1_433);
      writeReg(CC1101_FREQ0, CC1101_DEFVAL_FREQ0_433);
      break;
    case CFREQ_918:
      writeReg(CC1101_FREQ2, CC1101_DEFVAL_FREQ2_918);
      writeReg(CC1101_FREQ1, CC1101_DEFVAL_FREQ1_918);
      writeReg(CC1101_FREQ0, CC1101_DEFVAL_FREQ0_918);
      break;
    default:
      writeReg(CC1101_FREQ2, CC1101_DEFVAL_FREQ2_868);
      writeReg(CC1101_FREQ1, CC1101_DEFVAL_FREQ1_868);
      writeReg(CC1101_FREQ0, CC1101_DEFVAL_FREQ0_868);
      break;
  }

  //carrierFreq = freq;
}

void CC1101::setPowerDownState()
{
  // Comming from RX state, we need to enter the IDLE state first
  cmdStrobe(CC1101_SIDLE);
  // Enter Power-down state
  cmdStrobe(CC1101_SPWD);
}

void CC1101::setRxState(void)
{
  writeReg(CC1101_MDMCFG2, CC1101_DEFVAL_MDMCFG2);
  writeReg(CC1101_IOCFG0, CC1101_SERIAL_OUT_IOCFG0);	// Serial data output
  cmdStrobe(CC1101_SRX);
  //rfState = RFSTATE_RX;
}

void CC1101::setTxState(void)
{
  writeReg(CC1101_IOCFG0, CC1101_DEFVAL_IOCFG0);
  writeReg(CC1101_MDMCFG2, CC1101_DEFVAL_MDMCFG2);
  cmdStrobe(CC1101_STX);
  //rfState = RFSTATE_TX;
}
