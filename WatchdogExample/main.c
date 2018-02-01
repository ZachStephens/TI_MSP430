#include <msp430.h>				


#pragma vector = WDT_VECTOR  //Interval timer vector location
__interrupt void IntervalTimer(void)
{
  P1OUT ^= BIT0; // Toggle P1.0
}

void main(){
  WDTCTL = WDTPW + WDTHOLD;// Stop the WDT to prevent reset
  int i;                   // Count value for delay
  P1DIR |= BIT0  + BIT6;   // P1.0(Red) and P1.6(Green)output
  P1OUT &= ~BIT0 + BIT6 ;  // P1.0 = 0,P1.6 =0
  //for(i =0;i<0xffff;i++);  // A Small delay
  P1OUT |= BIT0;           // P1.0 = 1(Red LED On)
  WDTCTL = WDT_ARST_1000;  // Put WDT+ in Watch Dog Mode

  //while(1);              // used to simulate glitch

  WDTCTL = WDT_ARST_1000;  // Reset WDT+ for another 1 second,
  P1OUT  |= BIT6;          // Switch on green LED
  //WDTCTL = WDTPW + WDTHOLD;// Stop WDT
  while(1){}//_BIS_SR(LPM0);           // Put the CPU to sleep
}
