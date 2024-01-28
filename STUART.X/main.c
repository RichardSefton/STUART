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
void InitTCB(void);

//Functions to get the ultrasonic sensor calibration working. 
void Trigger(void);
void InitRTC(void);
void StartRTC(void);
uint16_t StopRTC(void);

int main()
{
    sei();

    InitClasses();
    InitTCB();

    UART_SetBaudRate(uart);
    UART_EnableTR(uart);

    PinSetup();
    // Ultrasonic_InitPins(ultrasonic);
    
    Ultrasonic_InitRTC(ultrasonic);
    // Ultrasonic_InitTC(ultrasonic);


    while(1)
    {
        // Trigger();
        // //Wait for the echo to go high
        // while(!(PORTA.IN & PIN2_bm));
        // //Start the timer
        // uint16_t startTime = RTC.CNT;
        // //Wait for the echo to go low
        // while(PORTA.IN & PIN2_bm);
        // // Stop the timer
        // uint16_t endTime = RTC.CNT;

        // uint16_t time = endTime - startTime;
        // float timeInMicroseconds = (float)time * 30.5176;

        // Transmit("startTime: ");
        // TransmitUint16(startTime);
        // Transmit("\n");
        // Transmit("endTime: ");
        // TransmitUint16(endTime);
        // Transmit("\n");
        // Transmit("Time: ");
        // TransmitUint16(time);
        // Transmit("\n");
        // Transmit("Time in Microseconds: ");
        // TransmitUint16(timeInMicroseconds);
        // Transmit("\n");
        
        // //Convert the time to distance
        // //The speed of sound is 343m/s
        // //The time is in uS
        // //So the distance is time * 34300 / 2
        // //The /2 is because the sound has to travel to the object and back

        // /*
        //     Units Conversion: 
        //     The speed of sound (343 m/s) needs to be converted into centimeters per microsecond (cm/μs) for your calculation. 
        //     There are 100 centimeters in a meter and 1,000,000 microseconds in a second. 
        //     Therefore, the speed of sound in cm/μs is 
        //         343 m/s×100 cm/m1,000,000 μs/s=0.0343 cm/μs1,000,000 μs/s343 m/s×100 cm/m​=0.0343 cm/μs.
        // */
        // float distance = (float)timeInMicroseconds * (float)0.0343 / (float)2.0;
        // Transmit("Distance: ");
        // TransmitFloat(distance);
        // Transmit("cm\n");

        // //We should wait at least 10uS for the next cycle
        // _delay_us(10);
        // Transmit("\n");
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
        PIN1_bm,
        PIN2_bm,
        ultrasonic_tcConfig,
        ultrasonic_rtcConfig,
        3333333UL
    );
    ultrasonic = Ultrasonic_new(ultrasonic_config);
}

 /* * Using the Compare buffer 1: we need 10us trigger for the ultrasonic sensor. 
 * This should be on an interrupt. 
 * Timer ticks = (3333333/2)0.00001
 * ~17
 */
void InitTCB(void) 
{
    TCB0.CCMP = 17;
    TCB0.CNT = 17;
    TCB0.CTRLA |= TCB_CLKSEL_DIV2_gc; 
    TCB0.CTRLB |= TCB_CNTMODE_INT_gc;
    
    TCB0.INTCTRL |= TCB_CAPT_bm;
}

void PinSetup(void)
{
    PORTB.DIR |= PIN7_bm;

    PORTA.DIR |= PIN1_bm;
    PORTA.DIR &= ~PIN2_bm;

    //Set the trigger pin to low
    // PORTA.OUTCLR |= PIN1_bm;
}

ISR(TCB0_INT_vect)
{
    UART_Transmit("ISR::TCB::In Interrupt\n");
    
    if(TCB0.INTFLAGS & TCB_CAPT_bm)
    {
        UART_Transmit("ISR::TCB::Capture Interrupt, Clearing flags\n");
        TCB0.INTFLAGS = TCB_CAPT_bm;
        //Disable the counter and reset the COUNT to 0
        TCB0.CTRLA &= ~TCB_ENABLE_bm;
        TCB0.CNT = 0;   

        //Turn off the trigger pin
        // Ultrasonic_TriggerOff(ultrasonic);
        // PORTA.OUT &= ~PIN1_bm;
        UART_Transmit("ISR::TCB::EndTime: ");
        UART_TransmitUint16(RTC.CNT);
        UART_Transmit("\n");
        PORTA.OUTTGL |= PIN1_bm;

        // Ultrasonic_Measure(ultrasonic);
        //Wait for PORTA.PIN2 to go high
        // UART_Transmit("ISR::TCB::Waiting for PORTA.PIN2 to go high\n");
        // while(!(PORTA.IN & PIN2_bm));
        UART_Transmit("ISR::TCB::PIN2: ");
        UART_TransmitUint8(!(PORTA.IN & PIN2_bm));
        UART_Transmit("\n");
        // // //Start the timer
        // // Ultrasonic_SetBeginTime(ultrasonic);
        
        // //Wait for PORTA.PIN2 to go low
        // UART_Transmit("ISR::TCB::Waiting for PORTA.PIN2 to go low\n");
        // while(PORTA.IN & PIN2_bm);
        // //Stop the timer
        // Ultrasonic_SetEndTime(ultrasonic);

        // Ultrasonic_CalculateDistance(ultrasonic);

        UART_Transmit("ISR::TCB::Distance: ");
        UART_TransmitFloat(ultrasonic->distance);
        UART_Transmit("cm\n");
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
    // Ultrasonic_TriggerOn(ultrasonic);
    PORTA.OUTTGL |= PIN1_bm;

    // Ultrasonic_EnableTC(ultrasonic);
    UART_Transmit("ISR::RTC::Enabled TCB::StartTime: ");
    UART_TransmitUint16(RTC.CNT);
    UART_Transmit("\n");
    TCB0.CTRLA |= TCB_ENABLE_bm;
}

// void Trigger(void)
// {
//     //Turn on
//     PORTA.OUT |= PIN1_bm;
//     //wait for 10uS
//     _delay_us(10);
//     //Turn off
//     PORTA.OUT &= ~PIN1_bm;
// }

// void InitRTC(void) 
// {
//     RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
//     while(RTC.STATUS & RTC_PERBUSY_bm);
//     RTC.PER = 32768;
    
//     // RTC.INTCTRL = RTC_OVF_bm;
    
//     while(RTC.CTRLA & RTC_PERBUSY_bm);
//     RTC.CTRLA = RTC_RUNSTDBY_bm | RTC_PRESCALER_DIV1_gc;
// }

// void StartRTC(void)
// {
//     RTC.CTRLA |= RTC_RTCEN_bm;
// }

// uint16_t StopRTC(void)
// {
//     RTC.CTRLA &= ~RTC_RTCEN_bm;
//     return RTC.CNT;
// }