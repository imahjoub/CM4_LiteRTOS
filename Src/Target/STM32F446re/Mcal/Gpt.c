#include <Mcal/Gpio.h>
#include <Mcal/Gpt.h>
#include <Mcal/Mcu.h>
#include <OS/Os.h>

/*----------------------------------------------------------------------------
- File-Local Variables
-----------------------------------------------------------------------------*/
static Gpt_ValueType millisec_counter;


/*----------------------------------------------------------------------------
- Function Declarations
-----------------------------------------------------------------------------*/
void SysTick_Handler(void);


/*----------------------------------------------------------------------------
- @brief Gpt_GetTimeElapsed
-
- @desc Returns the elapsed time in milliseconds from the system counter.
-
- @param DummyChannelIndex   Unused channel index parameter
- @return Gpt_ValueType      Elapsed time in milliseconds
-----------------------------------------------------------------------------*/
Gpt_ValueType Gpt_GetTimeElapsed(const Gpt_ChannelType DummyChannelIndex)
{
  (void) DummyChannelIndex;

  return millisec_counter;
}


/*----------------------------------------------------------------------------
- @brief SysTick_Handler
-
- @desc SysTick interrupt service routine: increments millisecond counter,
  updates delayed threads, runs scheduler, and toggles PC2 for timing.
-
- @param void
- @return void
-----------------------------------------------------------------------------*/
void SysTick_Handler(void)
{
  PC2_On();

  ++millisec_counter;

  OS_Tick();

  Disable_Irq();
  OS_Sched();
  Enable_Irq();

  PC2_Off();
}

