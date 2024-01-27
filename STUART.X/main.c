#define F_CPU 3333333UL

#include <avr/io.h>
#ifndef __AVR_ATtiny1627__
    #include <avr/iotn1627.h>
#endif
#include <util/delay.h>
#include "lib/utils/UART.h"

void PinSetup(void);

Config *config = NULL;
UART *uart = NULL;

int main()
{
    /**
     *     
    register8_t *RXDATA_L
    */
    config = Config_new(
        &USART0.BAUD,
        &PORTB.DIR,
        PIN2_bm,
        &PORTB.DIR,
        PIN3_bm,
        &USART0.CTRLA,
        &USART0.CTRLB,
        USART_RXCIE_bm,
        USART_RXEN_bm,
        USART_TXEN_bm,
        &USART0.RXDATAL
    );
    uart = UART_new(F_CPU, 9600, config);

    SetBaudRate(uart);

    EnableTR(uart);

    PinSetup();

    while(1)
    {
        PORTB.OUTTGL |= PIN7_bm;
        _delay_ms(1000);
        PORTB.OUTTGL |= PIN7_bm;
        _delay_ms(1000);
        Transmit("Loop end\n");
    }

    return 0;
}

void PinSetup(void)
{
    PORTB.DIR |= PIN7_bm;

    //Ultrasonic sensor is on PA1 and PA2
    //PA1 is the trigger
    //PA2 is the echo
    PORTA.DIR |= PIN1_bm;
    PORTA.DIR &= ~PIN2_bm;

}