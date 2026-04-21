#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//AVRDUDE Command
///Users/guest1/Library/Arduino15/packages/arduino/tools/avrdude/8.0.0-arduino1/bin/avrdude -c digilent -p m328p -P usb -v-c digilent -p m328p

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