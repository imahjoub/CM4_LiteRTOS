#include "stdint.h"

#include <Mcal/Gpio.h>
#include <Mcal/Gpt.h>
#include <Mcal/Mcu.h>
#include <OS/Os.h>

/***********************************************************************/
/*                           Defines                                   */
/***********************************************************************/
#define BSP_TICKS_PER_SEC 100U

/***********************************************************************/
/*                         Global Variables                            */
/***********************************************************************/

uint32_t Blinky_Stack    [40U];
uint32_t TogglePC3_Stack [40U];
uint32_t IdleThread_Stack[40U];

OSThread Blinky_Thread;
OSThread TogglePC3_Thread;

/***********************************************************************/
/*                            Prototypes                               */
/***********************************************************************/
void Blinky_Main    (void);
void TogglePC3_Main (void);


/***********************************************************************/
/*                        Functions Definition                         */
/***********************************************************************/

void Blinky_Main(void)
{
  while(1U)
  {
    Led_On();
    OS_Delay(300U);
    Led_Off();
    OS_Delay(300U);
  }
}



void TogglePC3_Main(void)
{
  while(1U)
  {
    PC3_On();
    OS_Delay(200U);
    PC3_Off();
    OS_Delay(200U);
  }
}


/***********************************************************************/
/*                              Main                                   */
/***********************************************************************/

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

  /* Fabricate Cortex-M ISR stack frame for blinky1 */
  OSThread_Start(&Blinky_Thread,
                 &Blinky_Main,
                 Blinky_Stack,
                 sizeof(Blinky_Stack));


  /* Fabricate Cortex-M ISR stack frame for Toggle-PC3 */
  OSThread_Start(&TogglePC3_Thread,
                 &TogglePC3_Main,
                 TogglePC3_Stack,
                 sizeof(TogglePC3_Stack));

  /* Run Os */
  OS_Run();
}
