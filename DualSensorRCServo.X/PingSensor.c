#include <proc/p32mx340f512h.h>
#include <proc/PIC32MX/p32mx340f512h.h>
#include <sys/attribs.h>
#include "PingSensor.h"
#include "Protocol2.h"
#include "BOARD.h"
#include "MessageIDs.h"
#include "FreeRunningTimer.h"

// #define PING_SENSOR_TEST

static unsigned int timeDiff;
static unsigned int risingEdge;
static unsigned int fallingEdge;
unsigned int dist;

/**
 * @Function PingSensor_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief Initializes the HC-SR04 ultrasonic sensor hardware, setting up timers and input capture.
 */
int PingSensor_Init(void) {
    T4CON = 0x0;
    T2CON = 0x0;
    IC3CON = 0;
    IC3BUF = 0;
    
    // Configure Timer 4 for TRIG pulse timing
    T4CONbits.TCKPS = 0b110;  // Set prescaler to 1:64
    TMR4 = 0;                 // Reset Timer 4 counter
    PR4 = 7;                  // Set period to generate a ~60ms rollover (TRIG pulse width)

    IPC4bits.T4IP = 4;        // Set Timer 4 interrupt priority
    IPC4bits.T4IS = 3;        // Set Timer 4 sub-priority
    IFS0bits.T4IF = 0;        // Clear Timer 4 interrupt flag
    IEC0bits.T4IE = 1;        // Enable Timer 4 interrupts

    T2CONbits.TCKPS = 0b110;  // Set prescaler to 1:64
    TMR2 = 0;                 // Reset Timer 2 counter
    PR2 = 37500;              // Set period register for timing pulse measurements

    IC3CONbits.ICTMR = 1;     // Use Timer 2 as the capture timer source
    IC3CONbits.ICM = 0b001;   // Capture on every edge (both rising and falling)
    IC3CONbits.ICI = 0b00;    // Interrupt on every capture event

    IPC3bits.IC3IP = 6;       // Set Input Capture interrupt priority
    IPC3bits.IC3IS = 3;       // Set Input Capture interrupt sub-priority
    IFS0bits.IC3IF = 0;       // Clear Input Capture interrupt flag
    IEC0bits.IC3IE = 1;       // Enable Input Capture interrupts

    T2CONbits.ON = 1;         // Enable Timer 2
    T4CONbits.ON = 1;         // Enable Timer 4
    IC3CONbits.ON = 1;        // Enable Input Capture 3

    LATDbits.LATD5 = 1;       // Set TRIG pin high initially
    TRISDbits.TRISD5 = 0;     // Configure TRIG as an output

    // Configure ECHO pin as input
    TRISDbits.TRISD10 = 1;    // Set RD10 (ECHO) as input

    return SUCCESS;
}

/**
 * @Function PingSensor_GetDistance(void)
 * @param None
 * @return Calculated distance in predefined units
 * @brief Returns the most recent calculated distance from the sensor.
 */
unsigned short PingSensor_GetDistance(void) {
    return dist;
}

#ifdef PING_SENSOR_TEST

/**
 * @Function main(void)
 * @param None
 * @return None
 * @brief Test program for the PingSensor module. Periodically measures and sends the distance via Protocol2.
 */
int main(void) {
    BOARD_Init();
    PingSensor_Init();
    FreeRunningTimer_Init();
    Protocol_Init(115200);
    
    unsigned short distance;
    unsigned int lastval;
    unsigned char val[2];

    while (1) {
        if ((FreeRunningTimer_GetMilliSeconds() - lastval) >= 100) {
            lastval = FreeRunningTimer_GetMilliSeconds();
            distance = PingSensor_GetDistance();

            val[1] = distance & 0xff;
            val[0] = (distance >> 8) & 0xff;

            Protocol_SendPacket(2, ID_PING_DISTANCE, val);        
        }
    }
}
#endif

/**
 * @Function Timer4IntHandler(void)
 * @brief Handles Timer 4 interrupts, toggling the TRIG signal for distance measurement.
 */
void __ISR(_TIMER_4_VECTOR) Timer4IntHandler(void) {
    if (PR4 == 37500) {
        PR4 = 7; 
        LATDbits.LATD5 = 1; 
    } else {  // switch to long pulse
        PR4 = 37500; 
        LATDbits.LATD5 = 0; 
    }
    
    IFS0bits.T4IF = 0;  
}

/**
 * @Function IC3InterruptHandler(void)
 * @brief Handles Input Capture 3 (IC3) interrupts, recording pulse timings and computing distance.
 */
void __ISR(_INPUT_CAPTURE_3_VECTOR) IC3InterruptHandler(void) {
    if (PORTDbits.RD10 == 1) {  // Rising edge
        risingEdge = (0xFFFF & IC3BUF);  
    }
    if (PORTDbits.RD10 == 0) {  // Falling edge 
        fallingEdge = (0xFFFF & IC3BUF);  
        timeDiff = fallingEdge - risingEdge;  // Compute pulse

        dist = ((340 * timeDiff) / (10000));
    }
    
    IFS0bits.IC3IF = 0;
}


