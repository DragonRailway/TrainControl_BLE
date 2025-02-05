

#include <Arduino.h>
#include <pwmWrite.h> //  https://github.com/Dlloydev/ESP32-ESP32S2-AnalogWrite
Pwm pwm = Pwm();
#include <AceRoutine.h> //  https://github.com/bxparks/AceRoutine
using namespace ace_routine;

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "Dragon Railway Locomotive" //  Locomotive Model Name
#define REMOTEXY_ACCESS_PASSWORD "1234"                     //  Bluetooth Access Password

//  https://github.com/RemoteXY/RemoteXY-Arduino-library
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <RemoteXY.h>

// Set the pins for your DIY board in this file
#include <custom_pins.h>

// Uncomment this line if your model has more than 2 leds
// #define EXTENDED_LED_CONTROL

// Uncomment this line if you use more than 6 leds
// #define FREE_UP_LEDC

// Uncomment this line if you use 2 motor drivers MA & MB
// #define DUAL_MOTOR_DRIVER

// Uncomment if you want to invert motor direction
// #define INVERT_MOTOR_A
// #define INVERT_MOTOR_B

// The motor will be silent at frequencies above 20,000Hz
// Set the minimum power which the locomotive starts to move
// This is usually higher with higher frequencies
int8_t minPower = 50; //  in percentage

float vBat = 0.00;
float batteryLow = 3.3;      //  warning voltage
float batteryCritical = 3.0; //  cell cutoff voltage
uint8_t cellCount = 0;       //  0 for auto detection

// Set the maximum power for lede in percentage
uint16_t headlightMaxPower = 100;
uint16_t taillightMaxPower = 100;
uint16_t cablightMaxPower = 100;
uint16_t walklightMaxPower = 100;
uint16_t ditchlightMaxPower = 100;

// Variables to keep track of led brightness
int16_t headlightBrightness = 0;
int16_t taillightBrightness = 0;
int16_t cablightBrightness = 0;
int16_t walklightBrightness = 0;
int16_t ditchlightBrightness = 0;

bool lightState[5] = {0};
enum LedIndex
{
  HEADLIGHT = 0, //  L1
  TAILLIGHT = 1, //  L2
  CABLIGHT = 2,  //  L3
  WALKLIGHT = 3, //  L4
  DITCHLIGHT = 4 //  L5,L6
};

uint16_t ledMaxPwm = 0;
uint16_t motorMaxPwm = 0;

// you can enable debug logging to Serial at 115200
// #define REMOTEXY__DEBUGLOG

// RemoteXY GUI configuration
#pragma pack(push, 1)

//////////////////////////////////////////////
//     RemoteXY include configuration       //
//////////////////////////////////////////////

#ifdef EXTENDED_LED_CONTROL

uint8_t RemoteXY_CONF[] = // 76 bytes
    {255, 4, 0, 17, 0, 69, 0, 19, 0, 0, 0, 0, 24, 1, 126, 200, 1, 1, 5, 0,
     4, 78, 23, 19, 100, 0, 79, 111, 2, 64, 138, 46, 20, 1, 172, 79, 16, 31, 62, 0,
     60, 0, 10, 20, 134, 27, 27, 48, 78, 26, 31, 79, 78, 0, 31, 79, 70, 70, 0, 3,
     24, 31, 20, 89, 5, 78, 26, 67, 0, 1, 126, 16, 69, 31, 186, 17};

// this structure defines all the variables and events of your control interface
struct
{

  // input variables
  int8_t speed;   // from 0 to 100
  uint8_t dir;    // =1 if switch ON and =0 if OFF
  uint8_t toggle; // =1 if state is ON, else =0
  uint8_t led_id; // from 0 to 5

  // output variables
  char string[17]; // string UTF8 end zero

  // other variable
  uint8_t connect_flag; // =1 if wire connected, else =0

} RemoteXY;

#else

uint8_t RemoteXY_CONF[] = // 85 bytes
    {255, 4, 0, 17, 0, 78, 0, 19, 0, 0, 0, 0, 24, 1, 126, 200, 1, 1, 5, 0,
     4, 78, 29, 19, 100, 0, 79, 111, 2, 36, 143, 55, 21, 1, 172, 79, 16, 31, 62, 0,
     60, 0, 10, 24, 92, 28, 28, 48, 78, 26, 31, 79, 78, 0, 31, 79, 70, 70, 0, 10,
     24, 45, 28, 28, 48, 78, 26, 31, 79, 78, 0, 31, 79, 70, 70, 0, 67, 0, 10, 126,
     15, 69, 31, 186, 17};

