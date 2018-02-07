#include <msp430.h>
#include <msp430_pwm.h>

//static unsigned int t1;
#define G_LED BIT6                        // Green G_LED -> P1.6
/*
#pragma vector = TIMER0_A0_VECTOR       // CCR0 Interrupt Vector
__interrupt void CCR0_ISR(void)
{
    TA0CTL &= ~TAIFG;               //clear timer A interrupt flag


    P1OUT ^= G_LED;                   //Toggle G_LED
}

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;           //watchdog



    P1DIR |= G_LED;                       // Set G_LED pin -> Output
    P1OUT &=~ G_LED;                      // Turn OFF G_LED

    TACCR0 = 1000;                      // Set Timer Timeout Value
    TACCTL0 |= CCIE;                   // Enable Overflow Interrupt
    TACTL |=   MC_1                    // mode count up to CCR0
             | TASSEL_1    // clock source A_clk
             | TACLR        //timer A counter clear
             |  ID_0;       //divides A clock by 1,2,4 or 8 if 0,1,2,or 3 respectively

    //BCSCTL3 |= LFXT1S_2;    //sets clock to internal oscillator (VLO)
    __bis_SR_register(LPM3_bits + GIE); // Goto LPM3 (Only ACLK active), Enable CPU Interrupt
}*/

unsigned int blink_count;
void main()
{

    WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
    P1SEL &= (~BIT0);
    P1OUT &= (~BIT0);
    P1DIR |= (BIT0);


    /* Check if 1MHz Calibration is present */
    if (CALBC1_1MHZ != 0xFF)
    {
        DCOCTL = 0; // Select lowest DCOx and MODx
        BCSCTL1 = CALBC1_1MHZ; // Set range
        DCOCTL = CALDCO_1MHZ; // Set DCO step + modulation
    }

    for(blink_count = 0; blink_count < 100; blink_count++)
    {
        P1OUT ^= (BIT0); // Toggle LED
        __delay_cycles(1000);  // Wait 10ms to debounce
    }
}
