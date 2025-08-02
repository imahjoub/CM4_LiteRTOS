#include <stdint.h>
#include "Os.h"

__attribute__ ((naked)) void PendSV_Handler(void);

OSThread * volatile OS_Curr; /* pointer to the current thread */
OSThread * volatile OS_Next; /* pointer to the next thread to run */

OSThread *OS_Thread[32U + 1U]; /* array of threads started so far */
uint8_t   OS_ThreadNum;        /* number of threads started so far */
uint8_t   OS_CurrIdx;          /* current thread index for round robin scheduling */

void OS_Init(void)
{
  /* set the PendSV interrupt priority to the lowest level 0xFF */
  *(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16);
}

void OS_Sched(void)
{
  /* OS_next = ... */
  ++OS_CurrIdx;

  if(OS_CurrIdx == OS_ThreadNum)
  {
    OS_CurrIdx = 0U;
  }

  OS_Next = OS_Thread[OS_CurrIdx];

  /* trigger PendSV, if needed */
  if(OS_Next != OS_Curr)
  {
    *(uint32_t volatile *)0xE000ED04 = (1UL << 28U);
  }
}

void OS_OnIdle(void)
{
#ifdef NDBEBUG
  __WFI(); /* stop the CPU and Wait for Interrupt */
#endif
}


void OS_Run(void)
{
  /* callback to configure and start interrupts */
  OS_OnStartup();

  __asm volatile ("cpsid i");
  OS_Sched();
  __asm volatile ("cpsie i");
}

void OSThread_Start(OSThread *me, OSThreadHandler threadHandler, void *stkSto, uint32_t stkSize)
{
  /* round down the stack top to the 8-byte boundary
  * NOTE: ARM Cortex-M stack grows down from hi -> low memory
  */
  uint32_t *sp = (uint32_t *)((((uint32_t)stkSto + stkSize) / 8) * 8);

  uint32_t *stk_limit;

  *(--sp) = (1U << 24);              /* xPSR */
  *(--sp) = (uint32_t)threadHandler; /* PC   */
  *(--sp) = 0x0000000EU;             /* LR   */
  *(--sp) = 0x0000000CU;             /* R12  */
  *(--sp) = 0x00000003U;             /* R3   */
  *(--sp) = 0x00000002U;             /* R2   */
  *(--sp) = 0x00000001U;             /* R1   */
  *(--sp) = 0x00000000U;             /* R0   */
  /* additionally, fake registers R4-R11     */
  *(--sp) = 0x0000000BU;             /* R11  */
  *(--sp) = 0x0000000AU;             /* R10  */
  *(--sp) = 0x00000009U;             /* R9   */
  *(--sp) = 0x00000008U;             /* R8   */
  *(--sp) = 0x00000007U;             /* R7   */
  *(--sp) = 0x00000006U;             /* R6   */
  *(--sp) = 0x00000005U;             /* R5   */
  *(--sp) = 0x00000004U;             /* R4   */

  /* save the top of the stack in the thread's attibute */
  me->sp = sp;

  /* round up the bottom of the stack to the 8-byte boundary */
  stk_limit = (uint32_t *)(((((uint32_t)stkSto - 1U) / 8) + 1U) * 8);

  /* pre-fill the unused part of the stack with 0xDEADBEEF */
  for (sp = sp - 1U; sp >= stk_limit; --sp)
  {
    *sp = 0xDEADBEEFU;
  }

  /* register the thread with the OS */
  OS_Thread[OS_ThreadNum] = me;

  ++OS_ThreadNum;
}

/* inline assembly syntax for Compiler 6 (ARMCLANG) */
__attribute__ ((naked)) void PendSV_Handler(void)
{
  __asm volatile
  (
    /* __disable_irq(); */
    "  CPSID         I                 \n"

    /* if (OS_curr != (OSThread *)0) { */
    "  LDR           r1,=OS_Curr       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  CMP           r1,#0             \n"
    "  BEQ           PendSV_restore    \n"

    /*     push registers r4-r11 on the stack */
    "  PUSH          {r4-r11}          \n"

    /*     OS_curr->sp = sp; */
    "  LDR           r1,=OS_Curr       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  MOV           r0,sp             \n"
    "  STR           r0,[r1,#0x00]     \n"
    /* } */

    "PendSV_restore:                   \n"
    /* sp = OS_next->sp; */
    "  LDR           r1,=OS_Next       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  LDR           r0,[r1,#0x00]     \n"
    "  MOV           sp,r0             \n"

    /* OS_curr = OS_next; */
    "  LDR           r1,=OS_Next       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  LDR           r2,=OS_Curr       \n"
    "  STR           r1,[r2,#0x00]     \n"

    /* pop registers r4-r11 */
    "  POP           {r4-r11}          \n"

    /* __enable_irq(); */
    "  CPSIE         I                 \n"

    /* return to the next thread */
    "  BX            lr                \n"
  );
}
