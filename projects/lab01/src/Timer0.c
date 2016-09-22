#include "includes.h"

#define TIMER0_TICK_PER_SEC   1 // was 20
/*************************************************************************
 * Function Name: Timer0IntrHandler
 * Parameters: none
 *
 * Return: none
 *
 * Description: Timer 0 interrupt handler
 *
 *************************************************************************/
void Timer0IntrHandler (void) {
  
  /* Toggle USB Link LED */
  USB_LINK_LED_FIO ^= USB_LINK_LED_MASK;
  
  /* clear interrupt */
  T0IR_bit.MR0INT = 1;
  VICADDRESS = 0;
}



/*************************************************************************
 * Function Name: VIC_SetVectoredIRQ
 * Parameters:  void(*pIRQSub)()
 *              unsigned int VicIrqSlot
 *              unsigned int VicIntSouce
 *
 * Return: void
 *
 * Description:  Init vectored interrupts
 *
 *************************************************************************/
void VIC_SetVectoredIRQ(void(*pIRQSub)(), unsigned int Priority,
                        unsigned int VicIntSource)
{
unsigned long volatile *pReg;
  // load base address of vectored address registers
  pReg = &VICVECTADDR0;
  // Set Address of callback function to corresponding Slot
  *(pReg+VicIntSource) = (unsigned long)pIRQSub;
  // load base address of ctrl registers
  pReg = &VICVECTPRIORITY0;
  // Set source channel and enable the slot
  *(pReg+VicIntSource) = Priority;
  // Clear FIQ select bit
  VICINTSELECT &= ~(1<<VicIntSource);
}

void InitTimer0Interrupt(uint32_t ticksPerSec) {
   PCONP_bit.PCTIM0 = 1; // Enable TMR0 clk
   T0TCR_bit.CE = 0;     // counting  disable
   T0TCR_bit.CR = 1;     // set reset
   T0TCR_bit.CR = 0;     // release reset
   T0CTCR_bit.CTM = 0;   // Timer Mode: every rising PCLK edge
   T0MCR_bit.MR0I = 1;   // Enable Interrupt on MR0
   T0MCR_bit.MR0R = 1;   // Enable reset on MR0
   T0MCR_bit.MR0S = 0;   // Disable stop on MR0

/* Set timer 0 period */
   T0PR = 0;
   T0MR0 = SYS_GetFpclk(TIMER0_PCLK_OFFSET)/(ticksPerSec);
   
/* Init timer 0 interrupt */
   T0IR_bit.MR0INT = 1;  /* Clear pending interrupt */
   VIC_SetVectoredIRQ(Timer0IntrHandler,0,VIC_TIMER0); /* Assign address of ISR */
   VICINTENABLE |= 1UL << VIC_TIMER0;
   T0TCR_bit.CE = 1;     /* Counting Enable */

   __enable_interrupt();
}