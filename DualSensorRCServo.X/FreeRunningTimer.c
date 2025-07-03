#include <proc/p32mx340f512h.h>

#include "FreeRunningTimer.h"
#include "BOARD.h"
#include "Protocol2.h"
#include "uart.h"

// #define testerFRT
// #define testMilliseconds


#define BAUDrate 115200

typedef uint16_t tmrcount;
tmrcount mscount;


void __ISR(_TIMER_5_VECTOR, IPL3AUTO) Timer5IntHandler(void) {
    TMR5 = 0;
    mscount++;
    
    IFS0bits.T5IF = 0;
    return;
}

/**
 * @Function FreeRunningTimer_Init(void)
 * @param none
 * @return None.
 * @brief  Initializes the timer module */
void FreeRunningTimer_Init(void) {
    T5CONbits.ON = 0;
    T5CON |= 0x4000;
    T5CONbits.TCKPS = 0b010;
    
    TMR5 = 0;
    PR5 = 0x2710;
    mscount = 0;
    
    IEC0bits.T5IE = 0;
    IFS0bits.T5IF = 0;
    IPC5bits.T5IP = 3;
    IPC5bits.T5IS = 0;
    IEC0bits.T5IE = 1;
    
    T5CONbits.ON = 1;
    
    return;
    
}

/**
 * Function: FreeRunningTimer_GetMilliSeconds
 * @param None
 * @return the current MilliSecond Count
   */
unsigned int FreeRunningTimer_GetMilliSeconds(void) {
    return mscount;
}

/**
 * Function: FreeRunningTimer_GetMicroSeconds
 * @param None
 * @return the current MicroSecond Count
   */
unsigned int FreeRunningTimer_GetMicroSeconds(void) {
    return ((mscount * 1000) + TMR5);
}

#ifdef testerFRT
int main(void) {
    BOARD_Init();
    LEDS_INIT();
    Protocol_Init(BAUDrate);
    
    FreeRunningTimer_Init();
    
    tmrcount halfsec = 0;
    tmrcount twosec = 0;
    
    while (1) {
        tmrcount current = FreeRunningTimer_GetMilliSeconds();
        #if defined(testMilliseconds)

            if (current - halfsec >= 500) {
                halfsec = current;
                LATEbits.LATE0 ^= 1;
            }
        if (current - twosec >= 2000) {
            char disp[100];
            sprintf(disp, "Timer is at %d ms, %d microseconds", FreeRunningTimer_GetMilliSeconds(), FreeRunningTimer_GetMicroSeconds());
            Protocol_SendDebugMessage(disp);
            twosec = current;
        }
        #endif
            
    }
    return;
}
#endif
