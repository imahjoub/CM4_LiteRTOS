#include <stdint.h>
#include "Mcal/Gpio.h"
#include "Mcal/Mcu.h"
#include "Os.h"

__attribute__ ((naked)) void PendSV_Handler(void);
static void IdleThread_Main(void);

OSThread * volatile OS_Curr;    /* pointer to the current thread */
OSThread * volatile OS_Next;    /* pointer to the next thread to run */

OSThread  IdleThread;
OSThread *OS_Thread[32U + 1U];  /* array of threads started so far */

uint8_t   OS_CurrIdx;           /* current thread index for round robin scheduling */
uint32_t  OS_ReadySet;          /* bitmask of threads that are ready to run */
uint32_t  OS_DelayedSet;        /* bitmask of threads that are delayed */

#define LOG2(x) (32U - (uint32_t)__builtin_clz(x))


static void IdleThread_Main(void)
{
  while(1U)
  {
    OS_OnIdle();
  }
}


void OS_Init(void *StackStorage, uint32_t SatckSize)
{
  /* set the PendSV interrupt priority to the lowest level 0xFF */
  NVIC_SYS_PRI3_R |= (0xFFUL << 16U);

  /* start IdleThread thread */
  OSThread_Start(&IdleThread, 0U, &IdleThread_Main, StackStorage, SatckSize);
}

void OS_Sched(void)
{
  /* choose the next thread to execute... */
  OSThread* NextThread;

  /* idle condition? */
  if (OS_ReadySet == 0U)
  {
    NextThread = OS_Thread[0U]; /* the idle thread */
  }
  else
  {
    NextThread = OS_Thread[LOG2(OS_ReadySet)];
  }

  /* temporarty for the next thread */
  //OSThread * const next = OS_Thread[OS_CurrIdx];

  /* trigger PendSV, if needed */
  if(NextThread != OS_Curr)
  {
    OS_Next  = NextThread;
    ICSR    |= (1UL << 28U);
  }
}

void OS_OnIdle(void)
{
  PC10_On();
  PC10_Off();

  /* stop the CPU and Wait for Interrupt */
  //Wait_For_Interrupt();
}


void OS_msDelay(uint32_t Ticks)
{
  Disable_Irq();

  /* TBD never call OS_delay from the idleThread */
  //Q_REQUIRE(OS_curr != OS_thread[0]);

  OS_Curr->TimeOut = Ticks;

  OS_ReadySet   &= ~(1U << (OS_Curr->Prio - 1U));
  OS_DelayedSet |=  (1U << (OS_Curr->Prio - 1U));

  OS_Sched();

  Enable_Irq();
}

void OS_Tick(void)
{
  uint32_t workingSet = OS_DelayedSet;

  while (workingSet != 0U)
  {
    OSThread* LocalThread = OS_Thread[LOG2(workingSet)];
    uint32_t bit;

    bit = (1U << (LocalThread->Prio - 1U));

    --LocalThread->TimeOut;

    if (LocalThread->TimeOut == 0U)
    {
      OS_ReadySet   |=  bit;  /* insert to set */
      OS_DelayedSet &= ~bit;  /* remove from set */
    }

    workingSet &= ~bit;      /* remove from working set */
  }
}


void OS_Run(void)
{
  /* callback to configure and start interrupts */
  OS_OnStartup();

  Disable_Irq();
  OS_Sched();
  Enable_Irq();
}

void OSThread_Start(OSThread *TCB, uint8_t Prio, OSThreadHandler ThreadHandler, void *StkStorage, uint32_t StkSize)
{
  /* round down the stack top to the 8-byte boundary
  * NOTE: ARM Cortex-M stack grows down from hi -> low memory
  */
  uint32_t *StckPointer = (uint32_t *)((((uint32_t)StkStorage + StkSize) / 8) * 8);

  uint32_t *StckLimit;

  *(--StckPointer) = (1U << 24);              /* xPSR */
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
  StckLimit = (uint32_t *)(((((uint32_t)StkStorage - 1U) / 8U) + 1U) * 8U);

  /* pre-fill the unused part of the stack with 0xDEADBEEF */
  for (StckPointer = StckPointer - 1U; StckPointer >= StckLimit; --StckPointer)
  {
    *StckPointer = 0xDEADBEEFU;
  }

  /* register the thread with the OS */
  OS_Thread[Prio] = TCB;
  TCB->Prio       = Prio;

  /* make the thread ready to run */
  if(Prio > 0U)
  {
    OS_ReadySet |= (1U << (Prio - 1U));
  }

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
