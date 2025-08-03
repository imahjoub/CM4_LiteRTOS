#include <Mcal/Gpio.h>
#include <Mcal/Gpt.h>
#include <Mcal/Mcu.h>
#include <OS/Os.h>

static Gpt_ValueType millisec_counter;

Gpt_ValueType Gpt_GetTimeElapsed(const Gpt_ChannelType DummyChannelIndex)
{
  (void) DummyChannelIndex;

  return millisec_counter;
}

void SysTick_Handler(void);

void SysTick_Handler(void)
{
  PC2_On();

  ++millisec_counter;

  __disable_irq();
  OS_Sched();
  __enable_irq();

  PC2_Off();
}


void OS_OnStartup(void)
{
  /* set the SysTick interrupt priority (highest) */
  NVIC_SetPriority(SysTick_IRQn, 0U);
}

