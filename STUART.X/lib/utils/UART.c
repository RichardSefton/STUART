#include "UART.h"

static FILE USART_stream = FDEV_SETUP_STREAM(PrintChar, NULL, _FDEV_SETUP_WRITE);

void EnableTR(void)
{
    //Enable the RX Interrupt
    USART0.CTRLA = USART_RXCIE_bm;
//    USART0.CTRLA &= ~USART_TXCIE_bm;
    
    //Enable transmit and receive. 
//    USART0.CTRLB = USART_TXEN_bm;
    USART0.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;

    
    //override printf
    stdout = &USART_stream;
}

void SetBaudRate(void)
{
    USART0.BAUD = USART0_BAUD_RATE(BAUD_RATE, F_CPU);
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
    while (!(USART0.STATUS & USART_DREIF_bm));
    
    //Send char to buffer. 
    USART0.TXDATAL = c;
}

uint8_t Read(void)
{
//    while (!(USART0.STATUS & USART_RXCIF_bm));
	return USART0.RXDATAL;
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