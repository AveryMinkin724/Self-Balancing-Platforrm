/* Board Support Package (BSP) for the EK-TM4C123GXL board */
#include <stdint.h>  /* Standard integers. WG14/N843 C99 Standard */

#include "bsp.h"
#include "miros.h"
#include "qassert.h"
#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */
#include "uart.h"
#include "motor.h"

/* on-board LEDs */
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)
#define TEST_PIN  (1U << 4)

static uint32_t volatile l_tickCtr;

void SysTick_Handler(void) {
    GPIOF_AHB->DATA_Bits[TEST_PIN] = TEST_PIN;

    ++l_tickCtr;

    __disable_irq();
    OS_sched();
    __enable_irq();

    GPIOF_AHB->DATA_Bits[TEST_PIN] = 0U;
}

void Clock_init(void) {
		/* Force Config Clock 16MHz*/
		SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;      // Use RCC2
		SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;      // Bypass PLL while configuring
		SYSCTL_RCC_R &= ~SYSCTL_RCC_USESYSDIV;     // No system clock divider
		SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;        // Clear XTAL field
		SYSCTL_RCC_R |= SYSCTL_RCC_XTAL_16MHZ;     // Set crystal value to 16MHz
		SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;   // Clear oscillator source
		SYSCTL_RCC2_R |= SYSCTL_RCC2_OSCSRC2_MO;   // Use Main Oscillator
}

void BSP_init(void) {
		
		/* Configure Clock since some part of the startup seems to select clock that is not 16MHz (system_TM4C123GH6PM.c?) */
		Clock_init();
    
		/* For LEDs */
	
		SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run Mode for GPIOF */

    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN | TEST_PIN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN | TEST_PIN);
		
		/* For UART */
	
		//stop watchdog timer
		SYSCTL_RCGCWD_R &= ~(1U << 0); // Disable WDT0 clock
		SYSCTL_RCGCWD_R &= ~(1U << 1); // Disable WDT1 clock

		//Initialize UART (section 14.4 datasheet)
		uart_init();
	
		/* For Motor Driver */
		motor_init();
		
}

uint32_t BSP_tickCtr(void) {
    uint32_t tickCtr;

    __disable_irq();
    tickCtr = l_tickCtr;
    __enable_irq();

    return tickCtr;
}

void BSP_delay(uint32_t ticks) {
    uint32_t start = BSP_tickCtr();
    while ((BSP_tickCtr() - start) < ticks) {
    }
}

void BSP_ledRedOn(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
}

void BSP_ledRedOff(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
}

void BSP_ledBlueOn(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
}

void BSP_ledBlueOff(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = 0U;
}

void BSP_ledGreenOn(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
}

void BSP_ledGreenOff(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
}

void OS_onStartup(void) {
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* set the SysTick interrupt priority (highest) */
    NVIC_SetPriority(SysTick_IRQn, 0U);
}

//............................................................................
_Noreturn void Q_onAssert(char const * const module, int const id) {
    (void)module; // unused parameter
    (void)id;     // unused parameter
#ifndef NDEBUG
    // light up all LEDs
    GPIOF_AHB->DATA_Bits[LED_GREEN | LED_RED | LED_BLUE] = 0xFFU;
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#endif
    NVIC_SystemReset();
}
//............................................................................
_Noreturn void assert_failed(char const * const module, int const id);
_Noreturn void assert_failed(char const * const module, int const id) {
    Q_onAssert(module, id);
}
