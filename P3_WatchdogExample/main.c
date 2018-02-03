#include <msp430.h>				

#define S2 BIT3
/*#pragma vector = WDT_VECTOR  //Interval timer vector location
__interrupt void IntervalTimer(void)
{
  P1OUT ^= BIT0; // Toggle P1.0
}*/

void main(){
  WDTCTL = WDTPW + WDTHOLD; // disable watchdog to prevent WD reset from triggering itself

  P1DIR |= BIT0  + BIT6;
  P1DIR &= ~S2;                   // mask S2 bit with 0 to indicate input
  P1REN |= S2;                    // Enable Resistor for S2 pin
  P1OUT |= S2;

  P1OUT &= 0x0 ;                  // turn off both LEDs


  P1OUT |= BIT0;            // P1.0 = 1(Red LED On)
  WDTCTL = WDT_ARST_1000;  // Put WDT+ in Watch Dog Mode

  //P1IFG is interrupt flag for P1 input
  if(!(P1IFG & S2)){            // S2 pressed triggers delay causing watchdog
          while(1);
  }

  WDTCTL = WDT_ARST_1000;  // Refresh WD so we can see green LED,
  P1OUT  |= BIT6;          // Switch on green LED
  while(1){}
}
