#include <stdint.h>
#include <intrinsics.h>
#include <iolpc2378.h>
#include <board.h>
#include <leds.h>
#include <buttons.h>
#include <potentiometer.h>
#include <accelerometer.h>
#include <lcd.h>
#include <can.h>
#include <bsp.h>


/*************************************************************************
* Function Name: mamInit
* Parameters: void
* Return: void
*
* Description: Initialise the memory accelerator module
*************************************************************************/

void mamInit() {
   MAMCR_bit.MODECTRL = 0;
   MAMTIM_bit.CYCLES = 3;    // FCLK > 40 MHz
   MAMCR_bit.MODECTRL = 2;   // MAM functions fully enabled
}


/*************************************************************************
 * Function Name: clockInit
 * Parameters: void
 * Return: void
 *
 * Description: Initialize PLL and clocks' dividers. Hclk - 288MHz,
 * Usbclk - 48MHz
 *
 *************************************************************************/
void clockInit(void)
{
  // 1. Init OSC
  SCS_bit.OSCRANGE = 0;
  SCS_bit.OSCEN = 1;
  // 2.  Wait for OSC ready
  while(!SCS_bit.OSCSTAT);
  // 3. Disconnect PLL
  PLLCON_bit.PLLC = 0;
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  // 4. Disable PLL
  PLLCON_bit.PLLE = 0;
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  // 5. Select source clock for PLL
  CLKSRCSEL_bit.CLKSRC = 1; // Selects the main oscillator as a PLL clock source.
  // 6. Set PLL settings 288 MHz
  PLLCFG_bit.MSEL = 24-1;
  PLLCFG_bit.NSEL = 2-1;
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  // 7. Enable PLL
  PLLCON_bit.PLLE = 1;
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  // 8. Wait for the PLL to achieve lock
  while(!PLLSTAT_bit.PLOCK);
  // 9. Set clk divider settings
  CCLKCFG   = 6-1;            // 1/6 Fpll
  USBCLKCFG = 6-1;            // 1/6 Fpll - 48 MHz
  PCLKSEL0 = PCLKSEL1 = 0;    // other peripherals
  // 10. Connect the PLL
  PLLCON_bit.PLLC = 1;
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
}


/*************************************************************************
 * Function Name: getFsclk
 * Parameters: none
 * Return: uint32_t
 *
 * Description: return Sclk [Hz]
 *
 *************************************************************************/
uint32_t getFsclk(void)
{
uint32_t Mul = 1, Div = 1, Osc, Fsclk;
  if(PLLSTAT_bit.PLLC)
  {
    // when PLL is connected
    Mul = PLLSTAT_bit.MSEL + 1;
    Div = PLLSTAT_bit.NSEL + 1;
  }

  // Find clk source
  switch(CLKSRCSEL_bit.CLKSRC)
  {
  case 0:
    Osc = I_RC_OSC_FREQ;
    break;
  case 1:
    Osc = MAIN_OSC_FREQ;
    break;
  case 2:
    Osc = RTC_OSC_FREQ;
    break;
  default:
    Osc = 0;
  }
  // Calculate system frequency
  Fsclk = Osc*Mul*2;
  Fsclk /= Div*(CCLKCFG+1);
  return(Fsclk);
}

/*************************************************************************
 * Function Name: getFpclk
 * Parameters: uint32_t peripheral
 * Return: uint32_t
 *
 * Description: return Pclk [Hz]
 *
 *************************************************************************/
uint32_t getFpclk(uint32_t peripheral)
{
uint32_t Fpclk;
uint32_t *pReg = (uint32_t *)((peripheral < 32)?&PCLKSEL0:&PCLKSEL1);

  peripheral  &= 0x1F;   // %32
  Fpclk = getFsclk();
  // find peripheral appropriate periphery divider
  switch((*pReg >> peripheral) & 3)
  {
  case 0:
    Fpclk /= 4;
    break;
  case 1:
    break;
  case 2:
    Fpclk /= 2;
    break;
  default:
    Fpclk /= 8;
  }
  return(Fpclk);
}

