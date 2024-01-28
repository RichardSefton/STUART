#include "Ultrasonic_HCSR04.h"

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
)
{
    Ultrasonic_TC_Config *instance = (Ultrasonic_TC_Config*)malloc(sizeof(Ultrasonic_TC_Config));
    if (instance == NULL)
    {
        while(1);
    }
    
    instance->COMPARE = COMPARE;
    instance->COMPARE_VALUE = COMPARE_VALUE;
    instance->CNT = CNT;
    instance->CNT_VALUE = CNT_VALUE;
    instance->CTRLA = CTRLA;
    instance->CLKSEL_gc = CLKSEL_gc;
    instance->CTRLB = CTRLB;
    instance->INT_gc = INT_gc;
    instance->INTCTRL = INTCTRL;
    instance->INT_bm = INT_bm;
    instance->RUN_MODE_bm = RUN_MODE_bm;
    
    return instance;
}
void Ultrasonic_TC_Config_delete(Ultrasonic_TC_Config *self)
{
    free(self);
}

Ultrasonic_RTC_Config* Ultrasonic_RTC_Config_new(
    register8_t *CLKSEL,
    uint8_t CLKSEL_gc,
    register8_t *STATUS,
    register8_t *CTRLA,
    uint8_t PERBUSY_bm,
    register16_t *PER,
    uint16_t PER_VALUE,
    register8_t *INTCTRL,
    uint8_t INT_bm,
    uint8_t RUN_MODE_bm,
    uint8_t PRESCALER_gc,
    register16_t *CNT
)
{
    Ultrasonic_RTC_Config *instance = (Ultrasonic_RTC_Config*)malloc(sizeof(Ultrasonic_RTC_Config));
    if (instance == NULL)
    {
        while(1);
    }
    
    instance->CLKSEL = CLKSEL;
    instance->CLKSEL_gc = CLKSEL_gc;
    instance->STATUS = STATUS;
    instance->CTRLA = CTRLA;
    instance->PERBUSY_bm = PERBUSY_bm;
    instance->PER = PER;
    instance->PER_VALUE = PER_VALUE;
    instance->INTCTRL = INTCTRL;
    instance->INT_bm = INT_bm;
    instance->RUN_MODE_bm = RUN_MODE_bm;
    instance->PRESCALER_gc = PRESCALER_gc;
    instance->CNT = CNT;
    
    return instance;
}
void Ultrasonic_RTC_Config_delete(Ultrasonic_RTC_Config *self)
{
    free(self);
}

Ultrasonic_Config* Ultrasonic_Config_new(
    register8_t *TRIGGER,
    register8_t *ECHO,
    register8_t *TRIG_TGL,
    register8_t *ECHO_IN,
    uint8_t TRIG_PIN_bm,
    uint8_t ECHO_PIN_bm,
    Ultrasonic_TC_Config *_TC,
    Ultrasonic_RTC_Config *_RTC,
    unsigned long CLK_FREQ
)
{
    Ultrasonic_Config *instance = (Ultrasonic_Config*)malloc(sizeof(Ultrasonic_Config));
    if (instance == NULL)
    {
        while(1);
    }
    
    instance->TRIGGER = TRIGGER;
    instance->ECHO = ECHO;
    instance->TRIG_TGL = TRIG_TGL;
    instance->ECHO_IN = ECHO_IN;
    instance->TRIG_PIN_bm = TRIG_PIN_bm;
    instance->ECHO_PIN_bm = ECHO_PIN_bm;
    instance->_TC = _TC;
    instance->_RTC = _RTC;
    instance->CLK_FREQ = CLK_FREQ;
    
    return instance;
}
void Ultrasonic_Config_delete(Ultrasonic_Config *self)
{
    free(self);
}

Ultrasonic* Ultrasonic_new(Ultrasonic_Config *config, float tickTime, float speedOfSound)
{
    Ultrasonic *instance = (Ultrasonic*)malloc(sizeof(Ultrasonic));
    if (instance == NULL)
    {
        while(1);
    }
    
    instance->startTime = 0;
    instance->endTime = 0;
    instance->distance = 255; //Init to max value. 0xFF is also valid. 
    instance->speedOfSound = speedOfSound;
    instance->tickTime = tickTime;
    instance->CONFIG= config;
    
    return instance;
}
void Ultrasonic_delete(Ultrasonic *self)
{
    free(self);
}


