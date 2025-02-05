#ifndef CUSTOM_PINS_H
#define CUSTOM_PINS_H
#ifdef DIY_BOARD

// Motor driver pins
#define DRV_MA1 32 //  PWM1 pin
#define DRV_MA2 33 //  PWM2 pin
// Second Motor driver pins, only required if you're using DUAL_MOTOR_DRIVER option
// #define DRV_MB1 14 //  PWM1 pin
// #define DRV_MB2 15 //  PWM2 pin
//  Driver enable pin, assign dummy pin if your driver doesn't have enable pin
#define DRV_EN 23

// LED pins
#define L1 12
#define L2 14
#define L3 27
#define L4 26
#define L5 25
#define L6 23

#define VSENS 4
#define VSCALE 4.2

#define LED_FREQ 10000
#define LED_RES 10
#define DRV_FREQ 24000
#define DRV_RES 8

#endif
#endif