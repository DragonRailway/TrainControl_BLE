
// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "Dragon Locomotive"  //  Locomotive Model Name
#define REMOTEXY_ACCESS_PASSWORD "1234"              //  Bluetooth Access Password

// Uncomment this line if your model has more than 2 leds
#define EXTENDED_LED_CONTROL

// Uncomment this line if you use more than 6 leds
// #define FREE_UP_LEDC

// Uncomment this line if you use 2 motor drivers MA & MB
// #define DUAL_MOTOR_DRIVER

// Uncomment if you want to invert motor direction
// #define INVERT_MOTOR_A
// #define INVERT_MOTOR_B

// The motor will be silent at frequencies above 20,000Hz
// Set the minimum power in which the locomotive starts to move
// This is usually higher with higher frequencies
int8_t minPower = 50;  //  in percentage

float vBat = 0.00;
float batteryLow = 3.3;       //  warning voltage
float batteryCritical = 3.0;  //  cell cutoff voltage
uint8_t cellCount = 0;        //  0 for auto detection

// Set the maximum power for led in percentage
uint16_t headlightMaxPower = 100;
uint16_t taillightMaxPower = 100;
uint16_t cablightMaxPower = 100;
uint16_t walklightMaxPower = 100;
uint16_t ditchlightMaxPower = 100;