#include "Gpio.h"
#include "OS/Os.h"

#include "stdatomic.h"


/*----------------------------------------------------------------------------
 - Atomic flag used to ensure exclusive access to GPIOC pins
  during read-modify-write operations.
-----------------------------------------------------------------------------*/
atomic_flag port_c_lock = ATOMIC_FLAG_INIT;

/*----------------------------------------------------------------------------
- @brief Port C Resource Management
-
- @desc Provides atomic functions to acquire and release exclusive access
        to GPIO port C using a spinlock mechanism.
-----------------------------------------------------------------------------*/
static inline void ReleasePortResource(void) { atomic_flag_clear       (&port_c_lock); }
static inline void AcquirePortResource(void) { atomic_flag_test_and_set(&port_c_lock); }


/*----------------------------------------------------------------------------
- @brief GPIO_Init
-
- @desc Initializes GPIO ports A and C: enables clocks and configures
        selected pins (USER_LED, PC2, PC3, PC10) as outputs.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
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


/*----------------------------------------------------------------------------
- @brief Led_Blinky
-
- @desc Toggles the USER LED on GPIOA with a 20 ms delay for blinking.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void Led_Blinky(void)
{
  /* Toggle the LED pin */
  GPIOA_ODR ^= (1UL << 5U);
  OS_msDelay(20U);
}


/*----------------------------------------------------------------------------
- @brief Led_On
-
- @desc Sets the USER LED pin on GPIOA to turn the LED on.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void Led_On(void)
{
  /* Turn on the LED */
  GPIOA_ODR |= (uint32_t)(1UL << 5U);
}


/*----------------------------------------------------------------------------
- @brief Led_Off
-
- @desc Clears the USER LED pin on GPIOA to turn the LED off.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void Led_Off(void)
{
  /* Turn off the LED */
  GPIOA_ODR &= (uint32_t)(~(1UL << 5U));
}


/*----------------------------------------------------------------------------
- @brief PC3_On
-
- @desc Sets GPIOC pin 3 high with atomic access to ensure exclusive
        port access.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void PC3_On(void)
{
  AcquirePortResource();
  GPIOC_ODR |= (uint32_t)(1UL << 3U);
  ReleasePortResource();
}


/*----------------------------------------------------------------------------
- @brief PC3_Off
-
- @desc Clears GPIOC pin 3 with atomic access to ensure exclusive
        port access.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void PC3_Off(void)
{
  AcquirePortResource();
  GPIOC_ODR &= (uint32_t)(~(1UL << 3));
  ReleasePortResource();
}


/*----------------------------------------------------------------------------
- @brief PC2_Off
-
- @desc Clears GPIOC pin 2 with atomic access to ensure exclusive
        port access.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void PC2_Off(void)
{
  AcquirePortResource();
  GPIOC_ODR &= (uint32_t)(~(1UL << 2U));
  ReleasePortResource();
}


/*----------------------------------------------------------------------------
- @brief PC2_On
-
- @desc Sets GPIOC pin 2 high with atomic access to ensure exclusive
        port access.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void PC2_On(void)
{
  AcquirePortResource();
  GPIOC_ODR |= (uint32_t)(1UL << 2U);
  ReleasePortResource();
}


/*----------------------------------------------------------------------------
- @brief PC10_On
-
- @desc Sets GPIOC pin 10 high with atomic access to ensure exclusive
        port access.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void PC10_On(void)
{
  AcquirePortResource();
  GPIOC_ODR |= (uint32_t)(1UL << 10U);
  ReleasePortResource();
}


/*----------------------------------------------------------------------------
- @brief PC10_Off
-
- @desc Clears GPIOC pin 10 with atomic access to ensure exclusive
        port access.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void PC10_Off(void)
{
  AcquirePortResource();
  GPIOC_ODR &= (uint32_t)(~(1UL << 10U));
  ReleasePortResource();
}

