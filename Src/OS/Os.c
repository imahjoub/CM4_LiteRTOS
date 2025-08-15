#include <stdint.h>
#include "Mcal/Gpio.h"
#include "Mcal/Mcu.h"
#include "Os.h"


/*----------------------------------------------------------------------------
- OS Global Variables
-
- @desc Variables used by the OS kernel to track threads, scheduling,
  and thread states.
-----------------------------------------------------------------------------*/
OSThread * volatile OS_Curr;    /* pointer to the current thread */
OSThread * volatile OS_Next;    /* pointer to the next thread to run */

OSThread  IdleThread;
OSThread *OS_Thread[32U + 1U];  /* array of threads started so far */

uint8_t   OS_CurrIdx;           /* current thread index for round robin scheduling */
uint32_t  OS_ReadySet;          /* bitmask of threads that are ready to run */
uint32_t  OS_DelayedSet;        /* bitmask of threads that are delayed */

/*----------------------------------------------------------------------------
- OS Function Declarations
-----------------------------------------------------------------------------*/
__attribute__ ((naked)) void PendSV_Handler(void);
static void IdleThread_Main(void);


/*----------------------------------------------------------------------------
- @brief LOG2
-
- @desc Returns the position of the highest set bit (1-based) in a 32-bit
  integer, used for priority selection in the OS.
-
- @param x   32-bit input value
- @return uint32_t  Highest set bit position
-----------------------------------------------------------------------------*/
#define LOG2(x) (32U - (uint32_t)__builtin_clz(x))


/*----------------------------------------------------------------------------
- @brief IdleThread_Main

- @desc Idle thread loop that repeatedly calls OS_OnIdle.

- @param void

- @return void
-----------------------------------------------------------------------------*/
static void IdleThread_Main(void)
{
  while(1U)
  {
    OS_OnIdle();
  }
}


/*----------------------------------------------------------------------------
- @brief OS_Init

- @desc Initializes the OS by setting PendSV to lowest priority and
        starting the idle thread.

- @param StackStorage   Idle thread stack base address
         SatckSize      Idle thread stack size

- @return void
-----------------------------------------------------------------------------*/
void OS_Init(void *StackStorage, uint32_t SatckSize)
{
  /* set the PendSV interrupt priority to the lowest level 0xFF */
  NVIC_SYS_PRI3_R |= (0xFFUL << 16U);

  /* Start IdleThread thread */
  OSThread_Start(&IdleThread, 0U, &IdleThread_Main, StackStorage, SatckSize);
}

/*----------------------------------------------------------------------------
- @brief OS_Sched

- @desc Selects the highest-priority ready thread and triggers PendSV
        if a context switch is required.

- @param void

- @return void
-----------------------------------------------------------------------------*/
void OS_Sched(void)
{
  /* Select the next thread to execute */
  OSThread* NextThread;

  /* Check for idle condition */
  if (OS_ReadySet == 0U)
  {
    NextThread = OS_Thread[0U]; /* the idle thread */
  }
  else
  {
    /* Pick the highest-priority ready thread */
    NextThread = OS_Thread[LOG2(OS_ReadySet)];
  }

  /* trigger PendSV, if needed */
  if(NextThread != OS_Curr)
  {
    OS_Next  = NextThread;
    ICSR    |= (1UL << 28U); /* set PendSV pending bit */
  }
}


/*----------------------------------------------------------------------------
- @brief OS_OnIdle

- @desc  Executes idle-time actions and places CPU in low-power
         wait-for-interrupt state.

- @param void

- @return void
-----------------------------------------------------------------------------*/
void OS_OnIdle(void)
{
  PC10_On();
  PC10_Off();

  /* Stop the CPU and Wait for Interrupt */
  Wait_For_Interrupt();
}


