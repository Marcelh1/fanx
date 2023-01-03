# MySensors configuration in Home Assistant
Use these settings:
- MySensors version: 2.3.2
- Serial connection
- Baudrate: 38400 bps

# MySensors Arduino LIB
In the library manager, install MySensors lib.

# Home Assistant
Add this to the configuration.yaml:

fan:
  - platform: template
    fans:
      mechanische_ventilatie:
        friendly_name: "Mechanische Ventilatie"
        value_template: "{{ states('light.fanx') }}"
        preset_mode_template: >
          {% set output = ['Laag','Mid','Hoog','Auto'] %}
          {% set idx = state_attr('light.fanx', 'V_PERCENTAGE') | int - 1 %}
          {{ output[idx] }}
        turn_on:
          service: homeassistant.turn_on
          entity_id: light.fanx
        turn_off:
          service: homeassistant.turn_off
          entity_id: light.fanx
        set_preset_mode:
          service: light.turn_on
          entity_id: light.fanx
          data:
            brightness_pct: >
              {% set mapper = {'Laag': 1, 'Mid': 2, 'Hoog': 3, 'Auto': 4} %}
              {{ mapper[preset_mode] }}
        preset_modes:
          - Laag
          - Mid
          - Hoog
          - Auto
