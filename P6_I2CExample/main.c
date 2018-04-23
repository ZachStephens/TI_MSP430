#include "msp430g2553.h"
#include "MPU6050.h"
#include "zs_msp430_Uart.h"
#include "I2C.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {

         ACC_DATA_RAW raw_acc_data;
         ACC_DATA_SCALED scaled_acc_data;
         char printBuf[16];
         volatile uint8_t who_am_i;
         //char test;


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

         serialInit();

         __delay_cycles(1000);
         I2C_Init();    // initialize i2c
         serialsendbytes("Hello ",6);
         serialsendbytes("world!\r\n",8);
         who_am_i = MPU6050_CheckI2C();
         MPU6050_Init(GYRO_CONFIG_250,ACC_CONFIG_4G);
         __delay_cycles(10000);
//initialize uart
         serialsendbytes("Hello ",6);
         serialsendbytes("world!\r\n",8);
         __bis_SR_register(GIE);
         while(1) {

             MPU6050_GetAccValueRaw(&raw_acc_data);
             MPU6050_AccConvertData(raw_acc_data,&scaled_acc_data);




             __delay_cycles(10000);
             itoa(raw_acc_data.x,printBuf,10);
             serialsendbytes(printBuf,2);
             sendByte(' ');
             itoa(raw_acc_data.y,printBuf,10);
             serialsendbytes(printBuf,2);
             sendByte(' ');
             itoa(raw_acc_data.z,printBuf,10);
             serialsendbytes(printBuf,2);
             sendByte('\n');
         }
}
