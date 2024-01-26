#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#ifndef __AVR_ATtiny1627__
    #include <avr/iotn1627.h>
#endif

#define F_CPU 3333333UL
#define BAUD_RATE 9600

#define USART0_BAUD_RATE(BAUD_RATE, CLK_FREQ) ((float)(((float)CLK_FREQ * 64) / (16 * (float)BAUD_RATE)) + 0.5)
#define USART_LENGTH 8

void SetBaudRate(void);
void EnableTR(void);
static void SendChar(char c);
static int PrintChar(char c, FILE *stream);
uint8_t Read(void);
void Transmit(char str[]);
void TransmitUint8(uint8_t value);
void TransmitUint16(uint16_t value);