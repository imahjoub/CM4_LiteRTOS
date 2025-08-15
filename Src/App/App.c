#include "stdint.h"

#include <Mcal/Gpio.h>
#include <Mcal/Gpt.h>
#include <Mcal/Mcu.h>
#include <OS/Os.h>

/*--------------------------------------------------------------
- Global Variables
---------------------------------------------------------------*/
uint32_t Blinky_Stack   [40U];
uint32_t TogglePC3_Stack[40U];
uint32_t IdleThread_Stack[40U];

OSThread Blinky_Thread;
OSThread TogglePC3_Thread;


/*--------------------------------------------------------------
- Function Declarations
---------------------------------------------------------------*/
void Blinky_Main    (void);
void TogglePC3_Main (void);


/*--------------------------------------------------------------
- Functions Definition
---------------------------------------------------------------*/
void Blinky_Main(void)
{
  while(1U)
  {
    Led_On();
    OS_msDelay(10);
    Led_Off();
    OS_msDelay(20);
  }
}


void TogglePC3_Main(void)
{
  while(1U)
  {
    PC3_On();
    OS_msDelay(20);
    PC3_Off();
    OS_msDelay(40);
  }
}



/*--------------------------------------------------------------
- Main
---------------------------------------------------------------*/
int main(void)
{
  /* Configure the System clock and flash */
  SystemInit();
  SetSysClock();

  /* Configure systick timer to generate half second delay */
  SysTick_Init();

  /*Initialize Gpio pins and EXTI */
  GPIO_Init();

  /* Os Initialization */
  OS_Init(IdleThread_Stack, sizeof(IdleThread_Stack));

  /* Initialize Cortex-M ISR stack frame for the Blinky1 thread */
  OSThread_Start(&Blinky_Thread,
                 3U,
                 &Blinky_Main,
                 Blinky_Stack,
                 sizeof(Blinky_Stack));

  /* Initialize Cortex-M ISR stack frame for the PC3 Toggle thread */
  OSThread_Start(&TogglePC3_Thread,
                 2U,
                 &TogglePC3_Main,
                 TogglePC3_Stack,
                 sizeof(TogglePC3_Stack));

  /* Run Os */
  OS_Run();
}