// this structure defines all the variables and events of your control interface
struct
{

  // input variables
  int8_t speed;    // from 0 to 100
  uint8_t dir;     // =1 if switch ON and =0 if OFF
  uint8_t toggle;  // =1 if state is ON, else =0
  uint8_t toggle1; // =1 if state is ON, else =0

  // output variables
  char string[17]; // string UTF8 end zero

  // other variable
  uint8_t connect_flag; // =1 if wire connected, else =0

} RemoteXY;

#endif

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

int16_t fade(uint8_t pinNumber, bool ledState, int16_t ledBrightness, int16_t maxPwmDuty)
{
  int16_t prevBrightness = ledBrightness;
  int16_t brightnessVariance = maxPwmDuty / 250;
  brightnessVariance = constrain(brightnessVariance, 1, maxPwmDuty);

  if (ledState == 1 && ledBrightness < maxPwmDuty)
  {                                                     // if led is on and brightness is less than max
    ledBrightness = ledBrightness + brightnessVariance; // increase brightness by 1
  }
  else if (ledState == 0 && ledBrightness > 0)
  {                                                         // if led is off and brightness is greater than 0
    ledBrightness = ledBrightness - brightnessVariance * 2; // decrease brightness by 1
  }
  ledBrightness = constrain(ledBrightness, 0, maxPwmDuty); // limit brightness between 0 and 255

#ifdef FREE_UP_LEDC
  if (ledBrightness >= maxPwmDuty || ledBrightness <= 0)
  { // if brightness did not change
    if (prevBrightness != ledBrightness)
    {
      pwm.detach(pinNumber); // this frees up a ledc channel
      pinMode(pinNumber, OUTPUT);
      digitalWrite(pinNumber, ledState);
    }
    digitalWrite(pinNumber, ledState);
  }
  else
  {
    pwm.write(pinNumber, ledBrightness, LED_FREQ, LED_RES); // write brightness to the led pin
  }
#else
  pwm.write(pinNumber, ledBrightness, LED_FREQ, LED_RES); // write brightness to the led pin
#endif

  return ledBrightness; // return the new brightness value
}

int16_t alternateFade(uint8_t pinNumber1, uint8_t pinNumber2, bool ledState, int16_t ledBrightness, int16_t maxPwmDuty)
{
  static int16_t alternateBrightness = maxPwmDuty / 50; // direction, strength of brightness change
  static bool startUpState = 0;                         // keep track of first cycle
  int16_t minPwnDuty = maxPwmDuty / 50;
  minPwnDuty = constrain(minPwnDuty, 0, maxPwmDuty);

  if (ledState == 1)
  {
    if (ledBrightness >= maxPwmDuty)
    {
      startUpState = 1;
      alternateBrightness = alternateBrightness * -1;
    }
    if (ledBrightness <= minPwnDuty)
    {
      alternateBrightness = alternateBrightness * -1;
    }
    ledBrightness = ledBrightness + alternateBrightness;
    ledBrightness = constrain(ledBrightness, minPwnDuty, maxPwmDuty); // limit pwm duty range
    pwm.write(pinNumber1, ledBrightness, LED_FREQ, LED_RES);          // write brightness to pin 1
    if (startUpState == 1)
    {
      pwm.write(pinNumber2, maxPwmDuty - ledBrightness, LED_FREQ, LED_RES); // write inverse brightness to pin 2}
    }
  }
  else
  {
    startUpState = 0;
#ifdef FREE_UP_LEDC
    pwm.detach(pinNumber1);      // detach pin 1 from Pwm
    pwm.detach(pinNumber2);      // detach pin 2 from Pwm
    pinMode(pinNumber1, OUTPUT); // set pin 1 to output
    pinMode(pinNumber2, OUTPUT); // set pin 2 to output
    digitalWrite(pinNumber1, 0); // turn off pin 1
    digitalWrite(pinNumber2, 0); // turn off pin 2
#else
    pwm.write(pinNumber1, 0, LED_FREQ, LED_RES); // write 0 to pin 1
    pwm.write(pinNumber2, 0, LED_FREQ, LED_RES); // write 0 to pin 2
#endif
  }
  return ledBrightness; // return new brightness value
}

