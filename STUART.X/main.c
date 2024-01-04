#define F_CPU 3333333UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Servo_SG90.h"

void MainClkCtrl(void);
void InitPins(void);
void InitPWM(void);
void InitRTC();
void InitTCB();
//void MoveServo(uint16_t);

volatile Servo* servo = NULL;
volatile uint8_t waitingForEcho = 0;
volatile uint16_t endTime = 0;
volatile float distance = 0;
volatile float speedOfSound = 344.5e-3;

int main(void) 
{
    sei();
    
    servo = Servo_new(F_CPU, 16); //16 as we want the TCA (PWM) Prescaler
    
    MainClkCtrl();
    InitPins();
    InitRTC();
    InitTCB();
    InitPWM();
    
    while (1) 
    {
//        MoveServo(312);
//        _delay_ms(1000);
        
//        MoveServo(208);
//        _delay_ms(1000);
//        
//        MoveServo(312);
//        _delay_ms(1000);
//        
//        MoveServo(417);
//        _delay_ms(1000);
        
    }

    return (0);
}

/**
 * This is the default setup from the FUSE settings on restart. (20MHz/6) = 3333333
 * 3.33MHz. 
 * 
 * We are explicitly setting this as this is what the CLK_PER is based on. CLK_PER is 
 * a vital part of creating the TCA0 PWM.  
 */
void MainClkCtrl(void) 
{
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLA, CLKCTRL_CLKSEL_OSC20M_gc | CLKCTRL_CLKOUT_bm);
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PDIV_6X_gc | CLKCTRL_PEN_bm);
}

/**
 * Setting PORTB Pin 0 to output. This pin is controlled by TCA0 on WO0 
 * 
 * Setting PORTB Pin 7 to output. This is for testing. 
 * 
 * Setting PA1 to U/S Trigger (OUTPUT)
 * Setting PA2 to U/S Echo (INPUT)
 * 
 * Added interrupt to Echo pin by setting ISC bits on PINnCTRL
 */
void InitPins(void)
{
    PORTB.DIR = (1 << PIN0_bp); 
    PORTB.DIR |= (1 << PIN7_bp);
    
    PORTA.DIR = (1 << PIN1_bp);
    PORTA.DIR |= PIN2_bp;
    
    PORTA.PIN2CTRL = PORT_ISC_FALLING_gc;
}

/**
 * PWM is used to control the Servo. 
 * 
 * Singleslope waveform generator. Using the CLK_PER and a prescaler on TCA of 16:
 * 
 * Period (TCA0.SINGLE.PER) = (33333333/16(50)) - 1 = 4166
 * 
 * Value for the compare buffer. 
 * 
 * Left = 1ms   Mid = 1.5ms   Right = 2ms
 * 0.0015(3333333)/16 = ~312
 * 0.001(3333333)/16 = ~208
 * 0.002(3333333)/16 = ~417
 * 
 * Also going to put the trigger for the ultrasonic sensor on CMP1 at max time 
 * which should be 20ms. Then we can use TCB to turn it off and RTC to count time
 * till echo. 
 * 
 */
void InitPWM(void)
{
    PORTMUX.TCAROUTEA |= PORTMUX_TCA00_DEFAULT_gc; 
    
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_CMP1EN_bm;
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc;
    
    //We want the CMP1 channel to trigger an interrupt
    TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP1_bm;
    
    // Enable the timer
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
    
    TCA0.SINGLE.PER = servo->DIR.PERIOD;
    TCA0.SINGLE.CMP0BUF = servo->DIR.MID;
    TCA0.SINGLE.CMP1BUF = servo->DIR.PERIOD; //This is the max.
}

/**
 * The realtime clock is to be configured to control when the ultrasonic sensor will 
 * pulse. 
 * 
 * We don't want to trigger the ultrasonic every cycle. It will cause delays with the 
 * interrupt code execution so every periodically will do. 
 */
void InitRTC(void) 
{
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
    while(RTC.STATUS & RTC_PERBUSY_bm);
    RTC.PER = 32768;
    
    RTC.INTCTRL = RTC_OVF_bm;
    
    while(RTC.CTRLA & RTC_PERBUSY_bm);
    RTC.CTRLA = RTC_RUNSTDBY_bm | RTC_PRESCALER_DIV1_gc;
}

