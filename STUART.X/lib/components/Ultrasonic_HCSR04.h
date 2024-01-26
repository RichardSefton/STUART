#include <stdlib.h>
#include <stdint.h>
#ifndef __AVR_ATtiny1627__
    #include <avr/iotn1627.h>
#endif

/*
 * For now, this library won't take temperature into consideration. 
 * 
 * speedOfSound is calculated at 22.5degC
 */
typedef struct 
{
    uint8_t minEchoTime;
    float speedOfSound;
} HC_SR04;

typedef enum
{
    DRIVE = 0,
    GATHER = 1,
    DECIDE = 2
} enum_Mode;

typedef struct
{
    float LEFT;
    float RIGHT;
} Directions;

typedef struct 
{
    uint8_t waitingForEcho;
    uint8_t startTime;
    uint16_t endTime;
    float distance;
    HC_SR04 config;
    enum_Mode mode;
    Directions dir;
} Ultrasonic;

Ultrasonic* Ultrasonic_new(uint8_t, float);
void Ultrasonic_delete(Ultrasonic*);
void Ultrasonic_reset(Ultrasonic*);
void Ultrasonic_beginCount(Ultrasonic*);
void Ultrasonic_updateTime(Ultrasonic*, uint16_t);
float Ultrasonic_calcDistance(Ultrasonic*);

void Ultrasonic_changeMode(Ultrasonic*, enum_Mode);

void Ultrasonic_updateLeft(Ultrasonic*, float);
void Ultrasonic_updateRight(Ultrasonic*, float);