void runMotor(uint8_t pwmPin1, uint8_t pwmPin2, int16_t speed, bool direction)
{
  uint16_t minPwmDuty = minPower * motorMaxPwm / 100;

  if (speed > 0)
  {
    speed = map(speed, 0, 100, minPwmDuty, motorMaxPwm);
    digitalWrite(DRV_EN, HIGH);
    switch (direction)
    {
    case 0:
      pwm.write(pwmPin1, 0);
      pwm.write(pwmPin2, speed);
      break;
    case 1:
      pwm.write(pwmPin1, speed);
      pwm.write(pwmPin2, 0);
      break;
    }
  }
  else
  {
    digitalWrite(DRV_EN, LOW);
    pwm.write(pwmPin1, 0);
    pwm.write(pwmPin2, 0);
  }
}

void setup()
{
  RemoteXY_Init();
  RemoteXY.dir = 1; // Set initial direction to forward

// Power control - only for official boards
#ifndef DIY_BOARD
  pinMode(PWR_EN, OUTPUT);
  digitalWrite(PWR_EN, HIGH);
#endif

  ledMaxPwm = (1 << LED_RES) - 1;
  motorMaxPwm = (1 << DRV_RES) - 1;

//  LED pins
#ifdef FREE_UP_LEDC
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);
  pinMode(L5, OUTPUT);
  pinMode(L6, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  headlightMaxPower = ledMaxPwm;
  taillightMaxPower = ledMaxPwm;
  cablightMaxPower = ledMaxPwm;
  walklightMaxPower = ledMaxPwm;
  ditchlightMaxPower = ledMaxPwm;
#else
  //  attach led pins to LEDC channels
  headlightMaxPower = map(headlightMaxPower, 0, 100, 0, ledMaxPwm);
  taillightMaxPower = map(taillightMaxPower, 0, 100, 0, ledMaxPwm);
  cablightMaxPower = map(cablightMaxPower, 0, 100, 0, ledMaxPwm);
  walklightMaxPower = map(walklightMaxPower, 0, 100, 0, ledMaxPwm);
  ditchlightMaxPower = map(ditchlightMaxPower, 0, 100, 0, ledMaxPwm);
  pwm.attach(L1, 2);
  pwm.attach(L2, 3);
  pwm.attach(L3, 4);
  pwm.attach(L4, 5);
  pwm.attach(L5, 6);
  pwm.attach(L6, 7);
#endif

//  H bridge pins
#ifndef INVERT_MOTOR_A
  pwm.attach(DRV_MA1, 0); //  set ledc channel
  pwm.attach(DRV_MA2, 1); //  set ledc channel
#else
  pwm.attach(DRV_MA1, 1); //  set ledc channel
  pwm.attach(DRV_MA2, 0); //  set ledc channel
#endif
#ifdef DUAL_MOTOR_DRIVER
#ifndef INVERT_MOTOR_B
  pwm.attach(DRV_MB1, 0); //  use same channel as motor 1
  pwm.attach(DRV_MB2, 1); //  use same channel as motor 1
#else
  pwm.attach(DRV_MB1, 1); //  use same channel as motor 1
  pwm.attach(DRV_MB2, 0); //  use same channel as motor 1
#endif
#endif

  pwm.setFrequency(DRV_MA1, DRV_FREQ); //  set pwm frequency
  pwm.setFrequency(DRV_MA2, DRV_FREQ);
  pwm.setResolution(DRV_MA1, DRV_RES); //  set pwm resolution
  pwm.setResolution(DRV_MA2, DRV_RES);
  pinMode(DRV_EN, OUTPUT);   //  set enable pin as output
  digitalWrite(DRV_EN, LOW); //  initialize driver disabled at startup

  if (cellCount == 0)
  {
    vBat = analogRead(VSENS) * VSCALE / 1000.0;
    if (vBat > (batteryCritical * 3.0))
    {
      cellCount = 3;
    }
    else if (vBat > (batteryCritical * 2.0))
    {
      cellCount = 2;
    }
    else if (vBat > (batteryCritical * 1.0))
    {
      cellCount = 1;
    }
  }
  batteryLow = cellCount * batteryLow;
  batteryCritical = cellCount * batteryCritical;

  /*
    //  Audio amplifier pins - not implemented yet
    pinMode(SD_MODE, OUTPUT);
    digitalWrite(SD_MODE, LOW);
  */

  CoroutineScheduler::setup();
}

#ifdef EXTENDED_LED_CONTROL
void printLEDtype()
{
  switch (RemoteXY.led_id)
  {
  case 0:
    sprintf(RemoteXY.string, "Head light");
    break;
  case 1:
    sprintf(RemoteXY.string, "Tail light");
    break;
  case 2:
    sprintf(RemoteXY.string, "Cab light");
    break;
  case 3:
    sprintf(RemoteXY.string, "Walk light");
    break;
  case 4:
    sprintf(RemoteXY.string, "Ditch light");
    break;
  default:
    break;
  }
}
COROUTINE(ledInput)
{
  COROUTINE_LOOP()
  {
    static int8_t led_select = 0;
    if (led_select != RemoteXY.led_id)
    {
      led_select = RemoteXY.led_id;
      RemoteXY.toggle = lightState[led_select];
      printLEDtype();
    }
    lightState[led_select] = RemoteXY.toggle;

    COROUTINE_DELAY(50);
  }
}

