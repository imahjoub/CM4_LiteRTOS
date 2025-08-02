#ifndef GPT_2023_08_23_H
  #define GPT_2023_08_23_H

  #include <stdbool.h>
  #include <stdint.h>

  typedef uint64_t Gpt_ValueType;
  typedef uint8_t Gpt_ChannelType;

  Gpt_ValueType Gpt_GetTimeElapsed(const Gpt_ChannelType DummyChannelIndex);

  static inline Gpt_ValueType TimerStart(const unsigned millisec)
  {
    return Gpt_GetTimeElapsed(0U) + millisec;
  }

  static inline bool TimerTimeout(Gpt_ValueType MyTimer)
  {
    return ((Gpt_GetTimeElapsed(0U) > MyTimer) ? true : false);
  }

  void BSP_delay(uint32_t ticks);


#endif /* GPT_2023_08_23_H */
