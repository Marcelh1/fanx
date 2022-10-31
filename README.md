# Open source FanX RF Dongle 

[Wireless USB Dongle](https://fan-x.eu/product/fan%cb%a3-rf-usb-dongle/) for home automation systems to improve the indoor air quality.

![usb dongle](https://github.com/Marcelh1/fanx/blob/main/high_q.png)

# HW versions (important!)
There are two versions of the dongle, V1.0 and V2.0. They use different bootloaders, same application software (Firmata or Standalone) can be used. It's however very important to select the right board in the Arduino IDE:

V1.0 = Arduino micro

V2.0 = Sparkfun micro pro

If the wrong target was selected during upload, the dongle will not be recognised by Windows anymore, you have to give a short reset pulse by short circuiting the reset pin with ground on the 6p connector just before Arduino is trying to upload the code. This will put the Dongle into bootloader mode for few seconds. Check the assigned com port first after giving the first reset pulse (or double pulse to have some more time). 


# General

The repository contains three folders:
- Standalone
- Firmata
- MySensors
- Bootloaders

# Standalone version
The "standalone" is the most basic version to communicate with the ventilation unit, after powering the USB dongle, it will stay in "clone mode" for 5sec, when you press a button on the RF15 remote control, the addresses will be cloned. If no button pressed within 5sec, it will use previous stored address in eeprom. The addresses will be stored in eeprom so that it is not forgotten during power off. Open a terminal to see the debug messages, use settings: **9600bps,none,8,1**. If the FAN speed is changed by the stock remote, it will report that FAN speed has been changed. Use the function: **radio.tx_orcon(uint8_t fan_speed)** to transmit a new FAN speed.

# Firmata version
The "Firmata" version is able to communicate with home automation systems like "Home Assistant". You have to configure it like this:
```
firmata:
  - serial_port: /dev/serial/by-id/usb-Arduino_LLC_FanX_RF_Dongle-if00
    serial_baud_rate: 57600
    lights:
      - name: fan speed
        pin_mode: PWM
        pin: 3
        minimum: 0
        maximum: 99
    sensors:
      - name: fan speed
        pin_mode: ANALOG
        pin: A0
        differential: 1
```

Also the Firmata version will support "clone mode" for the frst 5sec. To be able to write and read the FAN speed from Home Assistant, a "sensor" and "light" entity is used. At the moment this is the only method to communicate an analog value between Home Assistant and the Arduino board. Normally the entity: "lights" in Home Assistant is being controlled by sending percentage based on a range from 0 till 255, therefore setting the min/max to 0-99, will result in percentage equals decimal number, what is more convenient in automations.

You can add custom card to the Lovelace front panel like this:

![custom card](https://tweakers.net/i/92_YFrTlCgnYt5MYhOnoPeuxj60=/full-fit-in/4000x4000/filters:no_upscale():fill(white):strip_exif()/f/image/rBX8e7hh46UIrR00872p1JKJ.png?f=user_large)

# MySensors version
The "MySensors" version is able to communicate with home automation systems like "Home Assistant" and "Domoticz". You have to configure it like this:

- MySensors version: 2.3.2
- Serial connection
- Baudrate: 38400 bps

In the Arduino library manager, install the MySensors lib before uploading the Sketch.

[Get one FanX RF Dongle here!](https://fan-x.eu/product/fan%cb%a3-rf-usb-dongle/)
