# TrainControl_BLE

This is the official firmware for our DragonRailway Project. **TrainControl_BLE** allows wireless control of a DragonRailway locomotives via Bluetooth (BLE).

## Hardware

### 1) TRACKLINK (recommended)

The **TRACKLINK** board is our custom designed control board for DragonRailway. It comes with the necessary components for your convenience:

- **2 DC Motor Drivers:** control the train's drive motors.
- **6 LED Drivers:** control lights on the train (headlights, tail lights, cablight, walkwaylight, ditchlight etc.)
- **Voltage sensor:** Detects the battery voltage - for low battery warning
- **1 MAX98357 Amplifier:** Audio output playing train horn sounds or other effects (Not implemented yet).

### 2) ESP32 or ESP32S3 Dev Boards

This project will still work with standard dev boards. However, you will need to manually connect external motor drivers. Below are some recommended modules:

- **DC Motor Control:** Connect external motor drivers (like L298N or TB6612FNG)
- **LEDs:** Use resistors with LEDs and connect them to available GPIO pins.
- **Voltage sensor:** Use voltage divider circuit to detect battery voltage.

## Software

- RemoteXY app - https://remotexy.com/en/download/
- VSCode with PlatformIO extension
    - Download VScode https://code.visualstudio.com/
    - Install PlatformIO https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide