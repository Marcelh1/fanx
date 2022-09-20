/**
 *Copyright (c) 2011 panStamp<contact@panstamp.com>
 *Copyright (c) 2016 Tyler Sommer<contact@tylersommer.pro>
 * 
 *This file is part of the CC1101 project.
 * 
 *CC1101 is free software; you can redistribute it and/or modify
 *it under the terms of the GNU Lesser General Public License as published by
 *the Free Software Foundation; either version 3 of the License, or
 *any later version.
 * 
 *CC1101 is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *GNU Lesser General Public License for more details.
 * 
 *You should have received a copy of the GNU Lesser General Public License
 *along with CC1101; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 
 *USA
 * 
 *Author: Daniel Berenguer
 *Creation date: 03/03/2011
 */
#include "cc1101.h"

/**
 *Macros
 */
// Select (SPI) CC1101
#define cc1101_Select() digitalWrite(chipSelectPin, LOW)
// Deselect (SPI) CC1101
#define cc1101_Deselect() digitalWrite(chipSelectPin, HIGH)
// Wait until SPI MISO line goes low
#define wait_Miso() while (digitalRead(MISO) > 0)

	/**
	 *CC1101
	 * 
	 *Class constructor
	 */
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

	// ########## BEGIN ORCON RF15 chip config ########## 
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
	uint8_t buff_lenght = 100;
	uint8_t rx_buffer[buff_lenght];
	bool rf15_frame_valid = false;
	unsigned long previousMillis = 0;
	unsigned long currentMillis = 0;
	bool pair_timeout_flag = false;
	bool header_detected_flag = false;
	uint8_t rx_frame_lenght = 0;
	
	previousMillis = millis();

	while (!rf15_frame_valid && !pair_timeout_flag)	// wait till both frames received, or timeout
	{
		// Handle timeout
		currentMillis = millis();
		if (currentMillis - previousMillis > PAIR_TIME_OUT)
			pair_timeout_flag = true;
		else
		{			
			while ((Serial1.available() > 0) && (!rf15_frame_valid) && (!pair_timeout_flag)) // Exit loop when frame recognised or timeout
			{
			 	// Fifo buffer
				for (uint8_t i = 0; i < buff_lenght - 1; i++)
					rx_buffer[i] = rx_buffer[i + 1];
				rx_buffer[buff_lenght - 1] = Serial1.read();

				if (header_detected_flag)
				{
					rx_frame_lenght++;

					if (rx_buffer[buff_lenght - 1] == 0x35)	// FRAME COMPLETE!
					{
						uint8_t bof_frame = buff_lenght - rx_frame_lenght - 2;	// 100 - 27 - 2 => 71
						uint8_t dataframe_encoded[buff_lenght - 1 - bof_frame];	// 100 - 71 => 28
						uint8_t frame_decoded_lenght = ((buff_lenght - bof_frame) - 1) / 2;	// (100 - 71 - 1) / 2 => 14

						uint8_t x = 0;
						for (uint8_t i = bof_frame; i < buff_lenght - 1; i++)
						{
							dataframe_encoded[x] = rx_buffer[i];
							x++;
						}

						uint8_t dataframe_decoded[frame_decoded_lenght];
						manchester_decode(dataframe_encoded, rx_frame_lenght, dataframe_decoded);

						// CRC check
						if (calc_crc(dataframe_decoded, frame_decoded_lenght) == dataframe_decoded[frame_decoded_lenght - 1])
						{
							// Clone and store RF15 address in EEPROM
							for (uint8_t i = 1; i < 7; i++)
								EEPROM.update(i - 1, dataframe_decoded[i]);
							
							rf15_frame_valid = true;							
						}
						else
						{
							Serial.println("> CRC Error!");
						}
					}
				}
				else
				{
					// Detect header from remote control
					if ( (rx_buffer[99] == 0x5A) && (rx_buffer[98] == 0xA9) && (rx_buffer[97] == 0x53) && (rx_buffer[96] == 0x55) && (rx_buffer[95] == 0x33) && (rx_buffer[94] == 0x00))
						header_detected_flag = true;
				}
			}
		}
	}

	return rf15_frame_valid;

}

uint8_t *CC1101::manchester_decode(uint8_t rx_buff[], uint8_t len, uint8_t *rx_payload)
{
	uint8_t payload_cntr = 0;
	uint8_t mch_lookup[16] = { 0xAA, 0xA9, 0xA6, 0xA5, 0x9A, 0x99, 0x96, 0x95, 0x6A, 0x69, 0x66, 0x65, 0x5A, 0x59, 0x56, 0x55
	};

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
	uint8_t mch_lookup[16] = { 0xAA, 0xA9, 0xA6, 0xA5, 0x9A, 0x99, 0x96, 0x95, 0x6A, 0x69, 0x66, 0x65, 0x5A, 0x59, 0x56, 0x55
	};
	uint8_t encode_frame_size = (len *2) + 15;	// Example 14 * 2 + 15 = 43, array from 0 - 42

	// Manchester encode	
	uint8_t tx_cntr = 0;
	uint8_t temp_var = 0;

	for (int i = 0; i < len; i++)
	{
		temp_var = tx_buff[i] &B11110000;
		payload_encoded[tx_cntr] = mch_lookup[temp_var >> 4];
		tx_cntr++;
		temp_var = tx_buff[i] &B00001111;
		payload_encoded[tx_cntr] = mch_lookup[temp_var];
		tx_cntr++;
	}

	return payload_encoded;
}

