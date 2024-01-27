#include "UART.h"

static FILE USART_stream = FDEV_SETUP_STREAM(PrintChar, NULL, _FDEV_SETUP_WRITE);

Config* Config_new(
    register16_t *BAUD,
    register8_t *RX_PORT, uint8_t RX_PIN, 
    register8_t *TX_PORT, uint8_t TX_PIN, 
    register8_t *CTRLA, register8_t *CTRLB, 
    uint8_t INT_bm, 
    uint8_t RXEN_bm, uint8_t TXEN_bm,
    register8_t *RXDATA_L
)
{
    Config *instance = (Config*)malloc(sizeof(Config));

    if (instance == NULL)
    {
        while(1);
    }

    instance->BAUD = BAUD;
    instance->RX_PORT = RX_PORT;
    instance->RX_PIN_bm = RX_PIN;
    instance->TX_PORT = TX_PORT;
    instance->TX_PIN_bm = TX_PIN;
    instance->CTRLA = CTRLA;
    instance->CTRLB = CTRLB;
    instance->INT_bm = INT_bm;
    instance->RXEN_bm = RXEN_bm;
    instance->TXEN_bm = TXEN_bm;
    *instance->STATUS = __STATUS__;
    instance->INT_FLAG_bm = __INTFLAG_bm__;
    *instance->TXDATA_L = __TXDATA_L__;
    instance->RXDATA_L = RXDATA_L;

    return instance;
}

void Config_delete(Config *self)
{
    free(self);
}

UART* UART_new(unsigned long CLK_FREQ, uint16_t BAUD, Config *config)
{
    UART *instance = (UART*)malloc(sizeof(UART));

    if (instance == NULL)
    {
        while(1);
    }

    instance->CLK_FREQ = CLK_FREQ;
    instance->BAUD_RATE = BAUD;
    instance->CONFIG = config;

    return instance;
}

void UART_delete(UART *self)
{
    free(self);
}

void SetBaudRate(UART *self)
{
    uint16_t baud_setting = (uint16_t)(USART0_BAUD_RATE(self->BAUD_RATE, self->CLK_FREQ) + 0.5); // Calculate and round
    *(self->CONFIG->BAUD) = baud_setting; // Assign to the BAUD register
}

void EnableTR(UART *self)
{
    //Enable the RX Interrupt
    *self->CONFIG->CTRLA = self->CONFIG->INT_bm;
    
    //Enable transmit and receive.
    *self->CONFIG->CTRLB |= self->CONFIG->RXEN_bm | self->CONFIG->TXEN_bm; 

    
    //override printf
    stdout = &USART_stream;

    //Setup the RX and TX pins
    *self->CONFIG->RX_PORT |= self->CONFIG->RX_PIN_bm;
    *self->CONFIG->TX_PORT &= ~self->CONFIG->TX_PIN_bm;
}

//Override the printf functionality. 
static int PrintChar(char c, FILE *stream)
{
    SendChar(c);
    return 0;
}

static void SendChar(char c) 
{
    //Check the status and wait for previous transmission to send
    while (!(__STATUS__ & __INTFLAG_bm__));
    
    //Send char to buffer. 
    __TXDATA_L__ = c;
}

uint8_t Read(UART *self)
{
//    while (!(USART0.STATUS & USART_RXCIF_bm));
	return *self->CONFIG->RXDATA_L;
}

void Transmit(char str[])
{
    // Iterate through each character using a while loop
    int i = 0;
    while (str[i] != '\0') {
        SendChar(str[i]);  // Print each character
        i++;
    }
}

void TransmitUint8(uint8_t value) {
    char str[4]; // Enough to hold any uint8_t value and null terminator
    sprintf(str, "%u", value); // Convert uint8_t to string
    Transmit(str); // Use the existing Transmit function
}
void TransmitUint16(uint16_t value) {
    char str[6]; // Enough to hold any uint16_t value and null terminator
    sprintf(str, "%u", value); // Convert uint16_t to string
    Transmit(str); // Use the existing Transmit function
}
void TransmitFloat(float value) {
    // Convert the float to an integer part and a fractional part
    int intPart = (int)value;
    int fracPart = (int)((value - intPart) * 100); // Adjust the multiplier (100) for desired precision

    char str[20]; // Buffer for the complete string
    sprintf(str, "%d.%02d", intPart, fracPart); // Combine integer and fractional parts
    Transmit(str);
}