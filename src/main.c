#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// -----------------------------------------------------------------------------
// Candidate Assignment: AVR LED Mode Controller
// -----------------------------------------------------------------------------
// Objective
// Implement a small AVR C program that changes LED blinking behavior based on a
// switch input. Use the AVR GCC toolchain to build and Wokwi VSCode Extension
// to simulate the result.
//
// What is Wokwi?
// - Wokwi is a hardware simulator that lets you run embedded firmware without
//   a physical board.
// - Example: https://wokwi.com/projects/461302817329780737
// - In this assignment, you will compile your AVR program, then load the
//   generated .elf or .hex into Wokwi to verify LED behavior with the switch.
// - This means your submission can be validated consistently even without
//   real hardware.
//
// Target Platform
// - MCU: ATmega328P
// - Language: C
// - Toolchain: AVR GCC
// - Simulator: Wokwi VSCode Extension
//
// Functional Requirements
// 1. Switch input controls blinking mode.
// 2. If the switch is LEFT: blink RED LED every 500 ms.
// 3. If the switch is RIGHT: blink GREEN and BLUE LEDs every 500 ms.
// 4. Use an interrupt to detect switch changes and update the active mode.
//
// Note:
// 1. Reference build artifacts .elf and .hex are provided.
// 2. Complete main.c and simulate by editing wokwi.toml to point to your generated .elf or .hex file.
//
// Pin Mapping
// - Switch: pin 5
// - RED LED: pin 13
// - GREEN LED: pin 12
// - BLUE LED: pin 11
//
//
// Minimum Deliverables
// 1. Git repository with working implementation in this file.
//    - repo should include a README with build instructions and any notes.
//    - diagram.json should be included for Wokwi simulation.
//    - .elf or .hex file generated from your build process.
//    - source file used to build the .elf or .hex.
// 2. Behavior aligned with reference implementation according to requirements.
// 3. Build notes at the top of the file, including compiler flags and commands.
// 4. Testability support with test-build and release-build configurations.
// 5. Comments for any assumptions or tradeoffs.
//
// Build and Simulation (high level)
// 1. Build with AVR GCC and generate .elf or .hex.
// 2. Update wokwi.toml to point to your generated artifact.
// 3. Open the Wokwi diagram in VSCode and start the simulation.
// 4. Toggle the virtual switch and verify both required blinking modes.
//
// Evaluation Notes
// - The checklist above is a baseline.
// - You may add improvements or optimizations as long as core behavior remains
//   correct and requirements are met.
// - comment any assumptions and tradeoffs in your implementation
// - testability and maintainability will be considered in evaluation
//
// Helpful Resources
// - AVR GCC toolchain:
//   https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers
// - Wokwi VSCode getting started:
//   https://docs.wokwi.com/vscode/getting-started
// - ATmega328P quick reference:
//   https://github.com/amirbawab/AVR-cheat-sheet

//Project Global Variables
uint16_t toggleTime = 32768; //LED on and off period 
uint16_t readVariable = 0; //LED on and off period

/**
 * @brief Reads the Output compare register TCNT1 SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be read from
 * @modifies Disables global interrupt for the time 
 */