/*************************************************************************
 * Function Name: gpioInit
 * Parameters: void
 * Return: void
 *
 * Description: Reset all GPIO pins to default: primary function
 *
 *************************************************************************/
void gpioInit(void) {

// Set to inputs
  IO0DIR  = 0;
  IO1DIR  = 0;
  FIO0DIR = 0;
  FIO1DIR = 0;
  FIO2DIR = 0;
  FIO3DIR = 0;
  FIO4DIR = 0;
  
// enable fast GPIOs
  SCS_bit.GPIOM = 1;	

// clear mask registers
  FIO0MASK = 0;
  FIO1MASK = 0;
  FIO2MASK = 0;
  FIO3MASK = 0;
  FIO4MASK = 0;

// Reset all GPIO pins to default primary function
  PINSEL0 = 0;
  PINSEL1 = 0;
  PINSEL2 = 0;
  PINSEL3 = 0;
  PINSEL4 = 0;
  PINSEL5 = 0;
  PINSEL6 = 0;
  PINSEL7 = 0;
  PINSEL8 = 0;
  PINSEL9 = 0;
  PINSEL10= 0;
}


__fiq __arm void FIQ_Handler(void);
__irq __arm void IRQ_Handler(void);

/*************************************************************************
 * Function Name: FIQ_Handler
 * Parameters: none
 *
 * Return: none
 *
 * Description: FIQ handler
 *
 *************************************************************************/
__fiq __arm void FIQ_Handler (void)
{
  while(1);
}

/*************************************************************************
 * Function Name: IRQ_Handler
 * Parameters: none
 *
 * Return: none
 *
 * Description: IRQ handler
 *
 *************************************************************************/
__irq __arm void IRQ_Handler (void) {

  if (VICADDRESS != 0) {           // if handler assigned
    (*(pVoidFunc_t)VICADDRESS)();  // call the handler
  }
  VICADDRESS = 0;                  // clear the interrupt
}


/*************************************************************************
 * Function Name: vicInit
 * Parameters: void
 * Return: void
 *
 * Description: Initialize VIC
 *
 *************************************************************************/
void vicInit(void)
{
volatile unsigned long *pVecAddr;
volatile unsigned long *pVecPrio;
int i;
  // Assign all interrupt channels to IRQ
  VICINTSELECT  =  0;
  // Disable all interrupts
  VICINTENCLEAR = 0xFFFFFFFF;
  // Clear all software interrupts
  VICSOFTINTCLEAR = 0xFFFFFFFF;
  // VIC registers can be accessed in User or privileged mode
  VICPROTECTION = 0;
  // Clear interrupt
  VICADDRESS = 0;

  // Clear address of the Interrupt Service routine (ISR) for vectored IRQs
  // and assign equal low priorities to all interrupt sources
  for(i = 0, pVecAddr = &VICVECTADDR0,  pVecPrio = &VICVECTPRIORITY0; i < 32; ++i) {
    *pVecAddr++ = 0;
    *pVecPrio++ = 0x0F;
  }
}

/*************************************************************************
 * Function Name: vicInstallIRQhandler
 * Parameters:  pVoidFunc_t pISR
 *              uint32_t    priority
 *              uint32_t    source
 *
 * Return: void
 *
 * Description:  Install VIC interrupt handler
 *
 *************************************************************************/
void vicInstallIRQhandler(pVoidFunc_t pISR, uint32_t priority,
                          uint32_t vicIntSource) {

  // store the handler address in the correct slot in the vector 
  *(&VICVECTADDR0 + vicIntSource) = (unsigned long)pISR;
  
  // set the priority of the interrupt for this slot
  *(&VICVECTPRIORITY0 + vicIntSource) = priority;
  
  // clear FIQ select bit i.e. assign this interrupt source to IRQ
  VICINTSELECT &= ~(1UL << vicIntSource);
  
  // enable the interrupt
  VICINTENABLE |= (1UL << vicIntSource);
}



void lowLevelInit() {
  mamInit();
  clockInit();
  gpioInit();
  vicInit();
}

void bspInit(void) {
  lowLevelInit();
  ledsInit();
  buttonsInit();
  potentiometerInit();
  accelerometerInit();
  lcdInit();
  canInit();
}