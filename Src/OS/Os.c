#include <stdint.h>
#include "Mcal/Mcu.h"
#include "Os.h"

__attribute__ ((naked)) void PendSV_Handler(void);
void IdleThread_Main();

OSThread * volatile OS_Curr; /* pointer to the current thread     */
OSThread * volatile OS_Next; /* pointer to the next thread to run */

OSThread *OS_Thread[32U + 1U]; /* array of threads started so far                 */
uint8_t   OS_ThreadNum;        /* number of threads started so far                */
uint8_t   OS_CurrIdx;          /* current thread index for round robin scheduling */
uint32_t  OS_ReadySet;         /* bitmask of threads that are ready to run        */

OSThread Idle_Thread;
void IdleThread_Main()
{
  while(1U)
  {
    OS_OnIdle();
  }
}


void OS_Init(void *StkStorage, uint32_t StkSize)
{
  /* set the PendSV interrupt priority to the lowest level 0xFF */
  NVIC_SYS_PRI3_R |= (0xFFUL << 16U);

  /* start idleThread thread */
  OSThread_Start(&Idle_Thread,
                 &IdleThread_Main,
                 StkStorage,
                 StkSize);
}

void OS_Sched(void)
{
  /* OS_next = ... */
  /* idle condition? */
  if (OS_ReadySet == 0U)
  {
    OS_CurrIdx = 0U; /* index of the idle thread */
  }
  else
  {
    do
    {
      ++OS_CurrIdx;

      if (OS_CurrIdx == OS_ThreadNum)
      {
        OS_CurrIdx = 1U;
      }
    }
    while ((OS_ReadySet & (1U << (OS_CurrIdx - 1U))) == 0U);
  }
  /* temporarty for the next thread */
  OSThread * const next = OS_Thread[OS_CurrIdx];

  /* trigger PendSV, if needed */
  if (next != OS_Curr)
  {
    OS_Next = next;
    ICSR |= (1UL << 28U);
  }
}

void OS_OnIdle(void)
{
  /* stop the CPU and Wait for Interrupt */
  WaitForIrq();
}


void OS_Delay(uint32_t Ticks)
{
  Disable_Irq();

  /* never call OS_delay from the idleThread */
  /* TBD add the following check */
  //Q_REQUIRE(OS_curr != OS_thread[0]);

  OS_Curr->TimeOut = Ticks;

  OS_ReadySet &= (uint32_t)(~(1UL << (OS_CurrIdx - 1U)));

  OS_Sched();

  Enable_Irq();
}

void OS_Tick(void)
{
  for(uint8_t i = 1U; i < OS_ThreadNum; ++i)
  {
    if(OS_Thread[i]->TimeOut != 0U)
    {
      --OS_Thread[i]->TimeOut;

      if(OS_Thread[i]->TimeOut == 0U)
      {
        OS_ReadySet |= (1U << (i - 1U));
      }
    }
  }
}



void OS_Run(void)
{
  /* callback to configure and start interrupts */
  /* TBD get rid of this function OS_OnStartup */
  OS_OnStartup();

  Disable_Irq();
  OS_Sched();
  Enable_Irq();
}

void OSThread_Start(OSThread *TCB, OSThreadHandler ThreadHandler, void *StkStorage, uint32_t StkSize)
{
  /* round down the stack top to the 8-byte boundary
  * NOTE: ARM Cortex-M stack grows down from hi -> low memory
  */
  uint32_t *StckPointer = (uint32_t *)((((uint32_t)StkStorage + StkSize) / 8U) * 8U);

  uint32_t *Stk_Limit;

  *(--StckPointer) = (1UL << 24U);            /* xPSR */
  *(--StckPointer) = (uint32_t)ThreadHandler; /* PC   */
  *(--StckPointer) = 0x0000000EU;             /* LR   */
  *(--StckPointer) = 0x0000000CU;             /* R12  */
  *(--StckPointer) = 0x00000003U;             /* R3   */
  *(--StckPointer) = 0x00000002U;             /* R2   */
  *(--StckPointer) = 0x00000001U;             /* R1   */
  *(--StckPointer) = 0x00000000U;             /* R0   */

  /* additionally, fake registers R4-R11     */
  *(--StckPointer) = 0x0000000BU;             /* R11  */
  *(--StckPointer) = 0x0000000AU;             /* R10  */
  *(--StckPointer) = 0x00000009U;             /* R9   */
  *(--StckPointer) = 0x00000008U;             /* R8   */
  *(--StckPointer) = 0x00000007U;             /* R7   */
  *(--StckPointer) = 0x00000006U;             /* R6   */
  *(--StckPointer) = 0x00000005U;             /* R5   */
  *(--StckPointer) = 0x00000004U;             /* R4   */

  /* save the top of the stack in the thread's attibute */
  TCB->MyStckPointer = StckPointer;

  /* round up the bottom of the stack to the 8-byte boundary */
  Stk_Limit = (uint32_t *)(((((uint32_t)StkStorage - 1U) / 8U) + 1U) * 8U);

  /* pre-fill the unused part of the stack with 0xDEADBEEF */
  for (StckPointer = StckPointer - 1U; StckPointer >= Stk_Limit; --StckPointer)
  {
    *StckPointer = 0xDEADBEEFUL;
  }

  /* register the thread with the OS */
  OS_Thread[OS_ThreadNum] = TCB;

  /* make the thread ready to run */
  if(OS_ThreadNum > 0U)
  {
    OS_ReadySet |= (1U << (OS_ThreadNum - 1U));
  }

  ++OS_ThreadNum;
}

__attribute__ ((naked)) void PendSV_Handler(void)
{
  __asm volatile
  (
           /* Disable_Irq(); */
    "  CPSID         I                  \n"

    /* if (OS_curr != (OSThread *)0) { */
    "  LDR           r1,=OS_Curr      \n"
    "  LDR           r1,[r1,#0x00]    \n"
    "  CMP           r1,#0            \n"
    "  BEQ           PendSV_restore   \n"

    /* push registers r4-r11 on the stack  */
    "  PUSH          {r4-r11}           \n"

         /* OS_curr->sp = sp; */
    "  LDR           r1,=OS_Curr      \n"
    "  LDR           r1,[r1,#0x00]    \n"
    "  MOV           r0,sp            \n"
    "  STR           r0,[r1,#0x00]    \n"

                /* } */
    "PendSV_restore:                  \n"

        /* sp = OS_next->sp; */
    "  LDR           r1,=OS_Next      \n"
    "  LDR           r1,[r1,#0x00]    \n"
    "  LDR           r0,[r1,#0x00]    \n"
    "  MOV           sp,r0            \n"

        /* OS_curr = OS_next; */
    "  LDR           r1,=OS_Next     \n"
    "  LDR           r1,[r1,#0x00]   \n"
    "  LDR           r2,=OS_Curr     \n"
    "  STR           r1,[r2,#0x00]   \n"

       /* pop registers r4-r11 */
    "  POP           {r4-r11}        \n"

         /* Enable_Irq(); */
    "  CPSIE         I               \n"

       /* return to the next thread */
    "  BX            lr              \n"
  );
}
