#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#ifndef __AVR_ATtiny1627__
    #include <avr/iotn1627.h>
#endif
#include <stdlib.h>

#ifndef __STATUS__
    #define __STATUS__ USART0.STATUS
#endif
#ifndef __TXDATA_L__
    #define __TXDATA_L__ USART0.TXDATAL
#endif
#ifndef __INTFLAG_bm__
    #define __INTFLAG_bm__ USART_DREIF_bm
#endif

typedef struct 
{
    register16_t* BAUD;
    register8_t* RX_PORT;
    uint8_t RX_PIN_bm;
    register8_t* TX_PORT;
    uint8_t TX_PIN_bm;
    register8_t* CTRLA;
    register8_t* CTRLB;
    uint8_t INT_bm;
    uint8_t INT_FLAG_bm;
    uint8_t RXEN_bm;
    uint8_t TXEN_bm;
    register8_t* STATUS;
    register8_t* TXDATA_L;
    register8_t* RXDATA_L;
} Config;

typedef struct
{
    Config* CONFIG;
    unsigned long CLK_FREQ;
    uint16_t BAUD_RATE;
} UART;

#define USART0_BAUD_RATE(BAUD_RATE, CLK_FREQ) ((float)(((float)CLK_FREQ * 64) / (16 * (float)BAUD_RATE)) + 0.5)
#define USART_LENGTH 8

Config* Config_new(
    register16_t*,
    register8_t*, uint8_t, 
    register8_t*, uint8_t, 
    register8_t*, register8_t*, 
    uint8_t, 
    uint8_t, uint8_t,
    register8_t*
);
void Config_delete(Config*);

UART* UART_new(unsigned long, uint16_t, Config*);
void UART_delete(UART*);

void SetBaudRate(UART*);
void EnableTR(UART*);
static void SendChar(char c);
static int PrintChar(char c, FILE *stream);
uint8_t Read(UART*);
void Transmit(char str[]);
void TransmitUint8(UART*, uint8_t value);
void TransmitUint16(UART*, uint16_t value);