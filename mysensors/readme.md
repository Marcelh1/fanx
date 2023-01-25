# MySensors configuration in Home Assistant

First, plug in the Dongle.

Install MySensors and use these settings:
- Check in HA hardware settings which comport was assigned to the dongle
- MySensors version: 2.3.2
- Serial connection
- Baudrate: 38400 bps

The Dongle will create 4 entities in Home Assistant, (if not, just plug out/in the dongle once):

![entities](https://github.com/Marcelh1/fanx/blob/main/images/ha_entities_ramses.png)

### 1. Clone Switch (Control)
Activate this switch to put the Dongle into clone mode, then press a random key on the RF15 remote to clone the addresses. After 5s timeout or succesful clone, the switch will be de-activated automatically, if the clone was succesful, the source and target address will be updated as well. If it's a new Dongle and not cloned yet, the onboard led will double blink.
### 2. FAN speed (Control)
This entity behaves as light control, the state can be read and written. Use the yaml code to convert it to FAN entity.
### 3. Source Address (Sensor)
The address of the RF15 remote control. According [Ramses II format](https://github.com/zxdavb/ramses_protocol/wiki/Decoding-Data-Fields#device-ids).
### 4. Target Address (Sensor)
The address of the Orcon unit. According [Ramses II format](https://github.com/zxdavb/ramses_protocol/wiki/Decoding-Data-Fields#device-ids).


Add this to the configuration.yaml:
```
fan:
  - platform: template
    fans:
      mechanische_ventilatie:
        friendly_name: "Mechanische Ventilatie"
        value_template: "{{ states('light.fan_speed') }}"
        preset_mode_template: >
          {% set output = ['Laag','Mid','Hoog','Auto'] %}
          {% set idx = state_attr('light.fan_speed', 'V_PERCENTAGE') | int - 1 %}
          {{ output[idx] }}
        turn_on:
          service: homeassistant.turn_on
          entity_id: light.fan_speed
        turn_off:
          service: homeassistant.turn_off
          entity_id: light.fan_speed
        set_preset_mode:
          service: light.turn_on
          entity_id: light.fan_speed
          data:
            brightness_pct: >
              {% set mapper = {'Laag': 1, 'Mid': 2, 'Hoog': 3, 'Auto': 4} %}
              {{ mapper[preset_mode] }}
        preset_modes:
          - Laag
          - Mid
          - Hoog
          - Auto
```

Perform a HA restart, check if the entity becomes visible and operates.

## Implement the Frontface card

![Preview](https://github.com/Marcelh1/fanx/blob/main/images/preview_animation.gif)

To implement a button control on your frontface, you should put these [files](https://github.com/Marcelh1/fanx/tree/main/data/mysensors%20www%20folder) in the “www” folder.

Next, you have to register the JavaScript at: “Settings”, “Dashboards”, then click on the “right top three dots”, hit: “Sources”. You should add a source: 

![Source](https://github.com/Marcelh1/fanx/blob/main/images/help_source.png)

Now it's a good time to restart HA, then add the entity card:
```
type: entities
entities:
  - entity: fan.mechanische_ventilatie
    name: FAN
    type: custom:custom-fan-card
    tap_action:
      action: none
title: Mechanische ventilatie
```

Optionally, to overwrite the [icon](https://github.com/Marcelh1/fanx/blob/main/images/fan_blue.png) shown on the frontpage, you can add the rule shown on the right in configurations.yaml
```
customize:
  fan.mechanische_ventilatie:
    entity_picture: /local/icons/fan_blue.png
```

# Testing the Dongle in Windows with MYSController
To test the Dongle operation, you can use the application: MYSController. When opening the connection with a Dongle that has cloned a remote before, it will show this:

![MYSController](https://github.com/Marcelh1/fanx/blob/main/images/myscontroller.png)

If you would like to clone a remote, just click on "Clone switch", change "Subtype" to "V_STATUS" and enter "1" in the Payload field and hit the "Send" button:

![Clone settings](https://github.com/Marcelh1/fanx/blob/main/images/clone_settings.png)
