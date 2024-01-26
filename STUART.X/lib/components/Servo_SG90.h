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
    SG90 MODEL;
    unsigned long CLK_SPEED;
    uint8_t PRESCALER;
    Direction DIR;
} Servo;

Servo* Servo_new(unsigned long, uint8_t);
void Servo_delete(Servo*);
void Servo_CalculatePeriod(Servo*);
void Servo_CalculateDirections(Servo*);
void Servo_Move(Servo*, uint16_t);