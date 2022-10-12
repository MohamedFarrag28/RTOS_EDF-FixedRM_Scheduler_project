/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"
#include "queue.h"

/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


TaskHandle_t Button1_TaskHandler  = NULL;
TaskHandle_t Button2_TaskHandler  = NULL;
TaskHandle_t Periodic_TaskHandler = NULL ;
TaskHandle_t Uart_TaskHandler = NULL ;
TaskHandle_t Load1_TaskHandler = NULL ;
TaskHandle_t Load2_TaskHandler = NULL ;


typedef struct AMessage
{
    char ucMessageID;
    char ucData[ 11 ];
}xMessage;


QueueHandle_t xQueue ;
/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );	
/*-----------------------------------------------------------*/

pinState_t button1_State  ;
pinState_t button2_State  ;


void Button_1_Monitor( void * pvParameters )
{
    TickType_t xLastWakeTime;
		const TickType_t xFrequency = 50;
	
		xMessage Button1_Rising = {1, "B1_Rising "} ,Button1_Falling={1, "B1_Falling"} ;
		Button1_Rising.ucData[10]= '\n' ;
		Button1_Falling.ucData[10]= '\n' ;


     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
		
		/* Assign a tag value of 1 to myself. */
		//vTaskSetApplicationTaskTag( NULL, ( void * ) 1 );
    for( ;; )
    {
        /* Task code goes here. */
				if(button1_State != GPIO_read(PORT_0,PIN0) )
				{
					if (button1_State) // ---> falling_egde
					{
						xQueueSendToBack( xQueue,( void * ) &Button1_Falling,0 );
					}
					else 							// ----> rising edge
					{
						xQueueSendToBack( xQueue,( void * ) &Button1_Rising,0 );

					}
					button1_State ^= 1 ; //toggle it
				}

			
				vTaskDelayUntil( &xLastWakeTime, xFrequency );

    }
}
 


void Button_2_Monitor( void * pvParameters )
{
    TickType_t xLastWakeTime;
		const TickType_t xFrequency = 50;

		xMessage Button2_Rising = {2, "B2_Rising "} ,Button2_Falling={2, "B2_Falling"} ;
		Button2_Rising.ucData[10]= '\n' ;
		Button2_Falling.ucData[10]= '\n' ;

     // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
		
		/* Assign a tag value of 1 to myself. */
		//vTaskSetApplicationTaskTag( NULL, ( void * ) 2 );

    for( ;; )
    {
        /* Task code goes here. */
							
				if(button2_State != GPIO_read(PORT_0,PIN1) )
				{
					if (button2_State) // ---> falling_egde
					{
						xQueueSendToBack( xQueue,( void * ) &Button2_Falling,0 );
					}
					else 							// ----> rising edge
					{
						xQueueSendToBack( xQueue,( void * ) &Button2_Rising,0 );

					}
					button2_State ^= 1 ; //toggle it
				}
				vTaskDelayUntil( &xLastWakeTime, xFrequency );

    }
}

void Periodic_Transmitter(void * pvParameters)
{
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = 100;
		xMessage Periodic_TxData = {3, "Periodic_T"};
		Periodic_TxData.ucData[10]= '\n' ;
     // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
		
		/* Assign a tag value of 1 to myself. */
		//vTaskSetApplicationTaskTag( NULL, ( void * ) 3 );
	
    for( ;; )
    {
        /* Task code goes here. */
				xQueueSendToBack( xQueue,( void * ) &Periodic_TxData,0 );
				vTaskDelayUntil( &xLastWakeTime, xFrequency );

    }
}


void Uart_Receiver(void * pvParameters)
{
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = 20;
		xMessage RxDate ;
     // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
		
		/* Assign a tag value of 1 to myself. */
		//vTaskSetApplicationTaskTag( NULL, ( void * ) 4 );
    for( ;; )
    {
        /* Task code goes here. */
				if (xQueueReceive( xQueue , &RxDate ,0) == pdTRUE )
				{
					vSerialPutString(RxDate.ucData,11);
				}
				vTaskDelayUntil( &xLastWakeTime, xFrequency );

    }
}


