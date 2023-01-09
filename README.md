# Open source 868MHz FanX RF Dongle 

[Wireless USB Dongle](https://fan-x.eu/product/fan%cb%a3-rf-usb-dongle/) for home automation systems to improve the indoor air quality.

![usb dongle](https://github.com/Marcelh1/fanx/blob/main/images/high_q.png)

# HW versions (important!)
There are three versions of the dongle, V1.0 and "V2.0 chip antenna" and "V2.0 sma antenna". They use different bootloaders, same application software (Firmata or MySensors) can be used. It's however very important to select the right board in the Arduino IDE:

V1.0 = Arduino micro

V2.0 = Sparkfun micro pro

If the wrong target was selected during upload, the dongle will not be recognised by Windows anymore, no worries! You have to give a short reset pulse by short circuiting the reset pin with ground on the 6p connector just before Arduino is trying to upload the code. This will put the Dongle into bootloader mode for few seconds. Check the assigned com port first after giving the first reset pulse (or double pulse to have some more time). 


# General

The repository contains of these folders:
- [MySensors](https://github.com/Marcelh1/fanx/tree/main/mysensors) (preferred one)
- [Firmata](https://github.com/Marcelh1/fanx/tree/main/firmata)
- [Bootloaders](https://github.com/Marcelh1/fanx/tree/main/bootloaders)

![Preview](https://github.com/Marcelh1/fanx/blob/main/images/preview_animation.gif)

# Flash new software into the dongle

## Using the Arduino IDE (installed bootloader)
Plug in the USB dongle, make sure you have the Arduino IDE installed. Select the Arduino Micro target and the right com port. Then open the sketch and click “upload”.

*Please note that by flashing the dongle, the USB name (description) will be set back to the default (Arduino Micro) one. This can be changed by editing this file: “C:\Program Files (x86)\Arduino\hardware\arduino\avr\boards.txt”, then goto “micro.name=Arduino Micro” and change this param: micro.build.usb_product=”FanX RF Dongle”

## Using Microchip Studio
Via the onboard 6p header, the software can be flashed directly from Microchip studio. You can use the Atmel ICE flasher, or the Atmel ISP MKII flasher. Via Atmel Studio, also the Arduino (micro) bootloader can be flashed. No example projects are available.

[Get one FanX RF Dongle here!](https://fan-x.eu/product/fan%cb%a3-rf-usb-dongle/)
