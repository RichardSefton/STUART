#define F_CPU 3333333UL

#include <avr/io.h>
#ifndef __AVR_ATtiny1627__
    #include <avr/iotn1627.h>
#endif
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lib/utils/UART.h"
#include "lib/components/Ultrasonic_HCSR04.h"
#include "lib/components/Servo_SG90.h"

void PinSetup(void);

UART_Config *uart_config = NULL;
UART *uart = NULL;

Ultrasonic_TC_Config *ultrasonic_tcConfig = NULL;
Ultrasonic_RTC_Config *ultrasonic_rtcConfig = NULL;
Ultrasonic_Config *ultrasonic_config = NULL;
Ultrasonic *ultrasonic = NULL;

ServoPWM_Config *servo_pwmConfig = NULL;
Servo *servo = NULL;

typedef enum
{
    MOVE,
    LOOK,
    DECIDE,
} Mode;

Mode mode = MOVE;

void InitClasses(void);

volatile uint8_t tcbIntInProgress = 0;
volatile uint8_t count = 0;

int main()
{
    sei();

    InitClasses();
    PinSetup();

    UART_SetBaudRate(uart);
    UART_EnableTR(uart);

    Servo_InitPins(servo);
    Servo_InitPWM(servo);

    Ultrasonic_InitPins(ultrasonic);
    Ultrasonic_InitRTC(ultrasonic);
    Ultrasonic_InitTC(ultrasonic);

    UART_Transmit("Setup complete\n");

    while(1)
    {
        // UART_Transmit("Hello World\n");        
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
        0xFFFF,
        &TCB0.CNT,
        0xFFFF,
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

    //Servo
    servo_pwmConfig = Servo_PWM_new(
        &PORTMUX.TCAROUTEA,
        &TCA0.SINGLE.CTRLA,
        &TCA0.SINGLE.CTRLB,
        PORTMUX_TCA00_DEFAULT_gc,
        TCA_SINGLE_WGMODE_SINGLESLOPE_gc,
        TCA_SINGLE_CMP0EN_bm,
        TCA_SINGLE_CLKSEL_DIV16_gc,
        TCA_SINGLE_ENABLE_bm,
        &TCA0.SINGLE.PER,
        &TCA0.SINGLE.CMP0BUF
    );
    servo = Servo_new(
        F_CPU, 
        16, 
        servo_pwmConfig,
        &PORTB.DIR,
        PIN0_bm 
    ); //16 as we want the TCA (PWM) Prescaler
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

        if (mode == MOVE)
        {
            if (ultrasonic->distance < 5.0)
            {
                mode = LOOK;
            }
        }
        if (mode == LOOK)
        {
            if (servo->DIR.CURRENT == servo->DIR.LEFT)
            {
                ultrasonic->DISTANCES.LEFT = ultrasonic->distance;
            }
            else if (servo->DIR.CURRENT == servo->DIR.RIGHT)
            {
                ultrasonic->DISTANCES.RIGHT = ultrasonic->distance;
            }
        }

        tcbIntInProgress = 0;
        UART_Transmit("\n");
    }
}

ISR(RTC_CNT_vect)
{
    UART_Transmit("ISR::RTC::Clear Flags\n");
    RTC.INTFLAGS = RTC_INTFLAGS; //Should reset the flags
    
    //Toggle the LED
    PORTB.OUTTGL |= PIN7_bm;


    //Enable TCB
    if (!tcbIntInProgress && mode == MOVE)
    {
        UART_Transmit("ISR::RTC::Move\n");
        Servo_Move(servo, servo->DIR.MID);
        Ultrasonic_ResetTime(ultrasonic);
        Ultrasonic_TriggerOn(ultrasonic);
        Ultrasonic_EnableTC(ultrasonic);
    }

    if (!tcbIntInProgress && mode == LOOK)
    {
        UART_Transmit("ISR::RTC::Look\n");
        if (servo->DIR.CURRENT == servo->DIR.RIGHT)
        {
            mode = DECIDE;
        } 
        else
        {
            if (servo->DIR.CURRENT == servo->DIR.MID)
            {
                Servo_Move(servo, servo->DIR.LEFT);
                //I Know its blocking, but we need to give the servo time to move
                _delay_ms(500);
            }
            else if (servo->DIR.CURRENT == servo->DIR.LEFT)
            {
                Servo_Move(servo, servo->DIR.RIGHT);
                _delay_ms(500);
            }
            Ultrasonic_ResetTime(ultrasonic);
            Ultrasonic_TriggerOn(ultrasonic);
            Ultrasonic_EnableTC(ultrasonic);
        }
    }

    if (mode == DECIDE)
    {
        if (ultrasonic->DISTANCES.LEFT > ultrasonic->DISTANCES.RIGHT)
        {
            Servo_Move(servo, servo->DIR.LEFT);
        }
        else
        {
            Servo_Move(servo, servo->DIR.RIGHT);
        }
        
        if (count <= 5)
        {
            count++;
        }
        else
        {
            count = 0;
            mode = MOVE;
        }
        UART_Transmit("\n");
    }
}