/*----------------------------------------------------------------------------
- @brief OS_msDelay

- @desc Puts the current thread into the delayed set for a given number
        of ticks and triggers rescheduling

- @param Ticks   Delay duration in system ticks

- @return void
-----------------------------------------------------------------------------*/
void OS_msDelay(uint32_t Ticks)
{
  Disable_Irq();

  OS_Curr->TimeOut = Ticks;

  OS_ReadySet   &= ~(1U << (OS_Curr->Prio - 1U));
  OS_DelayedSet |=  (1U << (OS_Curr->Prio - 1U));

  OS_Sched();

  Enable_Irq();
}

/*----------------------------------------------------------------------------
- @brief OS_Tick

- @desc Updates delayed threads each system tick, moves threads whose
        timeouts expire into the ready set.

- @param void

- @return void
-----------------------------------------------------------------------------*/
void OS_Tick(void)
{
  uint32_t pendingDelayedThreads = OS_DelayedSet;

  while (pendingDelayedThreads != 0U)
  {
    /* Find the highest-priority delayed thread */
    uint32_t Index      = LOG2(pendingDelayedThreads);
    OSThread *Thread    = OS_Thread[Index];
    uint32_t ThreadBit  = (1U << (Thread->Prio - 1U));

    /* Decrement timeout counter */
    if (--Thread->TimeOut == 0U)
    {
      /* Timeout expired: move thread to ready set */
      OS_ReadySet   |= ThreadBit;
      OS_DelayedSet &= ~ThreadBit;
    }

    /* Remove this thread from local working set */
    pendingDelayedThreads &= ~ThreadBit;
  }
}


/*----------------------------------------------------------------------------
- @brief OS_Run

- @desc

- @param void

- @return void
-----------------------------------------------------------------------------*/
void OS_Run(void)
{
  /* callback to configure and start interrupts */
  OS_OnStartup();

  Disable_Irq();
  OS_Sched();
  Enable_Irq();
}


/*----------------------------------------------------------------------------
- @brief OSThread_Start

- @desc  Initializes a thread’s stack and TCB, pre-fills stack for debugging,
         and marks the thread as ready to run in the OS.

- @param TCB  Thread   : Control block pointer
         Prio Thread   : Priority
         ThreadHandler : Entry function for the thread
         StkStorage    : Stack memory base address
         StkSize       : Stack size

- @return void
-----------------------------------------------------------------------------*/
void OSThread_Start(OSThread *TCB, uint8_t Prio, OSThreadHandler ThreadHandler, void *StkStorage, uint32_t StkSize)
{
  /* round down the stack top to the 8-byte boundary
  * NOTE: ARM Cortex-M stack grows down from hi -> low memory
  */
  uint32_t *StckPointer = (uint32_t *)((((uint32_t)StkStorage + StkSize) / 8) * 8);

  uint32_t *StckLimit;

  /* Initialize Cortex-M exception stack frame (automatically saved on exception entry) */
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

  /* Save top of stack pointer in TCB */
  TCB->MyStckPointer = StckPointer;

  /* Round bottom of stack up to 8-byte boundary for pre-fill */
  StckLimit = (uint32_t *)(((((uint32_t)StkStorage - 1U) / 8U) + 1U) * 8U);

  /* Pre-fill unused stack space with a known pattern for debugging */
  for (StckPointer = StckPointer - 1U; StckPointer >= StckLimit; --StckPointer)
  {
    *StckPointer = 0xFACEB00CUL;
  }

  /* Register thread with the OS */
  OS_Thread[Prio] = TCB;
  TCB->Prio       = Prio;

  /* Make thread ready to run (except priority 0, reserved for idle) */
  if(Prio > 0U)
  {
    OS_ReadySet |= (1U << (Prio - 1U));
  }

}


/*----------------------------------------------------------------------------
- @brief PendSV_Handler

- @desc Performs RTOS context switching: saves current thread state,
        restores next thread state, and updates OS_Curr pointer.

- @param void

- @return void
-----------------------------------------------------------------------------*/
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
