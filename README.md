# ESTC-project
ESTC Main Project

This application manipulates LED using Button, CLI and BLE
# Build 
```
make SDK_ROOT=(Path to SDK) dfu
```
To enable cli
```
make SDK_ROOT=(Path to SDK) ESTC_USB_CLI_ENABLED=1  dfu
```
# Usage
### Button
To change LED using button, there are 4 states to consider
1. Display mode
2. Hue change
3. Saturation change
4. Value change

In order to save current RGB value, loop through until Display mode

### CLI
To connect to dongle, use
```
picocom /dev/ttyACM1
```

To change LED using CLI, use following commands
#### RGB
```
RGB <red> <green> <blue>
```
Note: max value - 255

Example
```
RGB 255 0 0
```

#### HSV
```
HSV <hue> <saturation> <value>
```
Note: max value - 255

Example
```
HSV 255 0 0
```

### BLE
![image](https://github.com/Vafo/ESTC-project/assets/29276900/cf6f8f12-959e-4eb2-a1b3-c80caecd8177)

Unknown Service is corresponding to LED Service

To set new RGB value, enter value of format (0xRRGGBB) into Characteristic of UUID  a5dbabbb-...

To get notifications of RGB update, use Characteristic of UUID a5dbabbc-...

If confused, look for corresponding User Descriptions

# Logs
To view logs, use
```
picocom /dev/ttyACM0
```

# Known Issues
When setting RGB value, BLUE Component does not transmit properly on BLE

Relate to issue #45
