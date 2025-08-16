#ifndef GPT_2023_08_23_H
  #define GPT_2023_08_23_H

  #include <stdbool.h>
  #include <stdint.h>

  /* Defines the type used to identify timer channels and timer counter */
  typedef uint64_t Gpt_ValueType;
  typedef uint8_t Gpt_ChannelType;

  /* Returns the elapsed time in milliseconds from the system counter. */
  Gpt_ValueType Gpt_GetTimeElapsed(const Gpt_ChannelType DummyChannelIndex);

  /*----------------------------------------------------------------------------
  - @brief TimerStart
  -
  - @desc Creates a timer deadline by adding the given duration (ms)
    to the current system time.
  -
  - @param millisec   Duration in milliseconds
  - @return Gpt_ValueType  Absolute timeout value
  -----------------------------------------------------------------------------*/
  static inline Gpt_ValueType TimerStart(const unsigned millisec)
  {
    return Gpt_GetTimeElapsed(0U) + millisec;
  }


  /*----------------------------------------------------------------------------
  - @brief TimerTimeout
  -
  - @desc Checks whether the given timer deadline has expired.
  -
  - @param MyTimer   Absolute timeout value (from TimerStart)
  - @return bool     true if expired, false otherwise
  -----------------------------------------------------------------------------*/
  static inline bool TimerTimeout(Gpt_ValueType MyTimer)
  {
    return ((Gpt_GetTimeElapsed(0U) > MyTimer) ? true : false);
  }


#endif /* GPT_2023_08_23_H */
