# Preset switch for Line 6 M5 Stompbox

Repository contains a schematic and code for a configurable Line 6 M5 preset switcher based on Arduino Nano.
Switch has two buttons (B1, B2) and two LEDs with 6 colors and supports simple (only 4 presets) and more sophisticated (like 6 channels x 6  presets) configurations.

<img src="https://github.com/dawidm/m5_controller/blob/master/m5_controller.jpg?raw=true" alt="drawing" width="250"/>

## Configurations
The controller could be simply configured in 3 modes.

Only button 1 (B1) is used for configuration. To enter the configuration, you should plug in the power cord while holding button 1. LED 1 should be red and LED 2 off.

### 4 preset mode
In this mode, you can choose from M5 presets 1-4: 1 - short press B1, 2 - short press B2,  3 - short press B1, 4 - short press B2.

To set up this mode, enter configuration (plug in when holding B1). Then long press B1.

### Multiple channel mode
In this mode, you can choose a number of channels (maximum of 6) with 2 presets each.

To set up this mode:
* Enter configuration (plug in when holding B1).
* LED 1 is red (configuring channels).
* Short pres B1 as many times as many channels you want (maximum is 6).
* Then long press B1. LED 2 is red.
* Long press B1 again (done).

Now you can select a channel by holding B1 or B2 and a preset. Corresponding M5 preset numbers
* 1: Channel red + B1
* 2: Channel red + B2
* 3: Channel green + B1
* 4: Channel green + B2 etc. (blue, magenta, cyan, yellow)

### Multiple channel, multiple preset mode
In this mode, you can have multiple channels (maximum of 6), and each can have multiple presets (maximum of 6).

To set up this mode:
* Enter configuration (plug in when holding B1).
* LED 1 is red (configuring channels).
* Short press B1 for the number of desired channels.
* Long press B1.
* LED 2 is red (configuring presets).
* Short press B1 for the number of desired channels.
* Long press B1.

Now you can choose channel by short pressing B1 and preset by short pressing B2 (color order: red, green, blue, magenta, cyan, yellow).
