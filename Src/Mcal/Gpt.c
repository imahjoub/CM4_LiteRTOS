#include <Mcal/Gpt.h>
#include <Mcal/Mcu.h>
#include <OS/Os.h>

static Gpt_ValueType my_millisec_counter;
static uint32_t  volatile l_tickCtr;

uint32_t BSP_tickCtr(void);

Gpt_ValueType Gpt_GetTimeElapsed(const Gpt_ChannelType DummyChannelIndex)
{
  (void) DummyChannelIndex;

  return my_millisec_counter;
}

void SysTick_Handler(void);

void SysTick_Handler(void)
{
  ++my_millisec_counter;
  ++l_tickCtr;

  __disable_irq();
  OS_Sched();
  __enable_irq();
}


void OS_OnStartup(void)
{
  /* set the SysTick interrupt priority (highest) */
  NVIC_SetPriority(SysTick_IRQn, 0U);
}

