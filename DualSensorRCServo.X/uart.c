#include <stdio.h>
#include "uart.h"
#include "BOARD.h"
#include <xc.h>
#include "sys/attribs.h"
#include <string.h> 

#define BUFFSIZE 256
#define Baud 115200

// #define tester

typedef struct {
    char buffer[BUFFSIZE];
    volatile int head;
    volatile int tail;
    uint8_t Full;
} CircleBuff;

static CircleBuff rxBuffer = {{0}, 0, 0, 0};
static CircleBuff txBuffer = {{0}, 0, 0, 0};

void Uart_Init(unsigned long baudRate) {
    U1MODE = 0x00;
    U1STA = 0x00;
    U1BRG = ((BOARD_GetPBClock()) / (16 * baudRate)) - 1;
    
    U1MODEbits.ON = 1;
    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;
    
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;
    
    U1STAbits.UTXISEL = 0b00; 
    U1STAbits.URXISEL = 0b00;

    IEC0bits.U1RXIE = 1;
    IEC0bits.U1TXIE = 1;
    IPC6bits.U1IP = 0b110;
    IPC6bits.U1IS = 0b11;
}

int Buff_PutChar(volatile CircleBuff* buffchar, char ch) {
    if (!buffchar->Full) {
        buffchar->buffer[buffchar->tail] = ch;
        buffchar->tail = (buffchar->tail + 1) % BUFFSIZE;
        if (buffchar->tail == buffchar->head) {
            buffchar->Full = 1;
        }
        return 1;
    }
    return 0;
}

unsigned char Buff_GetChar(volatile CircleBuff* buffchar) {
    if ((!buffchar->Full) && (buffchar->head == buffchar->tail)) {
        return '\0';
    }
   
    unsigned char CharFromBuffer = buffchar->buffer[buffchar->head];
    
    buffchar->head = (buffchar->head + 1) % BUFFSIZE;
    
    if (buffchar->head == buffchar->tail) {
        buffchar->Full = 0;
    }
    
    return CharFromBuffer;
}

int PutChar(char ch) {
    int Result = Buff_PutChar(&txBuffer, ch);
    IFS0bits.U1TXIF = 1;
    return Result;
}

unsigned char GetChar(void) {
    return Buff_GetChar(&rxBuffer);
}

void _mon_putc(char c) {
    PutChar(c);
}

// if needed added during lab 2

unsigned int convertEndian(unsigned int *num) {
    unsigned int value = *num;
    return ((value >> 24) & 0x000000FF) | 
           ((value >> 8)  & 0x0000FF00) | 
           ((value << 8)  & 0x00FF0000) | 
           ((value << 24) & 0xFF000000);
}


void __ISR(_UART1_VECTOR) IntUart1Handler(void) {
    if (IFS0bits.U1RXIF) {
        while (U1STAbits.URXDA) {
            unsigned char DataIn = U1RXREG;
            Buff_PutChar(&rxBuffer, DataIn);
        }        
        IFS0bits.U1RXIF = 0;

    }
    
    if (IFS0bits.U1TXIF) {
        while (!U1STAbits.UTXBF) {
            if ((txBuffer.head == txBuffer.tail) && (!txBuffer.Full)) {
                break;
            }
            else U1TXREG = Buff_GetChar(&txBuffer);
            continue;
        }        
        IFS0bits.U1TXIF = 0;

    }
}

#ifdef tester
int main(void) {
    BOARD_Init();
    Uart_Init(Baud);

    unsigned char receivedData;
    
    while(1) {
        receivedData = GetChar();
        if (receivedData != '\0') {
            PutChar(receivedData);
        } 
    }
    return 0;
}
#endif