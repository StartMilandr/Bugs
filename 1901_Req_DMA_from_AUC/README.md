﻿# 1901_Req_DMA_from_AUC

DMA_ADC_VC1 - адаптация примера измерения АЦП с использованием DMA от 1986ВЕ92У для 1901ВЦ1. 
https://github.com/StartMilandr/6.x-DMA_Projects/tree/master/6.5-DMA_ADC/DMA_ADC_VExx

При включении тактирования ядра DSP возникает проблема, выражающаяся в том, что исполнение не выходит из обработчика прерывания DMA_IRQHandler().
Выяснилось, что к DMA висит активный запрос от FIFO ЦАПа аудиокодека, хотя тот и не включен.
В примере показаны два варианта решения проблемы, можно:
  * Разрешить работу канала DMA через запись MDR_DMA->CHNL_ENABLE_SET |= (1UL << 23), где 23 - это и есть номер канала AUC_FIFO_DAC.
  * Либо заполнить FIFO ЦАПа любыми значениями не включая сам ЦАП. Этот вариант реализован в функции BRD_ClockDSP_Init_HSE_PLL_fixDMAIRQ(), которая вынесена в модуль brdClock.c.

Макроопределения в начале main.c выбирают тот, или иной вариант. Если пути обхода не выбраны, то исполнение застревает в DMA_IRQHandler().
  #define FIX_BY_DMA_EN_AUC          0
  #define FIX_BY_FILL_AUC_DAC_FIFO   0

## ИНДИКАЦИЯ:
При перезапуске циклов DMA переключается светодиод PB15 на отладочной плате
  * PB15 мигает - исправная работа
  * PB15 НЕ мигает - программа висит в обработчике DMA_IRQHandler

## Странности:
При отладке обнаружено, что если в BRD_ClockDSP_Init_HSE_PLL_fixDMAIRQ() после включения тактирования DSP не сделать небольшую задержку delay_loc(1), то код работает только при подаче питания. При нажатии на Reset исполнение снова зависает в DMA_IRQHandler(), но уже по запросу от FIFO АЦП аудиокодека (22 канал DMA). Хотя АЦП тоже не включен. Попытка заменить delay_loc(1) на __ISB() не помогла.