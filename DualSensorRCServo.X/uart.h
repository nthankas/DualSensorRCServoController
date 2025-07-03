/***
* File: uart.h
* Author: Instructor
* Created: ECE121 W2022 rev 1
* This library implements a true UART device driver that enforces
* I/O stream abstraction between the physical and application layers.
* All stream accesses are on a per-character or byte basis.
*/

#ifndef UART_H
#define UART_H
/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/
#define BAUDRate 115200

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/
 
/**
 * @Function Uart_Init(unsigned long baudrate)
 * @param baudrate
 * @return none
 * @brief  Initializes UART1 to baudrate N81 and creates circ buffers
 * @author instructor ece121 W2022 */
void Uart_Init(unsigned long baudRate);

/**
 * @Function int PutChar(char ch)
 * @param ch - the character to be sent out the serial port
 * @return True if successful, else False if the buffer is full or busy.
 * @brief  adds char to the end of the TX circular buffer
 * @author instrutor ECE121 W2022 */
int PutChar(char ch);

/**
 * @Function unsigned char GetChar(void)
 * @param None.
 * @return NULL for error or a Char in the argument.
 * @brief  dequeues a character from the RX buffer,
 * @author instructor, ECE121 W2022 */
unsigned char GetChar(void);

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/

/****************************************************************************
 * Function: IntUart1Handler
 * Parameters: None.
 * Returns: None.
 * The PIC32 architecture calls a single interrupt vector for both the 
 * TX and RX state machines. Each IRQ is persistent and can only be cleared
 * after "removing the condition that caused it".  
 ****************************************************************************/
//void __ISR(_UART1_VECTOR) IntUart1Handler(void);
    //UxRXIF

/**
 * Parameter: c - char to be sent
 * Overwrites weakly defined extern to use the TX circular buffer
 * instead of Microchip functions
 */ 
void _mon_putc(char c);
	
/**
 * Parameter: s - pointer to the string to be sent.
 * Overwrites weakly defined extern to use the RX 
 * circular buffer instead of Microchip functions
 */
void _mon_puts(const char *s);

/****************************************************************************
 * Function: IntUart1Handler
 * Parameters: None.
 * Returns: None.
 * The PIC32 architecture calls a single interrupt vector for both the 
 * TX and RX state machines. Each IRQ is persistent and can only be cleared
 * after "removing the condition that caused it". This function is declared in
 * sys/attribs.h. 
 ****************************************************************************/
//void __ISR(_UART1_VECTOR) IntUart1Handler(void);




#endif // UART_H

