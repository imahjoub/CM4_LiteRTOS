#ifndef OS_2025_08_02_H
  #define OS_2025_08_02_H

  #include <stdint.h>

  /* Thread Control Block (TCB) */
  typedef struct
  {
    void     *MyStckPointer;  /* Stack pointer */
    uint8_t  Prio;            /* Thread priority */
    uint32_t TimeOut;         /* Timeout delay down-counter */
  } OSThread;

  typedef void (*OSThreadHandler)();

  /* Initializes the operating system */
  void OS_Init(void *StackStorage, uint32_t SatckSize);

  /* Execute Idle thread */
  void OS_OnIdle(void);

  /* This function must be called with interrupts DISABLED */
  void OS_Sched(void);

  /* Transfer control to the RTOS to run the threads */
  void OS_Run(void);

  /* Blocking delay */
  void OS_msDelay(uint32_t Ticks);

  /* Process all timeouts */
  void OS_Tick(void);

  /* Callback to configure and start interrupts */
  void OS_OnStartup(void);

 /* Initializes a thread control block (TCB) and sets up its stack frame for execution by the OS. */
  void OSThread_Start(OSThread *TCB, uint8_t Prio, OSThreadHandler ThreadHandler, void *StkStorage, uint32_t StkSize);


#endif /* OS_2025_08_02_H */
