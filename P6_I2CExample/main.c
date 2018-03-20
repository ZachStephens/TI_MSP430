#include <msp430.h> 
#include "MPU6050.h"

/**
 * main.c
 */
void init_I2C(void);
int i2c_notready(void);
char Receive(char);
void Transmit(char, char);

int main(void) {

         volatile char who_am_i;
         char test;

         WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
         BCSCTL1 = CALBC1_1MHZ;                    // Set DCO to 1Mhz
         DCOCTL = CALDCO_1MHZ;
         P1SEL |= BIT6 + BIT7;       // Assign I2C pins to USCI_B0
         P1SEL2 |= BIT6 + BIT7;       // Assign I2C pins to USCI_B0

         init_I2C();    // initialize i2c
         __delay_cycles(10000);

         while ( i2c_notready() );       // wait for bus to be free
         __delay_cycles(10000);

         who_am_i = Receive(MPU6050_RA_WHO_AM_I);

         while(1) {}
}

void init_I2C(void) {
          UCB0CTL1 |= UCSWRST;                      // Enable SW reset
          UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
          UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
          UCB0BR0 = 10;                             // fSCL = 1Mhz/10 = ~100kHz
          UCB0BR1 = 0;
          UCB0I2CSA = MPU6050_DEFAULT_ADDRESS;
          UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
          IE2 |= UCB0RXIE + UCB0TXIE;    // Enable RX and TX interrupt
}


int i2c_notready(){
        if(UCB0STAT & UCBBUSY) return 1;
        else return 0;
}


char Receive(char registerAddr){
        char receivedByte;
        while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
        UCB0CTL1 |= UCTR + UCTXSTT;             // I2C start condition with UCTR flag for transmit
        while((IFG2 & UCB0TXIFG) == 0);     //UCB0TXIFG is set immidiately
        UCB0TXBUF = registerAddr;           //write registerAddr in TX buffer
        while((IFG2 & UCB0TXIFG) == 0);     // wait until TX buffer is empty and transmitted
        UCB0CTL1 &= ~UCTR ;                // Clear I2C TX flag for receive
        UCB0CTL1 |= UCTXSTT + UCTXNACK;    // I2C start condition with NACK for single byte reading
        while (UCB0CTL1 & UCTXSTT);             // Start condition sent? RXBuffer full?
        receivedByte = UCB0RXBUF;
        UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
        return receivedByte;
}


void Transmit(char registerAddr, char data){
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C start condition with UCTR flag for transmit
    while((IFG2 & UCB0TXIFG) == 0);         //UCB0TXIFG is set immidiately
    UCB0TXBUF = registerAddr;               //write registerAddr in TX buffer
    while((IFG2 & UCB0TXIFG) == 0);         // wait until TX buffer is empty and transmitted
    UCB0TXBUF = data;                       //Write data in register
    while((IFG2 & UCB0TXIFG) == 0);         // wait until TX buffer is empty and transmitted
    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
    IFG2 &= ~UCB0TXIFG;                     // Clear TX interrupt flag
}
