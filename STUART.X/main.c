#define F_CPU 3333333UL

#include <avr/io.h>
#ifndef __AVR_ATtiny1627__
    #include <avr/iotn1627.h>
#endif
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lib/utils/UART.h"
#include "lib/components/Ultrasonic_HCSR04.h"

void PinSetup(void);

UART_Config *uart_config = NULL;
UART *uart = NULL;

Ultrasonic_TC_Config *ultrasonic_tcConfig = NULL;
Ultrasonic_RTC_Config *ultrasonic_rtcConfig = NULL;
Ultrasonic_Config *ultrasonic_config = NULL;
Ultrasonic *ultrasonic = NULL;

void InitClasses(void);
// void InitTCB(void);
// void InitRTC(void);

//Functions to get the ultrasonic sensor calibration working. 
// void Trigger(void);
// void InitRTC(void);
// void StartRTC(void);
// uint16_t StopRTC(void);

volatile uint8_t tcbIntInProgress = 0;

int main()
{
    sei();

    InitClasses();

    UART_SetBaudRate(uart);
    UART_EnableTR(uart);

    PinSetup();
    Ultrasonic_InitPins(ultrasonic);
    
    Ultrasonic_InitRTC(ultrasonic);
    Ultrasonic_InitTC(ultrasonic);


    while(1)
    {
        
    }

    return 0;
}

void InitClasses(void)
{
    //UART
    uart_config = UART_Config_new(  
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
    uart = UART_new(F_CPU, 9600, uart_config);

    //Ultrasonic
    //Setting CNT_VALUE to max. 
    ultrasonic_tcConfig = Ultrasonic_TC_Config_new(
        &TCB0.CCMP,
        17,
        &TCB0.CNT,
        17,
        &TCB0.CTRLA,
        TCB_CLKSEL_DIV2_gc,
        &TCB0.CTRLB,
        TCB_CNTMODE_INT_gc,
        &TCB0.INTCTRL,
        TCB_CAPT_bm,
        TCB_ENABLE_bm
    );
    ultrasonic_rtcConfig = Ultrasonic_RTC_Config_new(
        &RTC.CLKSEL,
        RTC_CLKSEL_INT32K_gc,
        &RTC.STATUS,
        &RTC.CTRLA,
        RTC_PERBUSY_bm,
        &RTC.PER,
        32768,
        &RTC.INTCTRL,
        RTC_OVF_bm,
        RTC_RTCEN_bm,
        RTC_PRESCALER_DIV1_gc,
        &RTC.CNT
    );
    ultrasonic_config = Ultrasonic_Config_new(
        &PORTA.DIR,
        &PORTA.DIR,
        &PORTA.OUT,
        &PORTA.IN,
        PIN1_bm,
        PIN2_bm,
        ultrasonic_tcConfig,
        ultrasonic_rtcConfig,
        3333333UL
    );
    ultrasonic = Ultrasonic_new(ultrasonic_config, 30.5176, 0.0343);
}

void PinSetup(void)
{
    PORTB.DIR |= PIN7_bm;
}

ISR(TCB0_INT_vect)
{
    Ultrasonic_DisableTC(ultrasonic);
    if(TCB0.INTFLAGS & TCB_CAPT_bm)
    {
        tcbIntInProgress = 1;
        TCB0.INTFLAGS = TCB_CAPT_bm;

        //Turn off the trigger pin
        Ultrasonic_TriggerOff(ultrasonic);
        Ultrasonic_Measure(ultrasonic);

        Ultrasonic_CalculateDistance(ultrasonic);

        UART_Transmit("ISR::TCB::Distance: ");
        UART_TransmitFloat(ultrasonic->distance);
        UART_Transmit("cm\n");
        tcbIntInProgress = 0;
    }

    UART_Transmit("\n");
}

ISR(RTC_CNT_vect)
{
    UART_Transmit("ISR::RTC::Clear Flags\n");
    RTC.INTFLAGS = RTC_INTFLAGS; //Should reset the flags
    
    //Toggle the LED
    PORTB.OUTTGL |= PIN7_bm;

    //Enable TCB
    if (!tcbIntInProgress)
    {
        Ultrasonic_ResetTime(ultrasonic);
        Ultrasonic_TriggerOn(ultrasonic);
        Ultrasonic_EnableTC(ultrasonic);
    }
}