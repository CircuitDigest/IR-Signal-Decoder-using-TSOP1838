/* MAIN.C file
 * 
 * Copyright (c) 2002-2005 STMicroelectronics
 */
#include "stm8s.h"
#include "stm8s_irlib.h"

#define test_LED GPIOB,GPIO_PIN_5 // Onboard LED Pin
int data = 0; // Variable to store IR data

main()
{
	Serial_begin(9600); // 
	Serial_print_string("Enter command \n");
  
	ir_init(); // Init IR
	
	GPIO_DeInit(GPIOB); // Prepare the GPIO by clearing all the register value
	GPIO_Init (GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW);  // set the GPIO as output
	
	
	while (1)
	{
		if(data == 18615)//if received data is true 
		{
			data =0;
			GPIO_WriteHigh(test_LED);// turn off the LED
		}
		
		if(data == 30855)//if received data is true 
		{
			data =0;
			GPIO_WriteLow(test_LED); // turn off the LED
		}
	}
}

@far @interrupt void EXTI3_IRQHandler(void){
	
	data = ir_result(1);
	
}
