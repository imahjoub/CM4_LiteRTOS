#include <stdbool.h>

#include <Mcal/Gpt.h>
#include <Mcal/Mcu.h>
#include <Mcal/Gpio.h>

/* Defines */

void SystemInit(void)
{
  /* set coprocessor access control register CP10 and CP11 Full Access */
  SCB_CPACR |= (uint32_t)((uint32_t)(3UL << 20U) | (uint32_t)(3UL << 22U));

  /* Set HSION (internal high-speed clock) enable bit */
  RCC_CR |= (uint32_t)(1UL << 0U);

  /* Reset HSEON, CSSON, HSEBYP and PLLON bits */
  RCC_CR &= (uint32_t)((~(1UL << 16U)) | (~(1UL << 18U))) | (~(1UL << 19U)) | (~(1UL << 24U));

  /* Reset CFGR register */
  RCC_CFGR = (uint32_t)0x00000000UL;

  /* Reset PLLCFGR register */
  RCC_PLLCFGR = (uint32_t)0x24003010UL;

  /* Disable all interrupts */
  RCC_CIR = (uint32_t)0x00000000UL;

  /* Configure Flash prefetch, Instruction cache, Data cache and wait state (5 wait states) */
  FLASH_ACR = (uint32_t)((1UL << 9U) | (1UL << 10U) | (5UL << 0U));
}

void SetSysClock(void)
{
  /* Enable HSE */
  RCC_CR |= ((uint32_t)(1UL << 16U));

  /* Wait till HSE is ready */
  while(!(RCC_CR & ((uint32_t)1UL << 17U)))
  {
    __asm volatile("nop");
  }

  /* PWREN: Power interface clock enable */
  RCC_APB1ENR |= (uint32_t)(1UL << 28U);

  /* Set HCLK  = sysclk / 1 */
  /* Set PCLK2 = hclk   / 2 */
  /* Set PCLK1 = hclk   / 4 */     // for WWDG  set --> (7UL << 10U)  for hclk / 16
  RCC_CFGR |= (uint32_t)((5UL << 10U) | (1UL << 15U));

  /* Configure the main PLL */
  /* PLL_M = 8              */
  /* PLL_N = 360            */
  /* PLL_P = 0 -> 2         */
  /* PLL_Q = 7              */
  /* SYSCLK = 180 MHz       */
  RCC_PLLCFGR = (uint32_t)(8UL << 0U) | (360UL << 6U) | (0UL << 16U) | (1UL << 22U) | (7UL << 24U);

  /* Enable the main PLL */
  RCC_CR |= (uint32_t)(1UL << 24U);

  /* Wait till the main PLL is ready */
  while(!(RCC_CR & (uint32_t)(1UL << 25U)))
  {
    __asm volatile("nop");
  }

  /* Select the main PLL as system clock source */
  RCC_CFGR &= (uint32_t)(~(3UL << 0U));
  RCC_CFGR |= (uint32_t)(2UL << 0U);

  /* Wait till the main PLL is used as system clock source */
  while ((RCC_CFGR & (uint32_t)(0x0CU << 0U)) != (8UL << 0U))
  {
  }
}


void SysTick_Init(void)
{
  /* Reset the SysTick control register. */
  STK_CTRL = (uint32_t)0x00000000UL;

  /* Set the SysTick reload register to be equivalent to 1ms. */
  STK_LOAD = (uint32_t)(180000UL);

  /* Initialize the SysTick counter value (clear it to zero). */
  STK_VAL = (uint32_t)0x00000000UL;

  /* Set the SysTicl clock source to be the main processor clock. */
  STK_CTRL = (uint32_t)0x00000004UL;

  /* Enable the SysTick interrupt. */
  STK_CTRL |= (uint32_t)(2UL);

  /* Enable the SysTick counter. */
  STK_CTRL |= (uint32_t)(1UL << 0U);
}


void NVIC_SetPriority(int32_t IRQn, uint32_t Priority)
{
  /* Shift for top 4 bits */
  uint8_t Prio_Val = (Priority & 0xFU) << 4U;

  if(IRQn < (int32_t)0U)
  {
    /* System Exceptions: map IRQn to SHP registers */
    uint32_t RegAddr           = 0xE000ED18UL + (((IRQn & (int32_t)0xFU) - 4U) * 1U); // 1 byte per priority field
    volatile uint8_t *Prio_Ptr = (volatile uint8_t *)RegAddr;
    *Prio_Ptr                  = Prio_Val;
  }
  else
  {
    /* Peripheral IRQs */
    volatile uint8_t *Prio_Ptr = (volatile uint8_t *)(NVIC_IPR_BASE + (uint32_t)IRQn);
    *Prio_Ptr                  = Prio_Val;
  }
}



inline void __enable_irq(void)
{
  __asm volatile ("cpsie i" ::: "memory");
}

inline void __disable_irq(void)
{
  __asm volatile ("cpsid i" ::: "memory");
}




