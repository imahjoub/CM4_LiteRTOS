#ifndef OS_2023_08_23_H
  #define OS_2023_08_23_H

  #include <stdint.h>

  /* Thread Control Block (TCB) */
  typedef struct
  {
    void *sp; /* stack pointer */
  } OSThread;

  typedef void (*OSThreadHandler)();

  void OS_Init(void);

  void OS_OnIdle(void);

  /* this function must be called with interrupts DISABLED */
  void OS_Sched(void);

  /* transfer control to the RTOS to run the threads */
  void OS_Run(void);

  /* callback to configure and start interrupts */
  void OS_OnStartup(void);

  void OSThread_Start( OSThread *me, OSThreadHandler threadHandler, void *stkSto, uint32_t stkSize);


#endif /* OS_2023_08_23_H */
