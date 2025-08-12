#ifndef OS_2025_08_10_H
  #define OS_2025_08_10_H

  #include <stdint.h>
  #include <stddef.h>

  extern uint32_t Blinky_Stack   [40U];
  extern uint32_t TogglePC3_Stack[40U];

  extern void Blinky_Main    (void);
  extern void TogglePC3_Main (void);

  OSThread Blinky_Thread;
  OSThread TogglePC3_Thread;

  typedef void (*TaskInitFunc)(void);
  typedef void (*TaskMainFunc)(void);

  typedef struct
  {
    OSThread*       Thread;             // Optional init function
    uint8_t         Priority;           // Thread priority
    OSThreadHandler Task;               // Main thread entry function
    void*           StackStorage;         // Pointer to stack memory
    size_t          StackSize;           // Stack size in bytes
  } OS_TaskConfig_t;

  #define OS_CFG_TASK_LIST_ENTRY(Thread, Priority, Task, StackStorage, StackSize) { (Thread), (Priority), (Task), (StackStorage), (StackSize) }

  OS_TaskConfig_t OsTaskList[] =
  {
    OS_CFG_TASK_LIST_ENTRY(&Blinky_Thread,    3U,    Blinky_Main,    Blinky_Stack, sizeof(Blinky_Stack)),
    OS_CFG_TASK_LIST_ENTRY(&TogglePC3_Thread, 2U, TogglePC3_Main, TogglePC3_Stack, sizeof(TogglePC3_Stack))
  };

const uint32_t OsTaskCount = sizeof(OsTaskList) / sizeof(OsTaskList[0U]);




#endif /* OS_2025_08_10_H */
