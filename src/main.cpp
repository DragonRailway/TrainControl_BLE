
#define EXTENDED_LED_CONTROL

//  https://github.com/bxparks/AceRoutine
#include <AceRoutine.h>  // AceRoutine
using namespace ace_routine;

//  https://github.com/madhephaestus/ESP32Servo

//  https://github.com/RemoteXY/RemoteXY-Arduino-library
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <RemoteXY.h>

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "Rambros_Locomotive"  //  Locomotive Model Name
#define REMOTEXY_ACCESS_PASSWORD "1234"               //  Bluetooth Access Password

// RemoteXY GUI configuration
#pragma pack(push, 1)

//////////////////////////////////////////////
//     RemoteXY include configuration       //
//////////////////////////////////////////////

#ifdef EXTENDED_LED_CONTROL

uint8_t RemoteXY_CONF[] =  // 103 bytes
  { 255, 4, 0, 1, 0, 96, 0, 19, 0, 0, 0, 0, 24, 1, 126, 200, 1, 1, 5, 0,
    4, 78, 23, 19, 100, 0, 79, 111, 2, 64, 138, 46, 20, 1, 172, 79, 16, 31, 62, 0,
    60, 0, 10, 20, 134, 27, 27, 48, 78, 26, 31, 79, 78, 0, 31, 79, 70, 70, 0, 74,
    0, 3, 126, 15, 21, 2, 186, 16, 64, 68, 114, 97, 103, 111, 110, 32, 82, 97, 105, 108,
    119, 97, 121, 0, 37, 0, 83, 116, 114, 105, 110, 103, 32, 50, 0, 3, 24, 31, 20, 89,
    5, 78, 26 };

// this structure defines all the variables and events of your control interface
struct {

  // input variables
  int8_t speed;    // from 0 to 100
  uint8_t dir;     // =1 if switch ON and =0 if OFF
  uint8_t toggle;  // =1 if state is ON, else =0
  uint8_t led_id;  // from 0 to 5

  // output variables
  uint8_t serial_string;  // from 0 to 2

  // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;

#else

uint8_t RemoteXY_CONF[] =  // 112 bytes
  { 255, 4, 0, 1, 0, 105, 0, 19, 0, 0, 0, 0, 24, 1, 126, 200, 1, 1, 5, 0,
    4, 78, 29, 19, 100, 0, 79, 111, 2, 36, 143, 55, 21, 1, 172, 79, 16, 31, 62, 0,
    60, 0, 10, 24, 92, 28, 28, 48, 78, 26, 31, 79, 78, 0, 31, 79, 70, 70, 0, 74,
    0, 3, 126, 15, 21, 2, 186, 16, 64, 68, 114, 97, 103, 111, 110, 32, 82, 97, 105, 108,
    119, 97, 121, 0, 37, 0, 83, 116, 114, 105, 110, 103, 32, 50, 0, 10, 24, 45, 28, 28,
    48, 78, 26, 31, 79, 78, 0, 31, 79, 70, 70, 0 };

// this structure defines all the variables and events of your control interface
struct {

  // input variables
  int8_t speed;     // from 0 to 100
  uint8_t dir;      // =1 if switch ON and =0 if OFF
  uint8_t toggle;   // =1 if state is ON, else =0
  uint8_t toggle1;  // =1 if state is ON, else =0

  // output variables
  uint8_t serial_string;  // from 0 to 2

  // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;

#endif

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

int16_t speed = 0;
int8_t dir = 1;

float voltage = 0.00;

bool headlight = 0;   //  L1
bool taillight = 0;   //  L2
bool cablight = 0;    //  L3
bool walklight = 0;   //  L4
bool ditchlight = 0;  //  L5,L6

int16_t headlightBrightness = 0;
int16_t taillightBrightness = 0;
int16_t cablightBrightness = 0;
int16_t walklightBrightness = 0;
int16_t ditchlightBrightness = 0;

int8_t led_select = 0;


int16_t fade(uint8_t pinNumber, bool ledState, int16_t ledBrightness) {
  if (ledState == 1 && ledBrightness < 255) {
    ledBrightness = ledBrightness + 1;
  } else if (ledState == 0 && ledBrightness > 0) {
    ledBrightness = ledBrightness - 1;
  }
  ledBrightness = constrain(ledBrightness, 0, 255);
  analogWrite(pinNumber, ledBrightness);
  return ledBrightness;
}

int16_t alternateFade(uint8_t pinNumber1, uint8_t pinNumber2, bool ledState, int16_t ledBrightness) {
  static int8_t alternateBrightness = 2;
  if (ledState == 1) {
    if (ledBrightness >= 255) {
      alternateBrightness = alternateBrightness * -1;
    }
    if (ledBrightness <= 0) {
      alternateBrightness = alternateBrightness * -1;
    }
    ledBrightness = ledBrightness + alternateBrightness;
    ledBrightness = constrain(ledBrightness, 0, 255);
    analogWrite(pinNumber1, ledBrightness);
    analogWrite(pinNumber2, 255 - ledBrightness);

  } else {
    analogWrite(pinNumber1, 0);
    analogWrite(pinNumber2, 0);
  }

  return ledBrightness;
}
void setup() {
  RemoteXY_Init();

  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);
  pinMode(L5, OUTPUT);
  pinMode(L6, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(DRV_MA1, OUTPUT);
  pinMode(DRV_MA2, OUTPUT);
  pinMode(DRV_MB1, OUTPUT);
  pinMode(DRV_MB2, OUTPUT);
  pinMode(DRV_EN, OUTPUT);

  pinMode(SD_MODE, OUTPUT);
  digitalWrite(SD_MODE, LOW);

  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
}

#ifdef EXTENDED_LED_CONTROL
COROUTINE(ledInput) {
  COROUTINE_LOOP() {

    switch (RemoteXY.led_id) {
      case 0:
        if (led_select != RemoteXY.led_id) {
          led_select = RemoteXY.led_id;
          RemoteXY.toggle = headlight;
        }
        headlight = RemoteXY.toggle;
        break;
      case 1:
        if (led_select != RemoteXY.led_id) {
          led_select = RemoteXY.led_id;
          RemoteXY.toggle = taillight;
        }
        taillight = RemoteXY.toggle;
        break;
      case 2:
        if (led_select != RemoteXY.led_id) {
          led_select = RemoteXY.led_id;
          RemoteXY.toggle = cablight;
        }
        cablight = RemoteXY.toggle;
        break;
      case 3:
        if (led_select != RemoteXY.led_id) {
          led_select = RemoteXY.led_id;
          RemoteXY.toggle = walklight;
        }
        walklight = RemoteXY.toggle;
        break;
      case 4:
        if (led_select != RemoteXY.led_id) {
          led_select = RemoteXY.led_id;
          RemoteXY.toggle = ditchlight;
        }
        ditchlight = RemoteXY.toggle;
        break;
    }

    COROUTINE_DELAY(50);
  }
}

#else
COROUTINE(ledInput) {
  COROUTINE_LOOP() {
    headlight = RemoteXY.toggle1;
    taillight = RemoteXY.toggle1;
    COROUTINE_DELAY(100);
  }
}
#endif

COROUTINE(ledOutput) {
  COROUTINE_LOOP() {

    headlightBrightness = fade(L1, headlight, headlightBrightness);
    headlightBrightness = fade(L2, headlight, headlightBrightness);
    taillightBrightness = fade(L2, taillight, taillightBrightness);
    cablightBrightness = fade(L3, cablight, cablightBrightness);
    walklightBrightness = fade(L4, walklight, walklightBrightness);
    ditchlightBrightness = alternateFade(L5, L6, ditchlight, ditchlightBrightness);
    COROUTINE_DELAY(10);
  }
}

COROUTINE(motorOutput) {
  COROUTINE_LOOP() {

    COROUTINE_DELAY(10);
  }
}

COROUTINE(blink) {
  COROUTINE_LOOP() {
    digitalWrite(LED_BUILTIN, HIGH);
    COROUTINE_DELAY(500);
    digitalWrite(LED_BUILTIN, LOW);
    COROUTINE_DELAY(500);
  }
}


void loop() {
  RemoteXY_Handler();

  ledInput.runCoroutine();
  ledOutput.runCoroutine();
  motorOutput.runCoroutine();
  blink.runCoroutine();
}
