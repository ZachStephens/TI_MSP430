#include "msp430g2553.h"
#include "MPU6050.h"
#include "zs_msp430_Uart.h"
#include "I2C.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define PRECISION 5

char itoa(int i){
    if((i < 0) || (i > 9)){
        return -1;
    }

    return '0' + (char)i;
}

//float to char array expecting array of size 10
//-,d1,d0,.,d.1,d.01,d.001...
void float2char(float data,char * buff,int places){
    char isNeg = (data < 0)?1:0;
    char isTen;
    int leftdigs = (int)data;
    int buff_i = 4;

    if(isNeg){
            data*=-1;
            leftdigs*=-1;
    }
    isTen = (data > 9)?1:0;

    buff[0] = (isNeg)?((isTen)?'-':' '):' ';
    buff[1] = (isTen)?1:((isNeg)?'-':' ');
    buff[2] = itoa(((int)data) - ((isTen)?10:0));
    buff[3] = '.';

    data = data - (float)leftdigs;
    while(places-- > 0){
        data *= 10;
        leftdigs = (int)data;
        buff[buff_i++] = itoa((int)data);
        data = data - (float)leftdigs;
    }

}


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
         MPU6050_Init(GYRO_CONFIG_250,ACC_CONFIG_2G);
         __delay_cycles(10000);
//initialize uart
         serialsendbytes("Hello ",6);
         serialsendbytes("world!\r\n",8);
         __bis_SR_register(GIE);
         while(1) {

             MPU6050_GetAccValueRaw(&raw_acc_data);
             MPU6050_AccConvertData(raw_acc_data,&scaled_acc_data);


            // sprintf(printBuf,"%f%f%f",scaled_acc_data.x,scaled_acc_data.y,scaled_acc_data.z);

             __delay_cycles(10000);
             float2char(scaled_acc_data.x,printBuf,PRECISION);
             serialsendbytes(printBuf,4+PRECISION);
             sendByte(' ');
             float2char(scaled_acc_data.y,printBuf,PRECISION);
             serialsendbytes(printBuf,4+PRECISION);
             sendByte(' ');
             float2char(scaled_acc_data.z,printBuf,PRECISION);
             serialsendbytes(printBuf,4+PRECISION);
             //sendByte('\n');
             sendByte('\r');
         }
}



