
#include "RotaryEncoder.h"
#include "MessageIDs.h"
#include "Protocol2.h"
#include <xc.h>
#include <sys/attribs.h>
#include <stdio.h>
#include "BOARD.h"
#include "uart.h"
#include <stdlib.h>

#define PBCLK 40000000
#define READANGLE_COMM 0xFFFF
#define CS LATDbits.LATD0

// #define RotaryTest

static int rec;

// private functions

void SPI_Init() {
        TRISGbits.TRISG6 = 0;
        TRISFbits.TRISF2 = 1;
        TRISFbits.TRISF3 = 0;
        TRISDbits.TRISD0 = 0;


        SPI1CON = 0;
        
        int rec;
        rec = SPI1BUF;
        SPI1BRG = 0x1;

        SPI1STATCLR = 0x40;
        SPI1CONbits.CKE = 1;
        SPI1CONbits.MODE16 = 1;
        SPI1CONbits.SMP = 1;
        SPI1CONbits.MSTEN = 1;
        SPI1CONbits.ON = 1;

}

unsigned short SPIRead(unsigned short x) {
    while (!SPI1STATbits.SPITBE);
    SPI1CONbits.SSEN = 0;
    SPI1BUF = x;
//    char hah[240];
//    sprintf(hah, "x is %d \n", x);
//    Protocol_SendDebugMessage(hah);
    while (!SPI1STATbits.SPIRBF);
    return SPI1BUF;
}   

// public

int RotaryEncoder_Init(char interfaceMode) {
    SPI_Init();
    return 1;
}

unsigned short RotaryEncoder_ReadRawAngle(void) {
    CS = 0;
    unsigned short rawang = SPIRead(READANGLE_COMM) & 0x3FFF;
    CS = 1;
    
    return rawang;
}


#ifdef RotaryTest

int main(int argc, char** argv) {
    BOARD_Init();
    Protocol_Init(115200);
    RotaryEncoder_Init(ENCODER_BLOCKING_MODE);
    
    char status[100];
    
    sprintf(status, "Rotary Test compiled at %s %s.", __DATE__, __TIME__);  
    Protocol_SendDebugMessage(status);
    unsigned char pay[MAXPAYLOADLENGTH];
    
    while (1) {
        unsigned short value = RotaryEncoder_ReadRawAngle();
        unsigned short angle = (((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8));
        Protocol_SendPacket(2, ID_ROTARY_ANGLE, &angle);
        for (int i = 0; i < 10; i++) {
            for (int j = 0; i < 80000; i++) {
                asm("nop");
            }
        }
    }
    
    return 0;
}
#endif