void TIM16_ReadTCNT1(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts
    /* Read TCNT1 into i */
    *i = TCNT1; //Reads the full 16 bit
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Writes the Output compare register TCNT1 SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be written from
 * @modifies Disables global interrupt for the time 
 */
void TIM16_WriteTCNT1(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;cli(); // Disable interrupts
    
    /* Set TCNT1 to i */
    TCNT1 = *i;
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Reads the Output compare register OCR1A SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be read to
 * @modifies Disables global interrupt for the time 
 */
void TIM16_ReadOCR1A(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts
    /* Read TCNT1 into i */
    *i = OCR1A; //Reads the full 16 bit
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Writes the Output compare register OCR1A SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be written from
 * @modifies Disables global interrupt for the time 
 */
void TIM16_WriteOCR1A(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts
    /* Set TCNT1 to i */
    OCR1A = *i;
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Writes the Output compare register OCR1B SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be written from
 * @modifies Disables global interrupt for the time 
 */
void TIM16_WriteOCR1B(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts
    /* Set TCNT1 to i */
    OCR1B = *i;
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Initializes the system clock with a 256 prescalar
 *        System clock: F_CPU/256
 * @modifies CLKPR Clock Prescale Register
 */
void system_clock_init(){
    //The CLKPCE bit must be written to logic one to enable change of the CLKPS bits. The CLKPCE bit is only
    //updated when the other bits in CLKPR are simultaneously written to zero
    CLKPR = (1 << CLKPCE); //Eanbles Makes CLKPR write (other bits in CLKPR zero)
    CLKPR = (1 << CLKPS3); //256 Clock Division Factor and CLKPCE set LOW, and must set CLKPCE LOW
}

void set_PB1_output(){
    DDRB |= (1 << DDB1); // 1. Set PB1 (OC1A) as output
}

void set_PB2_output(){
    DDRB |= (1 << DDB2); // 1. Set PB1 (OC1B) as output
}

/**
 * @brief Initializes the timer 1 peripheral
 * @modifies TCCR1A, TCCR1B, OCR1A, OCR1B
 */
void timer1_init() {
    TCCR1A &= ~(1 << WGM10); //Waveform Generation Mode Bit
    TCCR1A &= ~(1 << WGM11); //Waveform Generation Mode Bit

    TCCR1B &= ~(1 << ICNC1); //Disable Input Capture Noise Canceler
    TCCR1B &= ~(1 << WGM13); //Waveform Generation Mode: Set Timer 1 to CTC mode (WGM12 = 1)
    TCCR1B |= (1 << WGM12); //Waveform Generation Mode

    //Set the same toggle value for both pins
    TIM16_WriteOCR1A(&toggleTime); //Sets the Ouput compare value OCR1A
    TIM16_WriteOCR1B(&toggleTime); //Sets the Ouput compare value OCR1B

    //Timer starts when we set clock source - 0b000 No clock source (Timer/Counter stopped).
    TCCR1B |= (1 << CS10); //Clock Select: clkI/O/1 (No prescaling)
    TCCR1B &= ~(1 << CS11); //Clock Select: clkI/O/1 (No prescaling)
    TCCR1B &= ~(1 << CS12); //Clock Select: clkI/O/1 (No prescaling)
}

/**
 * @brief Initializes the interrupt connected to the toggle button
 * @modifies PRR, SMCR
 */
void PD5_interrupt_init() {
    DDRD &= ~(1 << DDD5); //PD5 as input
    PORTD |= (1 << PORTD2); //Enable internal pull-up

    //Pin 5 PD5 is PCINT21
    PCICR |= (1 << PCIE2); //Pin Change Interrupt Control Register, Enable the PCIE2 group
    PCMSK2 |= (1 << PCINT21); //Pin Change Mask Register 2, mask for pin PD5 (PCINT21)

    // 5. Global Interrupt Enable
    sei();
}

// ---- ISR Code ----
ISR(PCINT2_vect)
{
    //Global interrupt disabled automatically

    if (PIND & (1 << PIND5)) {
        //Toggle switch is LEFT HIGH
        TCCR1A |=  (1 << COM1A0); //Activate OC1A toggle
        TCCR1A &= ~(1 << COM1B0); //Deactivate OC1B toggle
    } else if(!(PIND & (1 << PIND5))){
        //Toggle switch is RIGHT LOW
        TCCR1A &= ~(1 << COM1A0); //Deactivate OC1A toggle
        TCCR1A |=  (1 << COM1B0); //Activate OC1B toggle
    }

    //Global interrupt enabled automatically
}

/**
 * @brief Finds out which way the switch is innitally and starts to toggle
 *        the appropriate colors
 * @modifies TCCR1A Clock Prescale Register
 */
void color_toggle_init(){
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts

    //Find out which way the toggle swicth is initally and start LED toggle
    if (PIND & (1 << PIND5)) {
        //Toggle switch is LEFT HIGH, start RED LED toggle
        TCCR1A |=  (1 << COM1A0); //Activate OC1A toggle
        TCCR1A &= ~(1 << COM1B0); //Deactivate OC1B toggle
    } else {
        //Toggle switch is RIGHT LOW, start GREEN and BLUE LED toggle
        TCCR1A &= ~(1 << COM1A0); //Deactivate OC1A toggle
        TCCR1A |=  (1 << COM1B0); //Activate OC1B toggle
    }

    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Enables Sleep mode and turns off all other peripherals
 * @modifies PRR, SMCR
 */
void enter_power_save_mode(){
    SMCR = (1 << SE); //Sleep Mode Control Register SE: Sleep Enable
    //Shut off all peripherals except TIMER1
    PRR = (1 << PRTWI) | (1 << PRTIM2) | (1 << PRTIM0) | (1 << PRADC) | (1 << PRUSART0) | (1 << PRSPI);
}

int main(void) {
    set_PB1_output(); //Initializes pins
    set_PB2_output(); //Initializes pins
    system_clock_init(); //Initialize systme clock 
    timer1_init(); //Initializes and starts the timers
    color_toggle_init(); //Sets up initial LED toggle
    PD5_interrupt_init(); //Initializze interrupt on PD5
    
    while (1) {
        enter_power_save_mode();

    }
    return 0;
}