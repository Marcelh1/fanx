# MySensors configuration in Home Assistant
Use these settings:
- MySensors version: 2.3.2
- Serial connection
- Baudrate: 38400 bps

# MySensors Arduino LIB
In the library manager, install MySensors lib if you want to change or upload the Arduino Sketch.

# Home Assistant
The Dongle will create 4 entities in Home Assistant:

![entities](https://github.com/Marcelh1/fanx/blob/main/images/ha_entities_ramses.png?)

### 1. Clone Switch (Control)
Activate this switch to put the Dongle into clone mode, then press a random key on the RF15 remote to clone the addresses. After 5s timeout or succesful clone, the switch will be de-activated automatically, if the clone was succesful, the source and target address will be updated as well.
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