/*
 * * Using the Compare buffer 1: we need 10us trigger for the ultrasonic sensor. 
 * This should be on an interrupt. 
 * Timer ticks = (3333333/2)0.00001
 * ~17
 */
void InitTCB(void) 
{
    TCB0.CCMP = 17;
    TCB0.CNT = 0xFFFF;
    TCB0.CTRLA |= TCB_CLKSEL_DIV2_gc; 
    TCB0.CTRLB |= TCB_CNTMODE_INT_gc;
    
    TCB0.INTCTRL |= TCB_CAPT_bm;
}

 
/**
 * Updates the Compare Value on TCA0 Duty Cycles which controls the direction the
 * servo points. Writing to the buffer so the values are put to the PER registry on the 
 * next cycle. 
 * 
 * @param pos - Integer value to write to TCA0.CMP
 */
//void MoveServo(uint16_t pos)
//{
//    TCA0.SINGLE.CMP0 = pos;
//}

/**
 * Interrupt for CMP1 on TCA0
 */
ISR(TCA0_CMP1_vect)
{
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP1_bm;
    //Turn on the Trigger pin.
    PORTA.OUT |= PIN1_bm;
    //Turn on TCB so we can count to 10us
    TCB0.CTRLA |= TCB_ENABLE_bm;
}

/**
 * Interrupt for OVF on TCB - This should trigger the Counter to turn off. 
 */
ISR(TCB0_INT_vect)
{
    if(TCB0.INTFLAGS & TCB_CAPT_bm)
    {
        TCB0.INTFLAGS = TCB_CAPT_bm;    
        
        if (!waitingForEcho)
        {
            PORTA.OUTCLR |= PIN1_bm;
            TCB0.CTRLA &= ~(1 << TCB_ENABLE_bp);
            waitingForEcho = 1;
            //Init the RTC
            RTC.CTRLA |= RTC_RTCEN_bm;
        }
    }
}

/**
 * Interrupt on PORTA for Echo activity
 */
ISR(PORTA_PORT_vect)
{
    if (PORTA.INTFLAGS & PORT_INT_2_bm)
    {
        if (waitingForEcho && !(PORTA.IN & (1 << PIN2_bp)))
        {
            // Pin is low, record the end time and calculate distance
            endTime = RTC.CNT; // Capture the time elapsed since the trigger was stopped
            //convert the endtime from cycles to ?ms?
            endTime = endTime/32.768;
            if (endTime)
            {
                // Calculate distance in millimeters (distance = speed * time / 2)
                distance = ((float)(speedOfSound) * endTime) / 2.0; // Dividing by 20 because speed is scaled by 10 and dividing by 2 for echo
//                if (distance >= (float)(5.0))
//                {
//                    MoveServo(312);
//                }
                if (distance < (float)(2.5)) // Distance threshold in millimeters
                {
                    // Do something for objects within 20 meters (20000 millimeters)
                    //Get current pos of servo
//                    volatile uint16_t cmp0 = TCA0.SINGLE.CMP0;
//                    MoveServo(servo->DIR.LEFT);
                    Servo_Move(servo, servo->DIR.LEFT);
                } 
                
                
                //Disable the RTC
                RTC.CTRLA &= ~(1 << RTC_RTCEN_bm);
                RTC.CNT = 0;

            }   
            
            // Reset waitingForEcho for the next cycle
            waitingForEcho = 0;

        }
        // Clear the interrupt flag
        PORTA.INTFLAGS = PORT_INT_2_bm;
    }
}

/**
 * RTC Interrupt to reenable the ability to trigger ultrasonic sensor
 */
ISR(RTC_CNT_vect)
{
    RTC.INTFLAGS = RTC_INTFLAGS; //Should reset the flags
//    MoveServo(312);
    Servo_Move(servo, servo->DIR.MID);
    waitingForEcho = 0; //Reenable the ability to trigger the ultrasonic sensor
    //Disable the RTC
    RTC.CTRLA &= ~(1 << RTC_RTCEN_bm);
    RTC.CNT = 0;
}


