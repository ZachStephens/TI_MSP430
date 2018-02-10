#include <msp430.h>


static unsigned int t1;
static unsigned int highcount;
static unsigned int resolution;
#define G_LED BIT6                        // Green G_LED -> P1.6

#pragma vector = TIMER0_A0_VECTOR       // CCR0 Interrupt Vector
__interrupt void CCR0_ISR(void)
{
    TA0CTL &= ~TAIFG;               //clear timer A interrupt flag

    t1=(++t1 > (resolution-1) )?0:t1;

    if(t1 >= highcount)
        P1OUT &= ~G_LED;
    else
        P1OUT |= G_LED;
}

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;           //watchdog

    P1DIR |= G_LED;                       // Set G_LED pin -> Output
    P1OUT &=~ G_LED;                      // Turn OFF G_LED

    resolution= 10;
    highcount = 4;
    TACCR0 = 4000;                      // Set Timer Timeout Value
    TACCTL0 |= CCIE;                   // Enable Overflow Interrupt
    TACTL |=   MC_1                    // mode count up to CCR0
             | TASSEL_2     // clock source sm_clk 1Mhz +-10%
             | TACLR        //timer A counter clear
             |  ID_0;       //divides A clock by 1,2,4 or 8 if 0,1,2,or 3 respectively

    //BCSCTL3 |= LFXT1S_2;    //sets clock to internal oscillator (VLO)
    __bis_SR_register(GIE); // Enable CPU Interrupt
}


