#include "Gpio.h"

void GPIO_Init(void)
{
  /* Enable GPIOA and GPIOC clocks */
  RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

  /* Configure USER_LED as output (01) */
  GPIOA_MODER |=  (1UL << (USER_LED * 2U));
  GPIOA_MODER &= (uint32_t)(~(1UL << (USER_LED * 2U + 1U)));

  /* Set PC3 as output */
  GPIOC_MODER &= ~(3U << (3 * 2));  // Clear mode bits for PC3
  GPIOC_MODER |= (1U << (3 * 2));   // Set mode to output for PC3

  __enable_irq();

}


void Led_Blinky(void)
{
  /* Toggle the LED pin */
  GPIOA_ODR ^= (1UL << 5U);
}

void Led_On(void)
{
  /* Turn on the LED */
  GPIOA_ODR = (uint32_t)(1UL << 5U);
}

void Led_Off(void)
{
  /* Turn off the LED */
  GPIOA_ODR &= (uint32_t)(~(1UL << 5U));
}

void PC3_Toggle(void)
{
  /* Toggle the LED pin */
  GPIOC_ODR ^= (1UL << 3U);
}

void PC3_On(void)
{
  GPIOC_ODR = (uint32_t)(1U << 3);
}

void PC3_Off(void)
{
  GPIOC_ODR &= (uint32_t)(~(1U << 3));
}


