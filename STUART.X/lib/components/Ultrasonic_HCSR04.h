#include <stdio.h>
#include <avr/io.h>
#ifndef __AVR_ATtiny1627__
    #include <avr/iotn1627.h>
#endif
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
    register16_t* COMPARE;
    uint16_t COMPARE_VALUE;
    register16_t* CNT;
    uint16_t CNT_VALUE;
    register8_t* CTRLA;
    uint8_t CLKSEL_gc;
    register8_t* CTRLB;
    uint8_t INT_gc;
    register8_t* INTCTRL;
    uint8_t INT_bm;
    uint8_t RUN_MODE_bm;
} Ultrasonic_TC_Config;

typedef struct
{
    register8_t* CLKSEL;
    uint8_t CLKSEL_gc;
    register8_t* STATUS;
    register8_t* CTRLA;
    uint8_t PERBUSY_bm;
    register16_t* PER;
    uint16_t PER_VALUE;
    register8_t* INTCTRL;
    uint8_t INT_bm;
    uint8_t RUN_MODE_bm;
    uint8_t PRESCALER_gc;
    register16_t* CNT;
} Ultrasonic_RTC_Config;

typedef struct
{
    //PORTS AND PINS
    register8_t* TRIGGER;
    register8_t* ECHO;

    register8_t* TRIG_TGL;
    register8_t* ECHO_IN;

    uint8_t TRIG_PIN_bm;
    uint8_t ECHO_PIN_bm; 

    //TIMER
    Ultrasonic_TC_Config* _TC;

    //RTC
    Ultrasonic_RTC_Config* _RTC;

    //CALIBRATION
    unsigned long CLK_FREQ;
} Ultrasonic_Config;

typedef struct 
{
    float LEFT;
    float MID;
    float RIGHT;
} Distances;

typedef struct
{
    Ultrasonic_Config* CONFIG;
    uint16_t startTime;
    uint16_t endTime;
    float speedOfSound;
    float time;
    float tickTime;
    float distance;
    Distances DISTANCES;
} Ultrasonic;

Ultrasonic_TC_Config* Ultrasonic_TC_Config_new(
    register16_t *COMPARE,
    uint16_t COMPARE_VALUE,
    register16_t *CNT,
    uint16_t CNT_VALUE,
    register8_t *CTRLA,
    uint8_t CLKSEL_gc,
    register8_t *CTRLB,
    uint8_t INT_gc,
    register8_t *INTCTRL,
    uint8_t INT_bm,
    uint8_t RUN_MODE_bm
);
void Ultrasonic_TC_Config_delete(Ultrasonic_TC_Config*);

Ultrasonic_RTC_Config* Ultrasonic_RTC_Config_new(
    register8_t* CLKSEL,
    uint8_t CLKSEL_gc,
    register8_t* STATUS,
    register8_t* CTRLA,
    uint8_t PERBUSY_bm,
    register16_t* PER,
    uint16_t PER_VALUE,
    register8_t* INTCTRL,
    uint8_t INT_bm,
    uint8_t RUN_MODE_bm,
    uint8_t PRESCALER_gc,
    register16_t* CNT
);
void Ultrasonic_RTC_Config_delete(Ultrasonic_RTC_Config*);

Ultrasonic_Config* Ultrasonic_Config_new(
    register8_t* TRIGGER,
    register8_t* ECHO,
    register8_t* TRIG_TGL,
    register8_t* ECHO_IN,
    uint8_t TRIG_PIN_bm,
    uint8_t ECHO_PIN_bm,
    Ultrasonic_TC_Config* _TC,
    Ultrasonic_RTC_Config* _RTC,
    unsigned long CLK_FREQ
);
void Ultrasonic_Config_delete(Ultrasonic_Config*);

Ultrasonic* Ultrasonic_new(Ultrasonic_Config*, float tickTime, float speedOfSound);
void Ultrasonic_delete(Ultrasonic*);

void Ultrasonic_InitPins(Ultrasonic* self);

void Ultrasonic_InitRTC(Ultrasonic* self);
void Ultrasonic_InitTC(Ultrasonic* self);
void Ultrasonic_EnableTC(Ultrasonic* self);
void Ultrasonic_DisableTC(Ultrasonic* self);

void Ultrasonic_ResetTime(Ultrasonic* self);
void Ultrasonic_SetBeginTime(Ultrasonic* self);
void Ultrasonic_SetEndTime(Ultrasonic* self);

void Ultrasonic_TriggerOn(Ultrasonic* self);
void Ultrasonic_TriggerOff(Ultrasonic* self);
void Ultrasonic_Measure(Ultrasonic* self);

void Ultrasonic_CalculateDistance(Ultrasonic* self);