#else
COROUTINE(ledInput)
{
  COROUTINE_LOOP()
  {
    if (lightState[HEADLIGHT] != RemoteXY.toggle1)
    {
      lightState[HEADLIGHT] = RemoteXY.toggle1;
      if (RemoteXY.toggle1 == 1)
        sprintf(RemoteXY.string, "Head light: ON");
      if (RemoteXY.toggle1 == 0)
        sprintf(RemoteXY.string, "Head light: OFF");
    }
    if (lightState[TAILLIGHT] != RemoteXY.toggle)
    {
      lightState[TAILLIGHT] = RemoteXY.toggle;
      if (RemoteXY.toggle == 1)
        sprintf(RemoteXY.string, "Tail light: ON");
      if (RemoteXY.toggle == 0)
        sprintf(RemoteXY.string, "Tail light: OFF");
    }

    COROUTINE_DELAY(50);
  }
}
#endif

COROUTINE(ledOutput)
{
  COROUTINE_LOOP()
  {
    headlightBrightness = fade(L1, lightState[HEADLIGHT], headlightBrightness, headlightMaxPower);
    taillightBrightness = fade(L2, lightState[TAILLIGHT], taillightBrightness, taillightMaxPower);
    cablightBrightness = fade(L3, lightState[CABLIGHT], cablightBrightness, cablightMaxPower);
    walklightBrightness = fade(L4, lightState[WALKLIGHT], walklightBrightness, walklightMaxPower);
    ditchlightBrightness = alternateFade(L5, L6, lightState[DITCHLIGHT], ditchlightBrightness, ditchlightMaxPower);
    COROUTINE_DELAY(10);
  }
}

COROUTINE(motorOutput)
{
  COROUTINE_LOOP()
  {
    static bool dir = 0;
    if (RemoteXY.dir != dir)
    {
      dir = RemoteXY.dir;
      RemoteXY.speed = 0;
      if (dir == 0)
        sprintf(RemoteXY.string, "Reverse");
      if (dir == 1)
        sprintf(RemoteXY.string, "Forward");
    }

#ifndef INVERT_MOTOR_A
    runMotor(DRV_MA1, DRV_MA2, RemoteXY.speed, RemoteXY.dir);
#else
    runMotor(DRV_MA2, DRV_MA1, RemoteXY.speed, RemoteXY.dir);
#endif
    COROUTINE_DELAY(10);
  }
}

COROUTINE(batteryWarning)
{
  COROUTINE_BEGIN();
  if (vBat < batteryCritical)
  {
    sprintf(RemoteXY.string, "Critical: %.2fV", vBat);
    COROUTINE_DELAY(2000);
    sprintf(RemoteXY.string, "Motor disabled", vBat);
  }
  else if (vBat < batteryLow)
  {
    sprintf(RemoteXY.string, "Fuel Low: %.2fV", vBat);
    COROUTINE_DELAY(2000);
    sprintf(RemoteXY.string, "Recharge soon", vBat);
  }
  COROUTINE_END();
}

COROUTINE(health)
{
  COROUTINE_LOOP()
  {
    static bool prevConnectflag = 0;
    if (RemoteXY.connect_flag == 0)
    {

      digitalWrite(DRV_EN, LOW);
      RemoteXY.speed = 0;
      RemoteXY.toggle = 0;
    }

    if (prevConnectflag != RemoteXY.connect_flag)
    {
      prevConnectflag = RemoteXY.connect_flag;
      if (RemoteXY.connect_flag)
      {
        sprintf(RemoteXY.string, "RamBros 3D");
        COROUTINE_DELAY(2000);
        sprintf(RemoteXY.string, "Power: %dS %.1fV", cellCount, vBat);
        COROUTINE_DELAY(3000);
        sprintf(RemoteXY.string, "Dragon Railway");
      }
    }

    vBat = analogRead(VSENS) * VSCALE / 1000.0;
    if (vBat < batteryLow)
    {
      batteryWarning.runCoroutine();
    }
    COROUTINE_DELAY(250); //  Check health 4 times/second
  }
}

void loop()
{
  RemoteXY_Handler();

  CoroutineScheduler::loop();
  // fully nonblocking loop
}
