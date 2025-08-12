#include "Gpio.h"
#include "OS/Os.h"

#include <stdatomic.h>

atomic_flag port_c_lock = ATOMIC_FLAG_INIT;

//static inline void acquire_port_resource() { Disable_Irq(); }
//static inline void release_port_resource() { Enable_Irq(); }

static inline void acquire_port_resource() { atomic_flag_test_and_set(&port_c_lock); }
static inline void release_port_resource() { atomic_flag_clear(&port_c_lock); }

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

  /* Set PC10 as output */
  GPIOC_MODER &= (uint32_t)(~(3U << (10U * 2U)));
  GPIOC_MODER |= (uint32_t)  (1U << (10U * 2U));
}


void Led_Blinky(void)
{
  /* Toggle the LED pin */
  GPIOA_ODR ^= (1UL << 5U);
  OS_Delay(200U);
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

void PC3_On(void)
{
  acquire_port_resource();
  GPIOC_ODR |= (uint32_t)(1UL << 3U);
  release_port_resource();
}

void PC3_Off(void)
{
  acquire_port_resource();
  GPIOC_ODR &= (uint32_t)(~(1UL << 3));
  release_port_resource();
}

void PC2_Off(void)
{
  acquire_port_resource();
  GPIOC_ODR &= (uint32_t)(~(1UL << 2U));
  release_port_resource();
}

void PC2_On(void)
{
  acquire_port_resource();
  GPIOC_ODR |= (uint32_t)(1UL << 2U);
  release_port_resource();
}

void PC10_On(void)
{
  acquire_port_resource();
  GPIOC_ODR |= (uint32_t)(1UL << 10U);
  release_port_resource();
}

void PC10_Off(void)
{
  acquire_port_resource();
  GPIOC_ODR &= (uint32_t)(~(1UL << 10U));
  release_port_resource();
}

