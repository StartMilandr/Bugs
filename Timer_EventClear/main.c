#include "brdDef.h"

#include "brdClock.h"
#include "brdLed.h"
#include "brdTimer.h"
#include "brdUtils.h"

#include "brdTimer_Select.h"

//  ------------    BUG ACTIVATE----------

//#define BUG_ACTIVATE

//  --------------------------------

#ifdef USE_BOARD_VE_92
  #define  LED_SEL_ALL         (BRD_LED_1 | BRD_LED_2)
  #define  LED_COUNT           2
  uint32_t LED_Sel[LED_COUNT] = {BRD_LED_1, BRD_LED_2};

#else
  #define  LED_SEL_ALL         (BRD_LED_1 | BRD_LED_2 | BRD_LED_3 | BRD_LED_4)
  #define  LED_COUNT           4
  uint32_t LED_Sel[LED_COUNT] = {BRD_LED_1, BRD_LED_2, BRD_LED_3, BRD_LED_4};
#endif

#define  PLL_MUX       RST_CLK_CPU_PLLmul10
#define  LED_FREQ_HZ   1
  
uint32_t DoLedSwitch = 0;
uint32_t DoErrSet = 0;

uint32_t IRQCount = 0;
uint32_t LEDCount = 0;

int main()
{
  uint32_t ledShowSel;
  uint32_t i;
  
  TIMER_CntInitTypeDef TimerInitStruct;
  
  //  Clock 80MHz
  BRD_Clock_Init_HSE_PLL(PLL_MUX);
  //  LEDs
  BRD_LEDs_Init();
  BRD_LED_Set(LED_SEL_ALL, 0);
  
  //  Timer
  BRD_Timer_InitStructDef(&TimerInitStruct, LED_FREQ_HZ, 64000);  // 1Hz, 64000 - ARR desired value
  BRD_Timer_Init(&brdTimer1, &TimerInitStruct);
  BRD_Timer_InitIRQ(&brdTimer1, 1);
  BRD_Timer_Start(&brdTimer1);
  
  while (1)
  {
    if (LEDCount != IRQCount)
    {  
      LEDCount = IRQCount;
      
      //  Led Show
      ledShowSel = 0;
      for (i = 0; i < LED_COUNT; ++i)
        if (LEDCount & (1 << i))
          ledShowSel |= LED_Sel[i];
      
      BRD_LED_Set(LED_SEL_ALL, 0);
      BRD_LED_Set(ledShowSel, 1);
    }
  }
}


#if defined (USE_BOARD_VE_8)
  #define TimerX_IRQHandler     INT_TMR1_Handler

#elif defined (USE_BOARD_VE_1)
  #define TimerX_IRQHandler     TIMER1_IRQHandler

#else
  #define TimerX_IRQHandler     Timer1_IRQHandler

#endif

void TimerX_IRQHandler (void)  
{
  IRQCount++;    

#ifdef BUG_ACTIVATE  
  MDR_TIMER1->STATUS &= ~TIMER_STATUS_CNT_ARR;
#else  
  TIMER_ClearITPendingBit (MDR_TIMER1, TIMER_STATUS_CNT_ARR);
#endif  
}