bool CC1101::transmit_data(uint8_t payload[], uint8_t len)
{
	bool orcon_frame_valid = false;
	uint8_t buff_lenght = 100;
	uint8_t rx_buffer[buff_lenght];
	uint8_t tx_buffer[buff_lenght];
	bool header_detected_flag = false;
	uint8_t tx_frame_lenght = (len *2) + 15;	// 15 bytes overhead, len*2 data
	uint8_t rx_frame_lenght = 0;
	uint8_t tx_payload_encoded[len *2];

	unsigned long previousMillis = 0;
	unsigned long currentMillis = 0;
	bool rx_timeout_flag = false;

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

	config_registers();	// Reconfigure CC1101
	setTxState();

	Serial1.write(tx_buffer, tx_frame_lenght);
	Serial1.flush();	// wait for Serial TX data completed

	// Config GDO0 to ouput
	cmdStrobe(CC1101_SIDLE);
	setRxState();

	// Read serial data, variable frame lenght
	previousMillis = millis();

	while (!orcon_frame_valid && !rx_timeout_flag)	// wait till both frames received, or timeout
	{
		// Handle timeout
		currentMillis = millis();
		if (currentMillis - previousMillis > RX_TIME_OUT)
			rx_timeout_flag = true;
		else
		{
			while ((Serial1.available() > 0) && (!orcon_frame_valid) && (!rx_timeout_flag)) // Exit loop when frame recognised or timeout
			{
			 	// Fifo buffer
				for (uint8_t i = 0; i < buff_lenght - 1; i++)
					rx_buffer[i] = rx_buffer[i + 1];
				rx_buffer[buff_lenght - 1] = Serial1.read();

				if (header_detected_flag)
				{
					rx_frame_lenght++;

					if (rx_buffer[buff_lenght - 1] == 0x35)	// FRAME COMPLETE!
					{
						uint8_t bof_frame = buff_lenght - rx_frame_lenght - 2;	// 100 - 27 - 2 => 71
						uint8_t dataframe_encoded[buff_lenght - 1 - bof_frame];	// 100 - 71 => 28
						uint8_t frame_decoded_lenght = ((buff_lenght - bof_frame) - 1) / 2;	// (100 - 71 - 1) / 2 => 14

						uint8_t x = 0;
						for (uint8_t i = bof_frame; i < buff_lenght - 1; i++)
						{
							dataframe_encoded[x] = rx_buffer[i];
							x++;
						}

						uint8_t dataframe_decoded[frame_decoded_lenght];
						manchester_decode(dataframe_encoded, rx_frame_lenght, dataframe_decoded);

						// CRC check
						if (calc_crc(dataframe_decoded, frame_decoded_lenght) == dataframe_decoded[frame_decoded_lenght - 1])
						{
						 				// Check address!
							orcon_frame_valid = true;
							for (uint8_t i = 4; i < 7; i++)
							{
								if (dataframe_decoded[i] != EEPROM.read(i - 1))
									orcon_frame_valid = false;
							}

							if (orcon_frame_valid)	// Update states
							{
                // Scan for 31D9 message + 5 positions = fan speed
                // This enabled support for HRC400 as well
                for (uint8_t i = 0; i < frame_decoded_lenght - 1; i++)
                {
                  if( (dataframe_decoded[i] == 0x31) && (dataframe_decoded[i+1] == 0xD9) ) // Position found!
                    orcon_state.fan_speed = dataframe_decoded[i+5];                 
                }                
							}
							else
							{
								Serial.println("> Dataframe error!");
							}
						}
						else
						{
							Serial.println("> CRC Error!");
						}
					}
				}
				else
				{
          // Accept 0x6A (MVS15) and 0x66 (HRC400)
					if ( ((rx_buffer[99] == 0x6A) || (rx_buffer[99] == 0x66)) && (rx_buffer[98] == 0xA9) && (rx_buffer[97] == 0x53) && (rx_buffer[96] == 0x55) && (rx_buffer[95] == 0x33) && (rx_buffer[94] == 0x00))
						header_detected_flag = true;
				}
			}
		}
	}

	// Back to idle/power down
	cmdStrobe(CC1101_SIDLE);
	writeReg(CC1101_IOCFG0, CC1101_DEFVAL_IOCFG0);
	setPowerDownState();

	return orcon_frame_valid;
}

bool CC1101::tx_orcon(uint8_t fan_speed)
{
	uint8_t payload[14];
	uint8_t ARR_SIZE = sizeof(payload) / sizeof(payload[0]);

	// header[RQ, W, I, RP]
	payload[0] = 0x1C;

	// Get souce and target address
	for (uint8_t i = 1; i < 7; i++)
		payload[i] = EEPROM.read(i - 1);

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

uint8_t CC1101::request_orcon_state(void)
{

	uint8_t payload[19];
	uint8_t ARR_SIZE = sizeof(payload) / sizeof(payload[0]);

	// header[RQ, W, I, RP]
	payload[0] = 0x1C;

	// Get souce and target address
	for (uint8_t i = 1; i < 7; i++)
		payload[i] = EEPROM.read(i - 1);

	// Opcode[FAN speed status]
	payload[7] = 0x31;
	payload[8] = 0xE0;

	// Command lenght
	payload[9] = 0x08;

	// Payload
	payload[10] = 0x00;
	payload[11] = 0x00;
	payload[12] = 0x00;
	payload[13] = 0x00;
	payload[14] = 0x01;
	payload[15] = 0x00;
	payload[16] = 0x64;
	payload[17] = 0x00;

	payload[ARR_SIZE - 1] = calc_crc(payload, ARR_SIZE);

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
