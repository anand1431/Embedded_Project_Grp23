#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include <ctype.h>

#define RED_LED      (1U << 1) // PF1
#define BLUE_LED     (1U << 2) // PF2
#define GREEN_LED    (1U << 3) // PF3

void UART0_Init(void) {
    // Enable UART0 and GPIO Port A
    SYSCTL_RCGCUART_R |= (1U << 0);   // Enable UART0
    SYSCTL_RCGCGPIO_R |= (1U << 0);    // Enable GPIO Port A

    // Configure PA0 and PA1 for UART
    GPIO_PORTA_AFSEL_R |= (1U << 0) | (1U << 1); // Set PA0 and PA1 as alternate function
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) | 0x00000011; // Set PA0 and PA1 for UART
    GPIO_PORTA_DEN_R |= (1U << 0) | (1U << 1); // Enable digital function on PA0 and PA1

    // Configure UART0 for 9600 baud rate, 8-N-1
    UART0_CTL_R &= ~UART_CTL_UARTEN; // Disable UART
    UART0_IBRD_R = 104;               // Integer part of BRD
    UART0_FBRD_R = 11;                // Fractional part of BRD
    UART0_LCRH_R = UART_LCRH_WLEN_8;  // 8 bits, no parity, 1 stop bit
    UART0_CTL_R |= UART_CTL_UARTEN;   // Enable UART
}

char UART0_Read(void) {
    // Wait until the receive FIFO is not empty
    while ((UART0_FR_R & UART_FR_RXFE) != 0); // RXFE (Receive FIFO Empty) flag
    return (char)(UART0_DR_R & 0xFF); // Read and return the character
}

void UART0_Write(char data) {
    // Wait until the transmit FIFO is not full
    while ((UART0_FR_R & UART_FR_TXFF) != 0); // TXFF (Transmit FIFO Full) flag
    UART0_DR_R = data; // Send the character
}

void LED_Init(void) {
    // Enable GPIO Port F
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;   // Enable clock for Port F

    // Set PF1, PF2, PF3 as output
    GPIO_PORTF_DIR_R |= RED_LED | BLUE_LED | GREEN_LED;
    GPIO_PORTF_DEN_R |= RED_LED | BLUE_LED | GREEN_LED;
}

void LED_Control(char note) {
    GPIO_PORTF_DATA_R &= ~(RED_LED | BLUE_LED | GREEN_LED); // Turn off all LEDs

    // Map each note to a specific LED using if-else
    if (note == 'a') {
        GPIO_PORTF_DATA_R |= RED_LED; // Turn on RED LED for note 'a'
    }
    else if (note == 's') {
        GPIO_PORTF_DATA_R |= GREEN_LED; // Turn on GREEN LED for note 's'
    }
    else if (note == 'd') {
        GPIO_PORTF_DATA_R |= BLUE_LED; // Turn on BLUE LED for note 'd'
    }
    else if (note == 'f') {
        GPIO_PORTF_DATA_R |= RED_LED; // Turn on RED LED for note 'f'
    }
    else if (note == 'g') {
        GPIO_PORTF_DATA_R |= GREEN_LED; // Turn on GREEN LED for note 'g'
    }
    else if (note == 'h') {
        GPIO_PORTF_DATA_R |= BLUE_LED; // Turn on BLUE LED for note 'h'
    }
    else if (note == 'j') {
        GPIO_PORTF_DATA_R |= RED_LED; // Turn on RED LED for note 'j'
    }
    else if (note == 'k') {
        GPIO_PORTF_DATA_R |= GREEN_LED; // Turn on GREEN LED for note 'k'
    }
}

int main(void) {
    UART0_Init();  // Initialize UART0
    LED_Init();    // Initialize LEDs

    while (1) {
        char receivedNote = UART0_Read(); // Read character from UART

        if (receivedNote) {                // If a character was received
            UART0_Write(receivedNote);     // Echo back the character
            LED_Control(receivedNote);     // Control LEDs based on received character
        }
    }
}
