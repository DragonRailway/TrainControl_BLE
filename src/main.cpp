//  Uncomment this line if your model has more than 2 leds
#define EXTENDED_LED_CONTROL

#include <Arduino.h>
#include <pwmWrite.h>  //  https://github.com/Dlloydev/ESP32-ESP32S2-AnalogWrite
Pwm pwm = Pwm();
#include <AceRoutine.h>  //  https://github.com/bxparks/AceRoutine
using namespace ace_routine;

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

int16_t speed = 0;
int8_t dir = 1;
int8_t minPower = 12;  //  in percentage

float vBat = 0.00;
uint8_t lowBatWarning = 0;

bool lightState[5] = { 0 };
enum LedIndex {
  HEADLIGHT = 0,  //  L1
  TAILLIGHT = 1,  //  L2
  CABLIGHT = 2,   //  L3
  WALKLIGHT = 3,  //  L4
  DITCHLIGHT = 4  //  L5,L6
};

// Variables to keep track of led brightness
int16_t headlightBrightness = 0;
int16_t taillightBrightness = 0;
int16_t cablightBrightness = 0;
int16_t walklightBrightness = 0;
int16_t ditchlightBrightness = 0;

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

int16_t fade(uint8_t pinNumber, bool ledState, int16_t ledBrightness) {
  int16_t prevBrightness = ledBrightness;
  if (ledState == 1 && ledBrightness < 255) {       // if led is on and brightness is less than max
    ledBrightness = ledBrightness + 1;              // increase brightness by 1
  } else if (ledState == 0 && ledBrightness > 0) {  // if led is off and brightness is greater than 0
    ledBrightness = ledBrightness - 1;              // decrease brightness by 1
  }
  ledBrightness = constrain(ledBrightness, 0, 255);  // limit brightness between 0 and 255

  if (ledBrightness >= 255 || ledBrightness <= 0) {  // if brightness did not change
    if (prevBrightness != ledBrightness) {
      pwm.detach(pinNumber);  // this frees up a ledc channel
      pinMode(pinNumber, OUTPUT);
    }

    digitalWrite(pinNumber, ledState);
  } else {
    pwm.write(pinNumber, ledBrightness, LED_FREQ, LED_FREQ);  // write brightness to the led pin
  }
  return ledBrightness;  // return the new brightness value
}

int16_t alternateFade(uint8_t pinNumber1, uint8_t pinNumber2, bool ledState, int16_t ledBrightness) {
  static int8_t alternateBrightness = 2;  // direction, strength of brightness change
  if (ledState == 1) {
    if (ledBrightness >= 255) {
      alternateBrightness = alternateBrightness * -1;
    }
    if (ledBrightness <= 0) {
      alternateBrightness = alternateBrightness * -1;
    }
    ledBrightness = ledBrightness + alternateBrightness;
    ledBrightness = constrain(ledBrightness, 0, 255);                // limit pwm duty range
    pwm.write(pinNumber1, ledBrightness, LED_FREQ, LED_FREQ);        // write brightness to pin 1
    pwm.write(pinNumber2, 255 - ledBrightness, LED_FREQ, LED_FREQ);  // write inverse brightness to pin 2

  } else {
    pwm.detach(pinNumber1);       // detach pin 1 from Pwm
    pwm.detach(pinNumber2);       // detach pin 2 from Pwm
    pinMode(pinNumber1, OUTPUT);  // set pin 1 to output
    pinMode(pinNumber2, OUTPUT);  // set pin 2 to output
    digitalWrite(pinNumber1, 0);  // turn off pin 1
    digitalWrite(pinNumber2, 0);  // turn off pin 2
  }

  return ledBrightness;  // return new brightness value
}

