#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <string.h>

#include "BOARD.h"
#include "uart.h"
#include "Protocol2.h"
#include "MessageIDs.h"
#include "PingSensor.h"
#include "FreeRunningTimer.h"
#include "RCServo.h"
#include "RotaryEncoder.h"

// Define which part of Lab 2 we?re implementing.
#define Part_5

// Mode definitions: MODE_PING (0) for ping sensor, MODE_DIAL (1) for rotary encoder.
#define MODE_PING 0
#define MODE_DIAL 1

// Mapping constants.
#define Servo_Min 600        // Minimum servo pulse width in microseconds
#define Servo_Max 2400       // Maximum servo pulse width in microseconds
#define Servo_Diff (Servo_Max - Servo_Min)  // 1800 us difference

#define Encoder_Max 16384    // Maximum raw encoder value (14-bit)

// For the ping sensor, we assume the sensor returns distance in millimeters.
// Our mapping constants are in centimeters: 25 cm to 125 cm.
#define Ping_Min_cm 25       
#define Ping_Max_cm 125      

// Flush the UART RX buffer to remove stray echoes.
void FlushUARTRx(void) {
    while(U1STAbits.URXDA) {
        (void)GetChar();
    }
}

#ifdef Part_5
int main(void)
{
    // Initialize all modules.
    BOARD_Init();
    Uart_Init(115200);
    Protocol_Init(115200);
    FreeRunningTimer_Init();
    PingSensor_Init();
    RotaryEncoder_Init(ENCODER_BLOCKING_MODE);
    RCServo_Init();
    
    // Send an initial debug message.
    char Message[128];
    sprintf(Message, "Lab 2 Application compiled at %s, %s", __DATE__, __TIME__);
    Protocol_SendDebugMessage(Message);
    
    // Default mode: start in PING mode.
    int Mode = MODE_PING;
    int Pulse_Width = 0;
    unsigned short rawPing_mm = 0;
    unsigned short Distance_cm = 0;
    unsigned short EncoderVal = 0;
      
    // Use a timer to update every ~50ms.
    unsigned int lastUpdate = FreeRunningTimer_GetMilliSeconds();
    packet Test_Packet = {0};
    
    while(1) {
        // Flush any stray RX data.
        FlushUARTRx();
        
        // Process incoming command packets.
        if(BuildRxPacket(&Test_Packet, 0)) {
            if(Test_Packet.ID == ID_LAB2_INPUT_SELECT) {
                // Payload 0 selects PING mode; nonzero selects DIAL mode.
                if(Test_Packet.payLoad[0] == 0) {
                    Mode = MODE_PING;
                    Protocol_SendDebugMessage("Switched to PING mode");
                } else {
                    Mode = MODE_DIAL;
                    Protocol_SendDebugMessage("Switched to DIAL mode");
                }
            }
        }
        
        // Update every 50ms.
        if(FreeRunningTimer_GetMilliSeconds() - lastUpdate >= 50) {
            lastUpdate = FreeRunningTimer_GetMilliSeconds();
            
            if(Mode == MODE_PING) {
                // Get raw distance from ping sensor (in mm) then convert to cm.
                rawPing_mm = PingSensor_GetDistance();
                Distance_cm = rawPing_mm / 10;
                
                char dbg[64];
                sprintf(dbg, "PING mode: Raw = %u mm, Distance = %u cm", rawPing_mm, Distance_cm);
                Protocol_SendDebugMessage(dbg);
                
                // Clamp and map distance to pulse width.
                if(Distance_cm < Ping_Min_cm) { Distance_cm = Ping_Min_cm; }
                if(Distance_cm > Ping_Max_cm) { Distance_cm = Ping_Max_cm; }
                Pulse_Width = Servo_Min + (Servo_Diff * (Distance_cm - Ping_Min_cm)) / (Ping_Max_cm - Ping_Min_cm);
                
                sprintf(dbg, "PING mode: Mapped Pulse = %d us", Pulse_Width);
                Protocol_SendDebugMessage(dbg);
            }
            else if(Mode == MODE_DIAL) {
                // Read raw encoder value.
                EncoderVal = RotaryEncoder_ReadRawAngle();
                int degrees = (EncoderVal * 360) / (Encoder_Max - 1);
                Pulse_Width = Servo_Min + (Servo_Diff * degrees) / 360;
                
                char dbg[64];
                sprintf(dbg, "DIAL mode: Encoder = %u, Deg = %d, Pulse = %d us", EncoderVal, degrees, Pulse_Width);
                Protocol_SendDebugMessage(dbg);
            }
            
            // Set the RC servo output.
            RCServo_SetPulse(Pulse_Width);
            
            // Add a debug message to read back the pulse.
            int actualPulse = RCServo_GetPulse();
            {
                char dbg[64];
                sprintf(dbg, "RCServo pulse set to: %d us", actualPulse);
                Protocol_SendDebugMessage(dbg);
            }
            
            // Report the pulse width in a 2-byte packet (big-endian).
            unsigned char reportPayload[2];
            reportPayload[0] = (Pulse_Width >> 8) & 0xFF;
            reportPayload[1] = Pulse_Width & 0xFF;
            Protocol_SendPacket(2, ID_LAB2_ANGLE_REPORT, reportPayload);
        }
    }
    
    return (EXIT_SUCCESS);
}
#endif
