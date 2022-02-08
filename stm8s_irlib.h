#ifndef __STM8S_H_IRLIB
#define __STM8S_H_IRLIB


#include "stm8s103_serial.h"
#include <iostm8s103.h> 
#include "stm8s.h" // 

#define IR_SENSOR_PIN GPIOD,GPIO_PIN_3 
#define IR_SENSOR_PIN_READ (GPIOD->IDR & 0b00001000) // IR sensor Pin

#define TIMER4_OVERFLOW_INTERRUPT_FLAG TIM4_SR // 1 = TMR0 register has overflowed (must be cleared in software) 0 = TMR0 register did not overflow
#define START_TIMER4 TIM4_CR1 |=(1<<0); // macro to Start timer4
#define STOP_TIMER4 TIM4_CR1 &= 0x00; //  macro clear the CR1 Control register 


int loop_counter; //variable for loop counter

/*Variables to store IR Data*/
int first_start_pulse; // holds counts and verifies the first 9ms data in IR protocol
int seccound_verification_puls; // holds counts and verifies the secound 4.5ms data in IR protocol
uint16_t ir_received_data; 
uint16_t ir_raw_data;


/*Function prototypes  */
void ir_init();  
void delay_us(unsigned char time);
void delay_ms(unsigned int time);
void timer4_init(void);
void gpio_init(void);
void start_timer4(void);
void protocol_check(void);
int read_data(void);
int ir_result(int debug_info);
void serial_init(void);
void ir_init(void);


void ir_init()
{
	gpio_init(); // Enable Interrupt on GPIO
	timer4_init(); // Configure Timer 4
	start_timer4(); //Start Timer4
}

void gpio_init()
{
	GPIO_Init(IR_SENSOR_PIN,GPIO_MODE_IN_FL_IT); // Enable Interrupt on D3 Pin
	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD, EXTI_SENSITIVITY_FALL_ONLY); // Interrupt on Falling Edge
  EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_FALL_ONLY); // Interrupt on Falling Edge
}


void timer4_init()
{
	TIM4_SR = 0x00;// Clear Interrupt Overflow Flag
	TIM4_IER = 0x00;// Disable Timer-4 Overflow Interrupt
	TIM4_CR1 &= 0x00; //  Clear the Timer-4 control register 
	TIM4_CR1 |=(1<<2); // Update Request Source Generates Interrupt
	TIM4_EGR = 0x00;// Clear Event Generation Register
	TIM4_CNTR = 0x00;// Set Counter to zero, We need Counter for Delay
	TIM4_PSCR = 0x01;// Prescaler -> Clock/2 16/2 ->8MHz
	TIM4_IER = 0x01; // Enable Timer4 Interrupt
	TIM4_CR1 |=(1<<0); // Start Timer4
}


void start_timer4()
{
	enableInterrupts();// Enable Global Interrupt
	TIM4_IER = 0x01; // Enable Timer4 Interrupt
	TIM4_CR1 |=(1<<0); // Start Timer4
}


int ir_result(int debug_info) {
	
		
		GPIO_Init(IR_SENSOR_PIN,GPIO_MODE_IN_FL_NO_IT);	//configure thepin as standard IO
		
    protocol_check(); // call protocol_check function
    /*inside the protocol check function the timer counter checks the pulse width 
        if it is greater than 141 and less than 145 the statement is true. 
        you can do this calculation by taking the datasheet as reference*/
				
				
    if (first_start_pulse >= 33 && first_start_pulse <= 38) {
        if (seccound_verification_puls >= 16 && seccound_verification_puls <= 21) {
            read_data(); //call the function and get first 16 bit
            ir_received_data = read_data(); //call the function again and get second 16 bit
            if (debug_info) { //if set to 1 then print the data if 0 don't print the data
								Serial_print_string("IR Received Data: ");
              	Serial_print_int(ir_received_data);
								Serial_print_string("\n");
            }
        }
}

	gpio_init(); // reconfigure the GPIO as interrupt
	start_timer4();// because of the delay_us function the interrupt gets turned off we need to turn the interrupt on for a smooth operation
	
	return ir_received_data;
	
}


void protocol_check(void) {
	
    first_start_pulse = 0; // remove all data
    seccound_verification_puls = 0; // remove all data
	
    while (!IR_SENSOR_PIN_READ) {
        if (TIMER4_OVERFLOW_INTERRUPT_FLAG) {
            TIMER4_OVERFLOW_INTERRUPT_FLAG = 0;
            first_start_pulse++;
            if (first_start_pulse >= 40) {
                break;
            }
        }
    }
    while (IR_SENSOR_PIN_READ) {
        if (TIMER4_OVERFLOW_INTERRUPT_FLAG) {
            TIMER4_OVERFLOW_INTERRUPT_FLAG = 0;
            seccound_verification_puls++;
            if (seccound_verification_puls >= 20) {
                break;
            }
        }
    }
		//	Serial_print_int(first_start_pulse);
		//	Serial_print_string("\n");
		//	Serial_print_int(seccound_verification_puls);
		//	Serial_print_string("\n");
				
}


int read_data() {
	ir_raw_data = 0; 
		
    for (loop_counter = 0; loop_counter < 16; loop_counter++) { // the data coming from the sensor is 16 bit long so we need a for loop
        while (!IR_SENSOR_PIN_READ);
					 //Wait until PORT goes HIGH
       delay_ms(9);
        if (!IR_SENSOR_PIN_READ) {
            ir_raw_data &= ~(1 << (15 - loop_counter)); //Clear bit (7-b)
        } else {
            ir_raw_data |= (1 << (15 - loop_counter)); //Set bit (7-b)
            while (IR_SENSOR_PIN_READ)
                ;
        } //Wait until PORT goes LOW
    }
    return ir_raw_data;
}




/* Timer Based delay_us Function */
void delay_us(unsigned char time){
   TIM4_EGR |= (1<<0); // Set event genaration registor
   TIM4_CNTR = 0; // Force set counter to zero
   TIM4_CR1 |= (1<<0); // Enable Timer4 Control Register
   while(TIM4_CNTR<time); 
   TIM4_CR1 &= ~(1<<0); // Disable Timer4 Control Register
   TIM4_SR &= ~(1<<0); //clear interrupt flag

}
/*delay_ms Function derived from delay_us Function */
void delay_ms(unsigned int time){
   time *= 10;
   while(time--);
   delay_us(100);
}







#endif

