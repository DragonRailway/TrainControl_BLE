# TrainControl_BLE

This is the official firmware for our DragonRailway Project. **TrainControl_BLE** allows wireless control of a DragonRailway locomotives via Bluetooth (BLE).

## Hardware

### TRACKLINK (recommended)

The **TRACKLINK** board is our custom designed control board for DragonRailway. It comes with the necessary components for your convenience:

- **2 DC Motor Drivers:** Control the train's propulsion motors.
- **6 LED Drivers:** lights on the train (headlights, tail lights, cablight, walkwaylight, ditchlight etc.)
- **Voltage sensor:** Detects the battery voltage - for low battery warning
- **1 MAX98357 Amplifier:** Audio output playing train horn sounds or other effects (Not implemented yet).

### ESP32 or ESP32S3 Dev Boards

This project will still work with a standard dev boards. However, you will need to manually connect external motor drivers. Below are some guidelines for using an ESP32 dev board:

- **DC Motor Control:** Connect external motor drivers (like L298N or TB6612FNG)
- **LEDs:** Use resistors with LEDs and connect them to available GPIO pins.

## Software

- RemoteXY app - https://remotexy.com/en/download/
- VSCode with PlatformIO extension
    - Download VScode https://code.visualstudio.com/
    - Install PlatformIO https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide