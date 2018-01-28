#include <msp430.h>

unsigned int t1;
#define interrupt(x) void __attribute__((interrupt (x)))

interrupt(TIMER0_A0_VECTOR) tmr_ccr0 () {
    // interrupt flag is automatically reset
    // ccr0
    TA0R=0;
    //if(++t1 == 1000){
       P1OUT ^= ~(BIT0);
    //    t1=0;
    //}
}

int main() {
    WDTCTL = WDTPW + WDTHOLD;

    // configure clocks
    DCOCTL  =  CALDCO_1MHZ;
    //BCSCTL1 =  CALBC1_1MHZ;

    // i/o
    P1DIR = BIT6 | BIT0;
    P1OUT = 0x00;

    // timer
    TACCR0 = 0xFFFF-1;     // 1 ms
    //TACCR1 = 0xFFFF;     // 2 ms

    TACCTL0 = CCIE;
    //TACCTL1 = CCIE;

    TACTL = TASSEL_2 | MC_2 | TAIE;

    t1=0;
    __enable_interrupt();
    for (;;) {}
    return 0;
}
