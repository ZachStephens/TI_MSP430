#include <msp430.h> 
#include "zs_msp430_Uart.h"



int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    DCOCTL = 0; // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ; // Set DCO
    DCOCTL = CALDCO_1MHZ;

    P2DIR |= 0xFF; // All P2.x outputs
    P2OUT &= 0x00; // All P2.x reset
    P1SEL |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
    P1DIR |= RXLED + TXLED;
    P1OUT &= 0x00;

    setupUCA0forSMCLK();
    setupUCA0_115200();

    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
    UC0IE |= UCA0RXIE; // Enable USCI_A0 RX interrupt
    __bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ int until Byte RXed
    while (1)
    { }
}


