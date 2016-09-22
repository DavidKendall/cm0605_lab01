#include <iolpc2378.h>
#include <board.h>
#include <bsp.h>
#include <timers.h>

// Initialise Timer0 
// handler - the interrupt handler for the timer0 interrupt
// ticksPerSec - the tick rate
void initTimer0(pVoidFunc_t handler, uint32_t ticksPerSec) {
  T0TCR = 0x02;  // reset timer
  T0PR = 0x00;   // set prescaler to 0
  T0CTCR = 0x00; // set mode: every rising PCLK edge
  T0MR0 = getFpclk(TIMER0_PCLK_OFFSET) / ticksPerSec; 
  T0IR = 0xff;   // reset all interrupts
  T0MCR = 0x03;  // enable interrupt and reset on match
  vicInstallIRQhandler(handler, 0, VIC_TIMER0);
}


void startTimer0(void) {
  T0TCR = 0x01;  // start timer 0
}

void stopTimer0(void) {
  T0TCR = 0x00;  // stop timer 0
}

