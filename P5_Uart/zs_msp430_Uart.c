
#include <msp430.h>
#include "zs_msp430_Uart.h"



char serialBuffer[TBUFFSIZE];
unsigned short buffHead;
unsigned short buffTail;

void setupUCA0forSMCLK(){
    UCA0CTL1 |= UCSSEL_2; // SMCLK --UCSSEL_1 would select external crystal


}

void setupUCA0_115200(){
    UCA0BR0 = 0x08; // 1MHz 115200
    UCA0BR1 = 0x00; // 1MHz 115200
    UCA0MCTL = UCBRS2 + UCBRS0; // Modulation UCBRSx = 5
}


void serialInit(){
    buffHead = 0;
    buffTail = 0;
}

unsigned short inline serialBufferisFull(){
    if((buffHead + 1) % TBUFFSIZE == buffTail){
        return True;
    }
    return False;
}

int sendByte(char B){
    if(serialBufferisFull())
        return -1;


    serialBuffer[buffHead] = B;
    buffHead= (buffHead +1) % TBUFFSIZE;
    UC0IE |= UCA0TXIE;

    return 0;
}

void serialsendbytes(char * p,unsigned int n){
    unsigned int i = 0;
    while(i < n){
       while(sendByte(*(p++)) != 0); // send when buffer is not full
    }
}



#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
    P1OUT |= TXLED;
    UCA0TXBUF = serialBuffer[buffTail++]; // TX next character
    if (buffTail == buffHead) // buffer full?
        UC0IE &= ~UCA0TXIE; // Disable USCI_A0 TX interrupt
    P1OUT &= ~TXLED;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    P1OUT |= RXLED;
    if (UCA0RXBUF == 'a') // 'a' received?
    {
            //
        serialsendbytes("caught a\r\n",10);
    }
    P1OUT &= ~RXLED;
}
