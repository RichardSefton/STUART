#include "Servo_SG90.h"

Servo* Servo_new(unsigned long CLK, uint8_t PRESCALER) 
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
 * This needs a better method. Currently its tied to a specific MCU set. 
 * @param self
 * @param pos
 */
void Servo_Move(Servo *self, uint16_t pos)
{
    self->DIR.CURRENT = pos;
    TCA0.SINGLE.CMP0 = pos;
}