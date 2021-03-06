﻿# Timer_EventClear
При сбросе флага события в регистре STATUS последней командой в таймерном прерывании, возникает вторичное прерывание.  Проверено на 
  * 1986ВЕ92У ревизия 4
  * 1986ВЕ8Т ревизия 4

В 1986ВЕ1Т ревизия 4 не обнаружено.

Проблемный код:

    void TimerX_IRQHandler (void)  
    {
      IRQCount++;
      MDR_TIMER1->STATUS &= ~TIMER_STATUS_CNT_ARR;
    }

## Предполагаемая причина
Сброс сигнала запроса прерывания к NVIC происходит позже, чем выход из таймерного прерывания. Поэтому происходит вторичное прерывание.

## Варианты обхода

1 - Сбрасывать флаг первой же командой, тогда последующие инструкции обеспечат необходимую задержку:

    void TimerX_IRQHandler (void)  
    {
      MDR_TIMER1->STATUS &= ~TIMER_STATUS_CNT_ARR; 
      IRQCount++;  
    }

2 - Использовать сброс через функцию SPL. Вносимой функцией задержки достаточно даже при максимальной частоте ядра.

    void TimerX_IRQHandler (void)  
    {
      IRQCount++;
      TIMER_ClearITPendingBit (MDR_TIMER1, TIMER_STATUS_CNT_ARR);
    }

## Описание примера
Пример реализован для 1986ВЕ1Т, 1986ВЕ92У, 1986ВЕ8Т. При двойном прерывании светодиоды начинают показывать IRQCount с первого разряда (светодиода). Без ошибки - с нулевого разряда. 
