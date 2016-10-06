/*************************************************************************
 * An exercise in the use of interrupts
 *
 * CM0605 - Lab01S
 *
 **************************************************************************/
#include <bsp.h>
#include <timers.h>
#include <lcd.h>
#include <buttons.h>
#include <leds.h>

void displayValue(int);
void timer0ISR(void);
void timer1ISR(void);

/*************************************************************************
 *
 * Function Name: main 
 * Parameters: none
 *
 * Return: none
 *
 *
 *
 *************************************************************************/
void main(void) { 

   int counter= 0;
   

   bspInit();                          /* Initialise board support package */
   initTimer(TIMER0, timer0ISR, 10);   /* Initialise timer 0 interrupt - parameter specifies number of ticks per second */
   initTimer(TIMER1, timer1ISR, 1);
   startTimer(TIMER0);
   startTimer(TIMER1);
   __enable_interrupt();

   displayValue(counter);
   while (true) {
     while (! isButtonPressed(BUT_1) ) {
                                       /* Wait for BUT_1 press */
     }
     counter++;                        /* Update counter */
     displayValue(counter);            /* Display the value of counter */
     while (isButtonPressed(BUT_1)) {
                                       /* Wait for BUT_1 release */
     }
   }
}

/* displayValue()
 *
 * Display the value of the int parameter at (2,1)
 * The "%d" formats an integer 
 */
void displayValue( int i ) {
   lcdSetTextPos( 2, 1 );        /* Put cursor at col=2, row= 1 */
   lcdWrite("counter: %d",i);    /* Print the value of the parameter */
}

void timer0ISR(void) {
  ledToggle(USB_LINK_LED);  
}

void timer1ISR(void) {
  ledToggle(USB_CONNECT_LED);  
}
