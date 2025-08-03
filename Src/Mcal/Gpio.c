#include "Gpio.h"

void GPIO_Init(void)
{
  /* Enable GPIOA and GPIOC clocks */
  RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

  /* Configure USER_LED as output (01) */
  GPIOA_MODER |= (uint32_t)(1UL <<   (USER_LED * 2U));
  GPIOA_MODER &= (uint32_t)(~(1UL << (USER_LED * 2U + 1U)));

  /* Set PC3 as output */
  GPIOC_MODER &= (uint32_t)(~(3U << (3U * 2)));
  GPIOC_MODER |= (uint32_t)  (1U << (3U * 2));

  /* Set PC2 as output */
  GPIOC_MODER &= (uint32_t)(~(3U << (2U * 2U)));
  GPIOC_MODER |= (uint32_t)  (1U << (2U * 2U));
}


void Led_Blinky(void)
{
  /* Toggle the LED pin */
  GPIOA_ODR ^= (1UL << 5U);
}

void Led_On(void)
{
  /* Turn on the LED */
  GPIOA_ODR |= (uint32_t)(1UL << 5U);
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
  GPIOC_ODR |= (uint32_t)(1UL << 3U);
}

void PC3_Off(void)
{
  GPIOC_ODR &= (uint32_t)(~(1UL << 3));
}

void PC2_Off(void)
{
  GPIOC_ODR &= (uint32_t)(~(1UL << 2U));
}

void PC2_On(void)
{
  GPIOC_ODR |= (uint32_t)(1UL << 2U);
}
