
#ifndef ZS_MSP430_UART_H_
#define ZS_MSP430_UART_H_

#define TXLED BIT0
#define RXLED BIT6
#define TXD BIT2
#define RXD BIT1

#define TBUFFSIZE 100

#define True 1
#define False 0

extern const char string[];
unsigned int i; //Counter

void setupUCA0forSMCLK();

void setupUCA0_115200();

void serialInit();

int sendByte(char);

void serialsendbytes(char*,unsigned int);

unsigned short inline serialBufferisFull();

__interrupt void USCI0RX_ISR(void);

__interrupt void USCI0TX_ISR(void);

#endif /* ZS_MSP430_UART_H_ */
