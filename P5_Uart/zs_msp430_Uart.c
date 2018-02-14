
#include <msp430.h>
#include "zs_msp430_Uart.h"

const char serialBuffer[40];
const char* buffHead, buffTail;

void setupUCA0forSMCLK(){
    UCA0CTL1 |= UCSSEL_2; // SMCLK --UCSSEL_1 would select external crystal


}

void setupUCA0_115200(){
    UCA0BR0 = 0x08; // 1MHz 115200
    UCA0BR1 = 0x00; // 1MHz 115200
    UCA0MCTL = UCBRS2 + UCBRS0; // Modulation UCBRSx = 5
}


void serialInit(){
    buffHead = serialBuffer[0];
    buffTail = serialBuffer[0];
}

int sendByte(char b){
    if(buffHead == buffTail)
        return -1;
    serialBuffer[buffHead] = *b;
    buffHead++;
    buffHead %= 40;
    UC0IE |= UCA0TXIE;

    return 0;
}

void serialsendbytes(char * p,unsigned int n){
    unsigned int i = 0;
    while(i < n){
       sendByte; // Enable USCI_A0 TX interrupt
    }

}


#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
    P1OUT |= TXLED;
    UCA0TXBUF = string[i++]; // TX next character
    if (i == sizeof string - 1) // TX over?
        UC0IE &= ~UCA0TXIE; // Disable USCI_A0 TX interrupt
    P1OUT &= ~TXLED;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    P1OUT |= RXLED;
    if (UCA0RXBUF == 'a') // 'a' received?
    {
        i = 0;
        UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
        UCA0TXBUF = string[i++];
    }
    P1OUT &= ~RXLED;
}