void Load_1_Simulation(void * pvParameters)
{
		TickType_t xLastWakeTime;
		int i ;
		const TickType_t xFrequency = 10;
	
     // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
		
		/* Assign a tag value of 1 to myself. */
		//vTaskSetApplicationTaskTag( NULL, ( void * ) 5 );
    for( ;; )
    {
        /* Task code goes here. */
			for(i=0;i<33010;i++)
			{
				i = i ;
			}
			
			vTaskDelayUntil( &xLastWakeTime, xFrequency );

    }
}


void Load_2_Simulation(void * pvParameters)
{
		TickType_t xLastWakeTime;
		int i ;
		const TickType_t xFrequency = 100;
	
     // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
		
		/* Assign a tag value of 1 to myself. */
		//vTaskSetApplicationTaskTag( NULL, ( void * ) 6 );
    for( ;; )
    {
        /* Task code goes here. */
			for(i=0;i<79224;i++)
			{
				i = i ;
			}
			
			vTaskDelayUntil( &xLastWakeTime, xFrequency );

    }
}




/*Implement TickHook*/
void vApplicationTickHook( void )
{
	GPIO_write(PORT_0,PIN2,PIN_IS_HIGH);
	GPIO_write(PORT_0,PIN2,PIN_IS_LOW);
}


int Task_1_in_time = 0 ,Task_1_off_time = 0,Task_1_total_time = 0 ;
int Task_2_in_time = 0 ,Task_2_off_time = 0,Task_2_total_time = 0 ;
int Task_3_in_time = 0 ,Task_3_off_time = 0,Task_3_total_time = 0 ;
int Task_4_in_time = 0 ,Task_4_off_time = 0,Task_4_total_time = 0 ;
int Task_5_in_time = 0 ,Task_5_off_time = 0,Task_5_total_time = 0 ;
int Task_6_in_time = 0 ,Task_6_off_time = 0,Task_6_total_time = 0 ;

int system_time = 0 ;
float cpu_load = 0 ;


/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	
  /* Create queue here */
	xQueue = xQueueCreate(6, sizeof(xMessage)) ;
	
	
	button1_State = GPIO_read(PORT_0,PIN0) ;
  button2_State = GPIO_read(PORT_0,PIN1);
	
  /* Create Tasks here */
										

	xTaskPeriodicCreate( 
                    Button_1_Monitor,       /* Function that implements the task. */
                    "button1",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Button1_TaskHandler ,      /* Used to pass out the created task's handle. */
										50 );									
	vTaskSetApplicationTaskTag( Button1_TaskHandler, ( void * ) 1 );
										
	xTaskPeriodicCreate( 
                    Button_2_Monitor,       /* Function that implements the task. */
                    "button2",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Button2_TaskHandler ,      /* Used to pass out the created task's handle. */
										50 );	
	
	vTaskSetApplicationTaskTag( Button2_TaskHandler, ( void * ) 2 );					
										
	xTaskPeriodicCreate( 
                    Periodic_Transmitter,       /* Function that implements the task. */
                    "Periodic_Transmitter",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Periodic_TaskHandler ,      /* Used to pass out the created task's handle. */
										100 );	
	vTaskSetApplicationTaskTag( Periodic_TaskHandler, ( void * ) 3 );


	xTaskPeriodicCreate( 
                    Uart_Receiver,       /* Function that implements the task. */
                    "Uart_Receiver",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Uart_TaskHandler ,      /* Used to pass out the created task's handle. */
										20 );											
	vTaskSetApplicationTaskTag( Uart_TaskHandler, ( void * ) 4 );

	

					
	xTaskPeriodicCreate( 
                    Load_1_Simulation,       /* Function that implements the task. */
                    "Load_1_Simulation",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Load1_TaskHandler ,      /* Used to pass out the created task's handle. */
										10	 );	
	vTaskSetApplicationTaskTag( Load1_TaskHandler, ( void * ) 5 );


	xTaskPeriodicCreate( 
                    Load_2_Simulation,       /* Function that implements the task. */
                    "Load_2_Simulation",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Load2_TaskHandler ,      /* Used to pass out the created task's handle. */
										100	 );	
	vTaskSetApplicationTaskTag( Load2_TaskHandler, ( void * ) 6 );




	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/



/* QQQQQ2 */
