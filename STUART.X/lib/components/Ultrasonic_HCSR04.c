#include "Ultrasonic_HCSR04.h"

Ultrasonic* Ultrasonic_new(uint8_t minEchoTime, float speedOfSound)
{
    Ultrasonic *instance = (Ultrasonic*)malloc(sizeof(Ultrasonic));
    if (instance == NULL)
    {
        while(1);
    }
    
    instance->startTime = 0;
    instance->endTime = 0;
    instance->waitingForEcho = 0;
    instance->distance = 255; //Init to max value. 0xFF is also valid. 
    
    instance->config.minEchoTime = minEchoTime;
    instance->config.speedOfSound = speedOfSound;
    
    instance->mode = DRIVE;
    instance->dir.LEFT = 0.0;
    instance->dir.RIGHT = 0.0;
    
    return instance;
}

void Ultrasonic_delete(Ultrasonic *self)
{
    free(self);
}

void Ultrasonic_reset(Ultrasonic *self)
{
    self->startTime = 0;
    self->endTime = 0;
    self->waitingForEcho = 0;
    self->distance = 255;
    
    self->mode = DRIVE;
    self->dir.LEFT = 0.0;
    self->dir.RIGHT = 0.0;
}

void Ultrasonic_beginCount(Ultrasonic *self)
{
    self->waitingForEcho = 1;
    self->startTime = 0;
    self->endTime = 0;
}

void Ultrasonic_updateTime(Ultrasonic *self, uint16_t time)
{
    self->endTime = time;
}

float Ultrasonic_calcDistance(Ultrasonic *self)
{
    self->distance = ((float)(self->config.speedOfSound * self->endTime)) / 2.0;
    return self->distance;            
}

void Ultrasonic_changeMode(Ultrasonic *self, enum_Mode mode)
{
    self->mode = mode;
}

void Ultrasonic_updateLeft(Ultrasonic *self, float dist)
{
    self->dir.LEFT = dist;
}

void Ultrasonic_updateRight(Ultrasonic *self, float dist)
{
    self->dir.RIGHT = dist;
}