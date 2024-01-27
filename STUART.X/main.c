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

//Functions to get the ultrasonic sensor calibration working. 
void Trigger(void);
void InitRTC(void);
void StartRTC(void);
uint16_t StopRTC(void);

int main()
{
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

    InitRTC();
    _delay_ms(1000);
    StartRTC();


    while(1)
    {
        PORTB.OUTTGL |= PIN7_bm;
        _delay_ms(1000);
        PORTB.OUTTGL |= PIN7_bm;
        _delay_ms(1000);
        // Transmit("Loop end\n");

        Trigger();
        //Wait for the echo to go high
        while(!(PORTA.IN & PIN2_bm));
        //Start the timer
        uint16_t startTime = RTC.CNT;
        //Wait for the echo to go low
        while(PORTA.IN & PIN2_bm);
        // Stop the timer
        uint16_t endTime = RTC.CNT;

        uint16_t time = endTime - startTime;
        float timeInMicroseconds = (float)time * 30.5176;

        Transmit("startTime: ");
        TransmitUint16(startTime);
        Transmit("\n");
        Transmit("endTime: ");
        TransmitUint16(endTime);
        Transmit("\n");
        Transmit("Time: ");
        TransmitUint16(time);
        Transmit("\n");
        Transmit("Time in Microseconds: ");
        TransmitUint16(timeInMicroseconds);
        Transmit("\n");
        
        //Convert the time to distance
        //The speed of sound is 343m/s
        //The time is in uS
        //So the distance is time * 34300 / 2
        //The /2 is because the sound has to travel to the object and back

        /*
            Units Conversion: 
            The speed of sound (343 m/s) needs to be converted into centimeters per microsecond (cm/μs) for your calculation. 
            There are 100 centimeters in a meter and 1,000,000 microseconds in a second. 
            Therefore, the speed of sound in cm/μs is 
                343 m/s×100 cm/m1,000,000 μs/s=0.0343 cm/μs1,000,000 μs/s343 m/s×100 cm/m​=0.0343 cm/μs.
        */
        float distance = (float)timeInMicroseconds * (float)0.0343 / (float)2.0;
        Transmit("Distance: ");
        TransmitFloat(distance);
        Transmit("cm\n");

        //We should wait at least 10uS for the next cycle
        _delay_us(10);
        Transmit("\n");
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

void Trigger(void)
{
    //Turn on
    PORTA.OUT |= PIN1_bm;
    //wait for 10uS
    _delay_us(10);
    //Turn off
    PORTA.OUT &= ~PIN1_bm;
}

void InitRTC(void) 
{
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
    while(RTC.STATUS & RTC_PERBUSY_bm);
    RTC.PER = 32768;
    
    // RTC.INTCTRL = RTC_OVF_bm;
    
    while(RTC.CTRLA & RTC_PERBUSY_bm);
    RTC.CTRLA = RTC_RUNSTDBY_bm | RTC_PRESCALER_DIV1_gc;
}

void StartRTC(void)
{
    RTC.CTRLA |= RTC_RTCEN_bm;
}

uint16_t StopRTC(void)
{
    RTC.CTRLA &= ~RTC_RTCEN_bm;
    return RTC.CNT;
}