#include <stdint.h> // C99 standard integers
#include "miros.h" 
#include "bsp.h"
#include "uart.h"
#include "logger.h"
#include "motor.h"

uint32_t stack_LEDHeartbeat[40]; //initialize array for stack, this will automatically be assigned to area of RAM
OSThread LEDHeartbeat;
void Task_LEDHeartbeat(void) {		
		while (1) {
				/* Keeps a green LED blinking to indicate system alive */
        BSP_ledGreenOn();
				Logger_log("Green On\r\n");
        BSP_delay(BSP_TICKS_PER_SEC / 4U);
        BSP_ledGreenOff();
				Logger_log("Green Off\r\n");
        BSP_delay(BSP_TICKS_PER_SEC * 4U);
    }
}

uint32_t stack_ControlLoop[40];
OSThread ControlLoop;
void Task_ControlLoop(void) {
		while (1) {
				/* Placeholder for balance control logic (PID, etc.) */
		}
}

uint32_t stack_SensorUpdate[40];
OSThread SensorUpdate;
void Task_SensorUpdate() {
		while (1) {
				/* Placeholder for IMU sampling & filtering */
				static uint32_t counter = 0;
				if (++counter >= 10000) {  // Assuming this task runs at 100 Hz
						Logger_log("Accel: x=..., y=..., z=...\r\n");
						counter = 0;
				}
		}
}

/* background code: sequential with blocking version */
int main(void) {
		BSP_init();
		OS_init();
    
		/* start UART Logging thread */
    logger_start(); 
	
		/* fabricate Cortex-M ISR stack frame for LEDHeartbeat */
		OSThread_start(&LEDHeartbeat,
									 &Task_LEDHeartbeat,
									 stack_LEDHeartbeat, sizeof(stack_LEDHeartbeat));
		
		/* fabricate Cortex-M ISR stack frame for ControlLoop */
		OSThread_start(&ControlLoop,
									 &Task_ControlLoop,
									 stack_ControlLoop, sizeof(stack_ControlLoop));
		
		/* start SensorUpdate thread */
    OSThread_start(&SensorUpdate,
                   &Task_SensorUpdate,
                   stack_SensorUpdate, sizeof(stack_SensorUpdate));
	
		/* start PWM thread */
		motor_start();
		
		/* transfer control to the RTOS to run the threads */
    OS_run();
		
	
	//return 0;
}