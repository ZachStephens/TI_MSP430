#include <msp430.h>				


#define S2  BIT3                    // Switch -> P1.3
#define R_LED BIT0                // Red LED -> P1.0
#define G_LED BIT6

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    P1DIR |= (R_LED ^ G_LED);       // mask LED bits with 1 indicate output
    P1DIR &= ~S2;                   // mask S2 bit with 0 to indicate input
    P1REN |= S2;                    // Enable Resistor for S2 pin
    P1OUT |= S2;                    // Select Pull Up for S2 pin

    while(1)
    {
        if(!(P1IN & S2))            // If SW is Pressed
        {
            __delay_cycles(5000);  // Wait 10ms to debounce
            while(!(P1IN & S2));    // ensure s2 not pressed
            P1OUT ^= (R_LED ^ G_LED);           // Toggle LED
            __delay_cycles(5000);  // Wait 10ms to debounce
            while(!(P1IN & S2));    // ensure s2 is still not pressed

        }
    }
}
