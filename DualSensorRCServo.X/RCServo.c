#include "RCServo.h"
#include "MessageIDs.h"
#include "Protocol2.h"
#include "uart.h"
#include "BOARD.h"
#include "RotaryEncoder.h"
#include "FreeRunningTimer.h"
#include <xc.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>

#define RCTest
#define BAUDRate 115200

unsigned int pulseW;

int RCServo_Init() {
    T3CON = 0x0; // timer 3 reset + OC3 reset
    OC3CON = 0;
    TMR3 = 0x000;
    // 50 ms period for PR3
    PR3 = 62500 - 1;
    
    T3CONbits.TCKPS = 0b101;
    
    
    OC3CONbits.OCM = 0b101;
    OC3R = 0;
    OC3RS = 0;      
    OC3CONbits.OCTSEL = 1;
    
    pulseW = RC_SERVO_CENTER_PULSE;
    
    IEC0bits.OC3IE = 0;
    IPC3bits.OC3IP = 6;
    IPC3bits.OC3IS = 3;
    IFS0bits.OC3IF = 0;

    OC3CONbits.ON = 1;
    T3CONbits.TON = 1;
    
    return 1;
    
}


int RCServo_SetPulse(unsigned int inPulse) {
    if (inPulse < RC_SERVO_MIN_PULSE) {
        pulseW = RC_SERVO_MIN_PULSE;
    }
    else if (inPulse > RC_SERVO_MAX_PULSE) {
        pulseW = RC_SERVO_MAX_PULSE;
    }
        pulseW = inPulse;
    OC3RS = pulseW - 1;
    return SUCCESS;
}

unsigned int RCServo_GetPulse(void) {
    return pulseW;
}

unsigned int RCServo_GetRawTicks(void) {
    return OC3RS;
}

void __ISR(_OUTPUT_COMPARE_3_VECTOR) __OC3INTERRUPT(void) {
    if (IEC0bits.OC3IE) {
        OC3RS = pulseW;
    }
    IFS0bits.OC3IF = 0;
}

#ifdef RCTest

int main(void) {
    BOARD_Init();
    Protocol_Init(115200);    
    
    
    if (RCServo_Init() != SUCCESS) {
        Protocol_SendDebugMessage("RCSERVO initialization failed.");
        while (1);
    }
    
    char debugS[200];
    sprintf(debugS, "RC Servo test harness compiled at %s %s", __DATE__, __TIME__);
    Protocol_SendDebugMessage(debugS);
    
    packet testpack = {0};
    
    while(1){
        
        if(BuildRxPacket(&testpack, 0)){
            if (Protocol_QueuePacket(testpack)){
                Protocol_SendDebugMessage("Packet Queue full");
            }
        }
        
        packet* inpack;
        if (Protocol_GetInPacket(&inpack)){
            Protocol_ParsePacket(*inpack);
        }
        
        int i;
        for (i = 0; i < 100000; i++) {
            asm("nop");
        }
    }
    
    return 0;
}

#endif