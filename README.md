# Open source FanX RF Dongle 

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

![Preview](https://github.com/Marcelh1/fanx/blob/main/images/Preview.png)

[Get one FanX RF Dongle here!](https://fan-x.eu/product/fan%cb%a3-rf-usb-dongle/)
