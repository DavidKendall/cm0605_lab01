#include <stdint.h>
#include <bsp.h>

typedef enum timerIdentifier {TIMER0, TIMER1} timerIdentifier_t;

void initTimer0(pVoidFunc_t handler, uint32_t ticksPerSec);
void startTimer0(void);
void stopTimer0(void);

inline void clearInterruptTimer0(void) {
  T0IR = 1;
}