void runMotor(uint8_t pwmPin1, uint8_t pwmPin2, int16_t speed, bool direction) {
  uint16_t minPwmDuty = minPower * 255 / 100;

  if (speed > 0) {
    speed = map(speed, 0, 100, minPwmDuty, 255);
    digitalWrite(DRV_EN, HIGH);
    switch (direction) {
      case 0:
        pwm.write(pwmPin1, 0);
        pwm.write(pwmPin2, speed);
        break;
      case 1:
        pwm.write(pwmPin1, speed);
        pwm.write(pwmPin2, 0);
        break;
    }
  } else {
    digitalWrite(DRV_EN, LOW);
    pwm.write(pwmPin1, 0);
    pwm.write(pwmPin2, 0);
  }
}
void setup() {
  RemoteXY_Init();

  //  LED pins
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);
  pinMode(L5, OUTPUT);
  pinMode(L6, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  //  H bridge pins
  pwm.attach(DRV_MA1, 0);
  pwm.attach(DRV_MA2, 1);
  pwm.attach(DRV_MB1, 0);
  pwm.attach(DRV_MB2, 1);
  pwm.setFrequency(DRV_MA1, DRV_FREQ);
  pwm.setFrequency(DRV_MA2, DRV_FREQ);
  pwm.setResolution(DRV_MA1, DRV_RES);
  pwm.setResolution(DRV_MA2, DRV_RES);
  pinMode(DRV_EN, OUTPUT);
  digitalWrite(DRV_EN, LOW);

  //  Audio amplifier pins - not implemented yet
  pinMode(SD_MODE, OUTPUT);
  digitalWrite(SD_MODE, LOW);
  //  Servo Pins
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
}

#ifdef EXTENDED_LED_CONTROL
COROUTINE(ledInput) {
  COROUTINE_LOOP() {
    static int8_t led_select = 0;
    switch (RemoteXY.led_id) {
      case 0:
        if (led_select != RemoteXY.led_id) {
          led_select = RemoteXY.led_id;
          RemoteXY.toggle = lightState[led_select];
        }
        lightState[led_select] = RemoteXY.toggle;
        break;
    }
    COROUTINE_DELAY(50);
  }
}

#else
COROUTINE(ledInput) {
  COROUTINE_LOOP() {
    lightState[HEADLIGHT] = RemoteXY.toggle;
    lightState[TAILLIGHT] = RemoteXY.toggle1;
    COROUTINE_DELAY(100);
  }
}
#endif

COROUTINE(ledOutput) {
  COROUTINE_LOOP() {
    headlightBrightness = fade(LED_BUILTIN, lightState[HEADLIGHT], headlightBrightness);

    headlightBrightness = fade(L1, lightState[HEADLIGHT], headlightBrightness);
    taillightBrightness = fade(L2, lightState[TAILLIGHT], taillightBrightness);
    cablightBrightness = fade(L3, lightState[CABLIGHT], cablightBrightness);
    walklightBrightness = fade(L4, lightState[WALKLIGHT], walklightBrightness);
    ditchlightBrightness = alternateFade(L5, L6, lightState[DITCHLIGHT], ditchlightBrightness);
    COROUTINE_DELAY(10);
  }
}

COROUTINE(motorOutput) {
  COROUTINE_LOOP() {
    runMotor(DRV_MA1, DRV_MA2, RemoteXY.speed, RemoteXY.dir);
    //runMotor(DRV_MB1, DRV_MB2, RemoteXY.speed, RemoteXY.dir);
    COROUTINE_DELAY(10);
  }
}

COROUTINE(health) {
  COROUTINE_LOOP() {
    if (RemoteXY.connect_flag == 0) {
      digitalWrite(DRV_EN, LOW);
      RemoteXY.speed = 0;
      RemoteXY.toggle = 0;
      lightState[HEADLIGHT] = !lightState[HEADLIGHT];
      lightState[5] = 1;
    }
    vBat = analogRead(VSENS) * VSCALE / 1000;
    COROUTINE_DELAY(500);  //  Check health 2 times/second
  }
}

void loop() {
  RemoteXY_Handler();

  ledInput.runCoroutine();
  health.runCoroutine();

  motorOutput.runCoroutine();
  ledOutput.runCoroutine();
}
