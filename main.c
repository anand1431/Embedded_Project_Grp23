#include <stdint.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "Waveforms.h" // Lookup tables for waveforms

void DisableInterrupts(void);
void EnableInterrupts(void);
void UART_Init(void);
void UART_OutString(char *pt);
void I2C0_Init(void);
void I2C0_Write(uint8_t device_addr, uint16_t data);
int getFrequencyFromUser(void);

extern int wt[1][120]; // Lookup tables for waveforms

void delay_32us();

// MCP4725 I2C address
#define MCP4725_ADDR 0x60

// Function to output the sine waveform
void OutputWaveform(int frequency) {
    int i, k = 0;
    int delayCycles = (int)(100000 / (96 * frequency)); // Delay adjustment based on frequency

    while (1) {
        // Send the next sample of the sine waveform to the DAC
        I2C0_Write(MCP4725_ADDR, wt[0][k]); // Always use sine waveform (wt[0])

        // Apply delay for the selected frequency
        for (i = 0; i < delayCycles; i++) {
            delay_32us();
        }

        // Increment sample index
        k = (k + 1) % 120;

        // Check if the user wants to change the frequency
        if ((UART0_FR_R & 0x10) == 0) {  // Non-blocking check if there is input in UART buffer
            break;  // Exit loop if user input is detected
        }
    }
}

int main(void) {
    int frequency = 10; // Default frequency

    // Initialize UART for console input
    UART_Init();
    I2C0_Init(); // Initialize I2C for DAC communication
    EnableInterrupts(); // Enable global interrupts

    while (1) {
        // Get user input for frequency
        frequency = getFrequencyFromUser();
        if (frequency <= 0) {
            UART_OutString("Invalid frequency. Please enter a positive integer.\n\n");
            continue;
        }

        UART_OutString("Generating sine waveform...\n\n");

        // Output the sine waveform with the selected frequency
        OutputWaveform(frequency);
    }
}

// Initialize UART0 for console input
void UART_Init(void) {
    SYSCTL_RCGCUART_R |= 0x01;   // Activate UART0
    SYSCTL_RCGCGPIO_R |= 0x01;   // Activate Port A
    UART0_CTL_R &= ~0x01;        // Disable UART0
    UART0_IBRD_R = 104;          // 9600 baud rate, assuming 16 MHz system clock
    UART0_FBRD_R = 11;
    UART0_LCRH_R = 0x0070;       // 8-bit word length, enable FIFO
    UART0_CTL_R = 0x301;         // Enable UART0
    GPIO_PORTA_AFSEL_R |= 0x03;  // Enable alt function on PA1-0
    GPIO_PORTA_DEN_R |= 0x03;    // Enable digital I/O on PA1-0
}

// Output string to UART
void UART_OutString(char *pt) {
    while (*pt) {
        while ((UART0_FR_R & 0x20) != 0);      // Wait until TXFF is empty
        UART0_DR_R = *pt;                      // Output character
        pt++;
    }
}

// Get frequency from the user via specific key presses
// Get frequency from the user via specific key presses
int getFrequencyFromUser(void) {
    char key;
    int frequency = 440; // Default frequency: A4

    // Print menu items sequentially
    UART_OutString("\n--- Piano Note Frequency Selection ---\n\n");
    UART_OutString("Press keys to select a piano note:\n\n");
    UART_OutString("1: A4 (440 Hz)\n");
    UART_OutString("2: B4 (494 Hz)\n");
    UART_OutString("3: C5 (523 Hz)\n");
    UART_OutString("4: D5 (587 Hz)\n");
    UART_OutString("5: E5 (659 Hz)\n");
    UART_OutString("6: F5 (698 Hz)\n");
    UART_OutString("7: G5 (784 Hz)\n");
    UART_OutString("8: A5 (880 Hz)\n\n");
    UART_OutString("Enter your choice: ");

    // Wait for user input
    while ((UART0_FR_R & 0x10) != 0); // Wait until a character is received
    key = UART0_DR_R;                // Read the received character

    // Output selection response
    UART_OutString("\n"); // Blank line to separate input from output

    switch (key) {
        case '1':
            frequency = 440; // A4
            UART_OutString("Selected frequency: A4 (440 Hz)\n\n");
            break;
        case '2':
            frequency = 494; // B4
            UART_OutString("Selected frequency: B4 (494 Hz)\n\n");
            break;
        case '3':
            frequency = 523; // C5
            UART_OutString("Selected frequency: C5 (523 Hz)\n\n");
            break;
        case '4':
            frequency = 587; // D5
            UART_OutString("Selected frequency: D5 (587 Hz)\n\n");
            break;
        case '5':
            frequency = 659; // E5
            UART_OutString("Selected frequency: E5 (659 Hz)\n\n");
            break;
        case '6':
            frequency = 698; // F5
            UART_OutString("Selected frequency: F5 (698 Hz)\n\n");
            break;
        case '7':
            frequency = 784; // G5
            UART_OutString("Selected frequency: G5 (784 Hz)\n\n");
            break;
        case '8':
            frequency = 880; // A5
            UART_OutString("Selected frequency: A5 (880 Hz)\n\n");
            break;
        default:
            UART_OutString("Invalid selection. Default frequency: A4 (440 Hz)\n\n");
            break;
    }

    return frequency;
}


// Initialize I2C0 for DAC communication
void I2C0_Init(void) {
    SYSCTL_RCGCI2C_R |= 0x01;    // Activate I2C0
    SYSCTL_RCGCGPIO_R |= 0x02;   // Activate Port B
    GPIO_PORTB_AFSEL_R |= 0x0C;  // Enable alt function on PB2-3
    GPIO_PORTB_ODR_R |= 0x08;    // Enable open drain on PB3
    GPIO_PORTB_DEN_R |= 0x0C;    // Enable digital I/O on PB2-3
    I2C0_MCR_R = 0x10;           // Initialize I2C Master
    I2C0_MTPR_R = 24;            // Set SCL clock speed
}

// Write 12-bit data to MCP4725 DAC
void I2C0_Write(uint8_t device_addr, uint16_t data) {
    I2C0_MSA_R = (device_addr << 1); // Set slave address and write
    I2C0_MDR_R = data >> 8;          // Transmit high byte first
    I2C0_MCS_R = 0x03;               // Start and run
    while (I2C0_MCS_R & 0x01);       // Wait for transmission
    I2C0_MDR_R = data & 0xFF;        // Transmit low byte
    I2C0_MCS_R = 0x05;               // Run and stop
    while (I2C0_MCS_R & 0x01);       // Wait for transmission
}

// 32 us delay function
void delay_32us() {
    SYSCTL_RCGCTIMER_R |= 2;         // Enable clock to Timer 1
    TIMER1_CTL_R = 0;                // Disable Timer 1
    TIMER1_CFG_R = 0x04;             // 16-bit timer
    TIMER1_TAMR_R = 0x02;            // Periodic mode
    TIMER1_TAILR_R = 512;            // Set interval for 32 us
    TIMER1_ICR_R = 0x01;             // Clear timeout flag
    TIMER1_CTL_R |= 0x01;            // Enable Timer 1
    while ((TIMER1_RIS_R & 0x01) == 0); // Wait for timeout
}

// Disable global interrupts
void DisableInterrupts(void) {
    __asm(" CPSID I");  // Disable interrupts
}

// Enable global interrupts
void EnableInterrupts(void) {
    __asm(" CPSIE I");  // Enable interrupts
}
