#ifndef CUSTOM_PINS_H
#define CUSTOM_PINS_H
#ifdef DIY_BOARD

// Motor driver pins
#define DRV_MA1 12 //  PWM1 pin
#define DRV_MA2 13 //  PWM2 pin
// Second Motor driver pins, only required if you're using DUAL_MOTOR_DRIVER option
// #define DRV_MB1 14 //  PWM1 pin
// #define DRV_MB2 15 //  PWM2 pin
//  Driver enable pin, assign dummy pin if your driver doesn't have enable pin
#define DRV_EN 14

// LED pins
#define L1 5
#define L2 15
#define L3 17
#define L4 18
#define L5 19
#define L6 21

#define VSENS 4
#define VSCALE 4.2

#define LED_FREQ 10000
#define LED_RES 10
#define DRV_FREQ 24000
#define DRV_RES 8

#endif
#endif