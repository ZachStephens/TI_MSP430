#include <msp430.h> 




//drives Brushless DC motor basic HW 30 Amp ESC.
//PWM of 50% a@ period 2ms engages the esc
//PWM 75%-100% (translates to 1.5-2 ms) adjusts throttle


unsigned int ADC_result=0;

static unsigned int highcount, lowcount;
static unsigned char dutyHigh;

#define G_LED BIT6                        // Green G_LED -> P1.6
#define CLOCKS_PER_PERIOD 1670

// Function prototypes
void ConfigureAdc(void);
void ConfigurePWM(void);


#pragma vector = TIMER0_A0_VECTOR       // CCR0 Interrupt Vector
__interrupt void CCR0_ISR(void)
{

    if(highcount < 100)
                    highcount = 100;
    if(!dutyHigh){
        TACCR0 = highcount;
        lowcount = CLOCKS_PER_PERIOD - highcount;
        P1OUT |= G_LED;
    }else{
        TACCR0 = lowcount;
        P1OUT &= ~G_LED;
    }
    dutyHigh = (dutyHigh==0);


    //TA0CTL &= ~TAIFG;               //clear timer A interrupt flag
    TA0CCTL0 &= ~CCIFG;

}


void main(void)
{
    {
        WDTCTL = WDTPW + WDTHOLD;       // Stop WDT

       //PWM clock
       TACTL |=   MC_1         // mode count up to CCR0
                | TASSEL_2     // clock source sm_clk 1Mhz +-10%
                | TACLR        //timer A counter clear
                |  ID_0;       //divides A clock by 1,2,4 or 8 if 0,1,2,or 3 respectively

       //ADC clock
        BCSCTL1 = CALBC1_1MHZ;          // Set range   DCOCTL = CALDCO_1MHZ;
        BCSCTL2 &= ~(DIVS_3);           // SMCLK = DCO = 1MHz
        P1SEL |= BIT3;                  // ADC input pin P1.3


        ConfigureAdc();                 // ADC set-up
        ConfigurePWM();                 // PWM set-up
        __enable_interrupt();           // Enable interrupts.

        while(1)
        {
            __delay_cycles(100000);               // Wait for ADC Ref to settle
            ADC10CTL0 |= ENC + ADC10SC;         // Sampling and conversion start
            __bis_SR_register(CPUOFF + GIE);    // Low Power Mode 0 with interrupts enabled
            ADC_result = ADC10MEM;               // Assigns the value held in ADC10MEM to the integer called ADC_value
            /*Max ADC result should be < CLOCKS_PER_PERIOD. Scale by half by truncating least signifant bits*/
            highcount=(ADC_result /*>> 1*/);


        }

    }
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
    __bic_SR_register_on_exit(CPUOFF);
}

// Function containing ADC set-up
void ConfigureAdc(void)
{

    ADC10CTL1 = INCH_3 + ADC10DIV_7 ;         // Channel 3, ADC10CLK/3
    ADC10CTL0 = SREF_0 + ADC10SHT_1 + ADC10ON + ADC10IE;  // Vcc & Vss as reference, Sample and hold for 8 Clock cycles, ADC on, ADC interrupt enable
    ADC10AE0 |= BIT3;                         // ADC input enable P1.3
}

void ConfigurePWM(void){
    P1DIR |= G_LED;                       // Set G_LED pin -> Output
    P1OUT &=~ G_LED;                      // Turn OFF G_LED

    dutyHigh = 0;
    highcount = CLOCKS_PER_PERIOD>>2;
    TACCR0 = 40;                      // Set Timer Timeout Value
    TACCTL0 |= CCIE;                   // Enable Overflow Interrupt
}
