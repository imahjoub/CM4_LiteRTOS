#include <stdbool.h>

#include <Mcal/Gpt.h>
#include <Mcal/Mcu.h>
#include <Mcal/Gpio.h>

/*----------------------------------------------------------------------------
- @brief SystemInit
-
- @desc Initializes system settings: enables FPU, configures
        internal clock, resets RCC registers, disables interrupts, and sets
        Flash latency and caches.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
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

/*----------------------------------------------------------------------------
- @brief SetSysClock
-
- @desc Configures and enables the system clock using HSE and PLL.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
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

/*----------------------------------------------------------------------------
- @brief SysTick_Init
-
- @desc Initializes the SysTick timer to generate interrupts every 1 ms
-       using the main processor clock.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void SysTick_Init(void)
{
  /* Reset the SysTick control register. */
  STK_CTRL = (uint32_t)0x00000000UL;

  /* Set the SysTick reload register to be equivalent to 1ms. */
  STK_LOAD = (uint32_t)(180000UL);   /* 1000us(ms) */
  //STK_LOAD = (uint32_t)(180UL);    /*   1us      here I saw a strange behvior, sytick interrupt too fast it does not let osthread start to happen*/
  //STK_LOAD = (uint32_t)(18000UL);  /*   100us      */

  /* Initialize the SysTick counter value (clear it to zero). */
  STK_VAL = (uint32_t)0x00000000UL;

  /* Set the SysTicl clock source to be the main processor clock. */
  STK_CTRL = (uint32_t)0x00000004UL;

  /* Enable the SysTick interrupt. */
  STK_CTRL |= (uint32_t)(2UL);

  /* Enable the SysTick counter. */
  STK_CTRL |= (uint32_t)(1UL << 0U);
}


/*----------------------------------------------------------------------------
- @brief NVIC_SetPriority
-
- @desc Sets the priority of a given IRQ or system exception.
-       Supports both system exceptions (negative IRQn) and peripheral IRQs.
-
- @param IRQn     : IRQ number (negative for system exceptions)
- @param Priority : Priority value (0..15, lower = higher priority)
- @return void
-----------------------------------------------------------------------------*/
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

/*----------------------------------------------------------------------------
- @brief Wait_For_Interrupt
-
- @desc Executes the WFI (Wait For Interrupt) instruction to put the CPU
-       into low-power sleep mode until an interrupt occurs.
-----------------------------------------------------------------------------*/
inline void Wait_For_Interrupt(void)
{
  __asm volatile ("wfi":::"memory");
}


/*----------------------------------------------------------------------------
- @brief Enable_Irq
-
- @desc Enables all maskable interrupts by setting the I-bit in the CPSR.
-----------------------------------------------------------------------------*/
inline void Enable_Irq(void)
{
  __asm volatile ("cpsie i" ::: "memory");
}


/*----------------------------------------------------------------------------
- @brief Disable_Irq
-
- @desc Disables all maskable interrupts by clearing the I-bit in the CPSR.
-----------------------------------------------------------------------------*/
inline void Disable_Irq(void)
{
  __asm volatile ("cpsid i" ::: "memory");
}


