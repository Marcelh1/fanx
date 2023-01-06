# Clone the stock remote
If you plug in the USB to anything that is able to power the dongle, the dongle will stay in clone mode for 5 seconds, if you press button 1, 2 or 3 of your stock remote within this timeframe, it will be cloned by the dongle and it should be able to control the FAN.
The onboard led will lit during “clone mode” and will turn off after the 5sec or immediately when successfully cloned.

# Testing the Dongle in Windows
To check whether the Dongle works ok, and if you are able to clone your stock remote, connect the Dongle to the USB port of your computer and open the App: “Windows Remote Arduino Experience”. Click on the “FanX RF Dongle” (USB Connection) and click connect. Now, open the tab “Analog and enable “Pin A0″, change the FAN speed with your stock remote and check if the value changes, it can take a few seconds before it updates the value.”

# Setting up Home Assistant

## Install the custom integration
Before changing anything in Home Assistant, it is always a good idea to make a backup. Then you should add the script to the yaml configuration:
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

After this modification, you should reboot Home Assistant and then navigate to “Settings”, “Integrations”. Here you should see the new FanX Firmata integration. If you click on the Dongle, you should be able to see the FAN state if the remote was cloned already.

## Implement the Frontface
To implement a button control on your frontface, add the entity card:
```
type: entities
entities:
  - entity: sensor.fan_speed
    type: custom:custom-fan-card
    tap_action:
      action: none
title: Mechanische ventilatie
```

Then, you should put the files in the “www” folder.
Next, you have to register the JavaScript at: “Settings”, “Dashboards”, then click on the “right top three dots”, hit: “Sources”. You should add a source. To overwrite the icon shown on the frontpage, you can add the rule shown on the right in configurations.yaml
```
customize:
  sensor.fan_speed:
    entity_picture: /local/icons/fan_blue.png
```
