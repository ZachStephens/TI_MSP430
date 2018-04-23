#include "msp430g2553.h"
#include <stdint.h>

void I2C_Init()
{
        P1OUT &=~(BIT0+BIT3);
        P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
        P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
        UCB0CTL1 |= UCSWRST;                      // Enable SW reset
        UCB0CTL0 = UCMST+UCMODE_3+UCSYNC;         // I2C Master, synchronous modev
        UCB0CTL1 = UCSSEL_2+UCSWRST;              // Use SMCLK, keep SW reset
        UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~400kHz
        UCB0BR1 = 0;
        UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
}


uint8_t I2C_ReadByte(uint8_t slaveAddress, uint8_t byRegister)
{
        UCB0I2CSA = slaveAddress;

        while (UCB0CTL1 & UCTXSTP);                 //
        UCB0CTL1 |= UCTR + UCTXSTT;                 // I2C TX,START

        while (!(IFG2&UCB0TXIFG));
        UCB0TXBUF = byRegister;                         //

        while (!(IFG2&UCB0TXIFG));

        UCB0CTL1 &= ~UCTR;                      // I2C RX
        UCB0CTL1 |= UCTXSTT;                    // I2C RESTART
        IFG2 &= ~UCB0TXIFG;                     //

        while (UCB0CTL1 & UCTXSTT);             //
        UCB0CTL1 |= UCTXSTP;                    //  bit STOP
        while (UCB0CTL1 & UCTXSTP);
        return UCB0RXBUF;
}


uint8_t I2C_WriteByte(uint8_t byData, uint8_t slaveAddress, uint8_t byRegister)
{

        UCB0I2CSA = slaveAddress;                         // Set slave address

        while (UCB0CTL1 & UCTXSTP);             // stop condition sent
        UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, bit START

        while (!(IFG2&UCB0TXIFG));              // ensure tx flag is set
        if(UCB0STAT & UCNACKIFG) return UCB0STAT;
        UCB0TXBUF = byRegister;


        while (!(IFG2&UCB0TXIFG));
        if(UCB0STAT & UCNACKIFG) return UCB0STAT;
        UCB0TXBUF = byData;

        while (!(IFG2&UCB0TXIFG));
        if(UCB0STAT & UCNACKIFG) return UCB0STAT;
        UCB0CTL1 |= UCTXSTP;
        IFG2 &= ~UCB0TXIFG;
        return 0;
}



uint8_t I2C_ReadData(uint8_t *Data,uint8_t slaveAddress,uint8_t Addr_Data, uint8_t Length)
{
        uint8_t i=0;

        UCB0I2CSA = slaveAddress;

        while (UCB0CTL1 & UCTXSTP);             // Loop until I2C STT is sent
        UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition

        while (!(IFG2&UCB0TXIFG));
        IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
        if(UCB0STAT & UCNACKIFG) return UCB0STAT;   //Neu bao loi
        UCB0TXBUF = Addr_Data;                          // Dia chi luu gia tri Seconds

        while (!(IFG2&UCB0TXIFG));
        if(UCB0STAT & UCNACKIFG) return UCB0STAT;   //Neu bao loi

        UCB0CTL1 &= ~UCTR;                      // I2C RX
        UCB0CTL1 |= UCTXSTT;                    // I2C start condition
        IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
        while (UCB0CTL1 & UCTXSTT);             // Loop until I2C STT is sent
        for(i=0;i<(Length-1);i++)
        {
                while (!(IFG2&UCB0RXIFG));
                IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
                Data[i] = UCB0RXBUF;
        }
        while (!(IFG2&UCB0RXIFG));
        IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
        UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
        Data[Length-1] = UCB0RXBUF;
        IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
        return 0;
}



uint8_t I2C_WriteData(uint8_t *data, uint8_t slaveAddress, uint8_t address, uint8_t Length)
{
        uint8_t i=0;

        UCB0I2CSA = slaveAddress;                         // Set slave address

        while (UCB0CTL1 & UCTXSTP);             // Loop until I2C STT is sent

        UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition

        while (!(IFG2&UCB0TXIFG));
        IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag

        if(UCB0STAT & UCNACKIFG) return UCB0STAT;   //Neu bao loi
        UCB0TXBUF = address;                        // Dia chi luu gia tri Seconds
        for(i = 0; i < (Length - 1); i++)
        {
                UCB0TXBUF = data[i];
        }

        while (!(IFG2&UCB0TXIFG));              // Cho gui xong
        if(UCB0STAT & UCNACKIFG) return UCB0STAT;   //Neu bao loi thì thoat khoi ham
        UCB0CTL1 |= UCTXSTP;                    // Gui bit STOP
        IFG2 &= ~UCB0TXIFG;                     // Xoa co USCI_B0 TX
        return 0;
}