void Ultrasonic_InitPins(Ultrasonic *self)
{
    //Set the trigger pin to output
    *self->CONFIG->TRIGGER |= self->CONFIG->TRIG_PIN_bm;
    //Set the echo pin to input
    *self->CONFIG->ECHO &= ~self->CONFIG->ECHO_PIN_bm;
}

void Ultrasonic_InitRTC(Ultrasonic *self)
{
    //Select clock source
    *self->CONFIG->_RTC->CLKSEL = self->CONFIG->_RTC->CLKSEL_gc;
    //Wait for the clock to be ready
    while(*self->CONFIG->_RTC->STATUS & self->CONFIG->_RTC->PERBUSY_bm);
    //Set the period - This is the max the clock will count to. 
    //Really its arbitrary as it will be always running but: 
    // 32768 @ 32KHz = 1 second with 30.5176uS ticks. 
    *self->CONFIG->_RTC->PER = self->CONFIG->_RTC->PER_VALUE;

    //Enable the overflow interrupt
    *self->CONFIG->_RTC->INTCTRL |= self->CONFIG->_RTC->INT_bm;

    //Wait for the clock to be ready
    while(*self->CONFIG->_RTC->STATUS & self->CONFIG->_RTC->PERBUSY_bm);
    //Set the prescaler and running mode
    *self->CONFIG->_RTC->CTRLA = self->CONFIG->_RTC->RUN_MODE_bm | self->CONFIG->_RTC->PRESCALER_gc;
}
void Ultrasonic_InitTC(Ultrasonic *self)
{
    //Set the compare value
    *self->CONFIG->_TC->COMPARE = self->CONFIG->_TC->COMPARE_VALUE;
    //Set the counter value
    *self->CONFIG->_TC->CNT = self->CONFIG->_TC->CNT_VALUE;
    //Set the clock source
    *self->CONFIG->_TC->CTRLA |= self->CONFIG->_TC->CLKSEL_gc;
    //Set the counter mode
    *self->CONFIG->_TC->CTRLB |= self->CONFIG->_TC->INT_gc;
    //Enable the compare interrupt
    *self->CONFIG->_TC->INTCTRL |= self->CONFIG->_TC->INT_bm;
}

void Ultrasonic_EnableTC(Ultrasonic *self)
{
    //Enable the timer
    *self->CONFIG->_TC->CTRLA |= self->CONFIG->_TC->RUN_MODE_bm;
}
void Ultrasonic_DisableTC(Ultrasonic *self)
{
    //Disable the timer
    *self->CONFIG->_TC->CTRLA &= ~self->CONFIG->_TC->RUN_MODE_bm;
}

void Ultrasonic_ResetTime(Ultrasonic *self)
{
    self->startTime = 0;
    self->endTime = 0;
    self->time = 0;
    self->distance = 0;
}
void Ultrasonic_SetBeginTime(Ultrasonic *self)
{
    self->startTime = *self->CONFIG->_RTC->CNT;
}
void Ultrasonic_SetEndTime(Ultrasonic *self)
{
    self->endTime = *self->CONFIG->_RTC->CNT;
}

void Ultrasonic_TriggerOn(Ultrasonic *self)
{
    //Turn on the trigger pin
    *self->CONFIG->TRIG_TGL |= self->CONFIG->TRIG_PIN_bm;
}
void Ultrasonic_TriggerOff(Ultrasonic *self)
{
    //Turn off the trigger pin
    *self->CONFIG->TRIG_TGL &= ~self->CONFIG->TRIG_PIN_bm;
}

void Ultrasonic_Measure(Ultrasonic *self)
{
    //Wait for echo pin to go high
    while(!(*self->CONFIG->ECHO_IN & self->CONFIG->ECHO_PIN_bm));
    //Set the start time
    Ultrasonic_SetBeginTime(self);
    //Wait for echo pin to go low
    while(*self->CONFIG->ECHO_IN & self->CONFIG->ECHO_PIN_bm);
    //Set the end time
    Ultrasonic_SetEndTime(self);
}

void Ultrasonic_CalculateDistance(Ultrasonic *self)
{
    self->time = (float)(self->endTime - self->startTime) * self->tickTime;
    //divide by 2 because we are calculating the time to the object and back
    self->distance = (float)self->time * (float)self->speedOfSound / (float)2.0;
}