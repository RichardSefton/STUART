#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>

#ifndef __AVR_ATtiny1627__
    #include <avr/iotn1627.h>
#endif

typedef enum 
{
    FREQ = 50,
    MID_MS = 15,
    LOW_MS = 10,
    HIGH_MS = 20,
} enum_SG90;

typedef struct
{
    uint8_t FREQ;
    float MID_MS;
    float LOW_MS;
    float HIGH_MS;
} SG90;

typedef struct
{
    uint16_t PERIOD;
    uint16_t LEFT;
    uint16_t MID;
    uint16_t RIGHT;
    uint16_t CURRENT;
} Direction;

typedef struct
{
    register8_t* TCAROUTE;
    register8_t* CTRLA;
    register8_t* CTRLB;
    uint8_t PORTMUX_gc;
    uint8_t WGMODE_gc;
    uint8_t CPMEN_bm;
    uint8_t CLKDIV_gc;
    uint8_t RUNMODE_bm;
    register16_t* PER;
    register16_t* CMP0BUFF;
} ServoPWM_Config;

typedef struct 
{
    SG90 MODEL;
    unsigned long CLK_SPEED;
    uint8_t PRESCALER;
    Direction DIR;
    ServoPWM_Config* PWM_CONFIG;
    register8_t* PORT;
    uint8_t PIN_bm;
} Servo;

ServoPWM_Config* Servo_PWM_new(
    register8_t *TCAROUTE,
    register8_t *CTRLA,
    register8_t *CTRLB,
    uint8_t PORTMUX_gc,
    uint8_t WGMODE_gc,
    uint8_t CPMEN_bm,
    uint8_t CLKDIV_gc,
    uint8_t RUNMODE_bm,
    register16_t *PER,
    register16_t *CMP0BUFF
);
void Servo_PWM_delete(ServoPWM_Config*);

Servo* Servo_new(
    unsigned long, 
    uint8_t, 
    ServoPWM_Config*,
    register8_t* PORT,
    uint8_t PIN_bm
);
void Servo_delete(Servo*);

void Servo_CalculatePeriod(Servo*);
void Servo_CalculateDirections(Servo*);

void Servo_InitPins(Servo*);
void Servo_InitPWM(Servo*);

void Servo_Move(Servo*, uint16_t);
