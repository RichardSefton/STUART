#include "Servo_SG90.h"

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
)
{
    ServoPWM_Config *instance = (ServoPWM_Config*)malloc(sizeof(ServoPWM_Config));
    
    if (instance == NULL)
    {
        while(1);
    }
    
    instance->TCAROUTE = TCAROUTE;
    instance->CTRLA = CTRLA;
    instance->CTRLB = CTRLB;
    instance->PORTMUX_gc = PORTMUX_gc;
    instance->WGMODE_gc = WGMODE_gc;
    instance->CPMEN_bm = CPMEN_bm;
    instance->CLKDIV_gc = CLKDIV_gc;
    instance->RUNMODE_bm = RUNMODE_bm;
    instance->PER = PER;
    instance->CMP0BUFF = CMP0BUFF;
    
    return instance;
}
void Servo_PWM_delete(ServoPWM_Config *self)
{
    free(self);
}


Servo* Servo_new(
    unsigned long CLK, 
    uint8_t PRESCALER, 
    ServoPWM_Config *config,
    register8_t *PORT,
    uint8_t PIN_bm
) 
{
    Servo *instance = (Servo*)malloc(sizeof(Servo));
    
    if (instance == NULL)
    {
        while(1);
    }
    
    instance->CLK_SPEED = CLK;
    instance->PRESCALER = PRESCALER;
    instance->MODEL.FREQ = FREQ;
    instance->MODEL.HIGH_MS = (float)(HIGH_MS/10000.0f);
    instance->MODEL.MID_MS = (float)(MID_MS/10000.0f);
    instance->MODEL.LOW_MS = (float)(LOW_MS/10000.0f);
    instance->PWM_CONFIG = config;
    instance->PORT = PORT;
    instance->PIN_bm = PIN_bm;

    Servo_CalculatePeriod(instance);
    Servo_CalculateDirections(instance);
    return instance;
}

void Servo_delete(Servo *self) 
{
    free(self);
}

void Servo_CalculatePeriod(Servo *self)
{    
    self->DIR.PERIOD = (uint16_t)(((unsigned long)(self->CLK_SPEED)/((uint8_t)(self->PRESCALER) * (uint8_t)(self->MODEL.FREQ))) - 1);
}

void Servo_CalculateDirections(Servo *self)
{
    self->DIR.MID = (uint16_t)(((float)(self->MODEL.MID_MS) * (unsigned long)(self->CLK_SPEED))/(uint8_t)(self->PRESCALER));
    self->DIR.LEFT = (uint16_t)(((float)(self->MODEL.HIGH_MS) * (unsigned long)(self->CLK_SPEED))/(uint8_t)(self->PRESCALER));
    self->DIR.RIGHT = (uint16_t)(((float)(self->MODEL.LOW_MS) * (unsigned long)(self->CLK_SPEED))/(uint8_t)(self->PRESCALER));
}

/**
 * PWM is used to control the Servo. 
 * 
 * Singleslope waveform generator. Using the CLK_PER and a prescaler on TCA of 16:
 * 
 * Period (TCA0.SINGLE.PER) = (33333333/16(50)) - 1 = 4166
 * 
 * Value for the compare buffer. 
 * 
 * Left = 1ms   Mid = 1.5ms   Right = 2ms
 * 0.0015(3333333)/16 = ~312
 * 0.001(3333333)/16 = ~208
 * 0.002(3333333)/16 = ~417
 * 
 * Also going to put the trigger for the ultrasonic sensor on CMP1 at max time 
 * which should be 20ms. Then we can use TCB to turn it off and RTC to count time
 * till echo. 
 * 
 */
void Servo_InitPWM(Servo *self)
{
    *self->PWM_CONFIG->TCAROUTE |= self->PWM_CONFIG->PORTMUX_gc;
    *self->PWM_CONFIG->CTRLB |= self->PWM_CONFIG->WGMODE_gc | self->PWM_CONFIG->CPMEN_bm;
    *self->PWM_CONFIG->CTRLA |= self->PWM_CONFIG->CLKDIV_gc;

    //Enable
    *self->PWM_CONFIG->CTRLA |= self->PWM_CONFIG->RUNMODE_bm;

    //Set the buffers
    *self->PWM_CONFIG->PER = self->DIR.PERIOD;
    *self->PWM_CONFIG->CMP0BUFF = self->DIR.MID;
}
void Servo_InitPins(Servo *self)
{
    *self->PORT |= self->PIN_bm;
}



/**
 * This needs a better method. Currently its tied to a specific MCU set. 
 * @param self
 * @param pos
 */
void Servo_Move(Servo *self, uint16_t pos)
{
    self->DIR.CURRENT = pos;
    *self->PWM_CONFIG->CMP0BUFF = pos;
}