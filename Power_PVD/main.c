#include "brdClock.h"
#include "brdLed.h"
#include "brdUtils.h"
#include "brdPower.h"
#include "brdLCD.h"


//  Для наблюдения ошибки закомментировать!
//    Флаг PVD не сбрасывается за один вызов POWER_ClearFlag
//    Прерывание возникает вторично и так далее.
#define BUG_PVD_FIX

//  Ускорение выставления уровня PVD, просто как факт. Практической пользы не имеет.
//#define SET_LEVEL_PVD_FASTER

//---------   Настройки LED статуса ----------

#define LED_OK            BRD_LED_1
#define LED_FAULT         BRD_LED_2

#define LED_STATUS_ERR    (BRD_LED_1 | BRD_LED_2)

#define LED_OK_SHOW_DELAY    8000000
#define LED_FAULT_PERIOD      100000
uint32_t LED_State = 0;


//---------   Выбор индикации статуса для 1986ВЕ92  ----------
// PORT_C pins [0, 1] - конфликт ресурсов LED и LCD)
#ifdef USE_BOARD_VE_92
  // Закомментировать для наблюдения статуса на светодиоде
  // Раскомментировать для наблюдения результатов для LCD
  #define LCD_ENA
  
  #ifdef  LCD_ENA
    #define LCD_INIT              BRD_LCD_Init()
    #define LCD_SHOW_LEVELS(a,b)  LCD_ShowVolts(a,b)
    #define LCD_SHOW_DELAY(a,b)   LCD_ShowDelays(a, b)
    
    #define LED_OK_SWITCH         __nop();
    #define LED_FAULT_SWITCH      __nop();
  #else
    #define LCD_INIT              __nop();
    #define LCD_SHOW_LEVELS(a,b)  __nop();
    #define LCD_SHOW_DELAY(a,b)   __nop();
    
    #define LED_OK_SWITCH         LedOK_Switch()  
    #define LED_FAULT_SWITCH      LedFault_Switch()
  #endif
  
#else  // Для 1986ВЕ1
  #define LCD_INIT                BRD_LCD_Init()
  
  #define LCD_SHOW_LEVELS(a,b)    LCD_ShowVolts(a,b)
  #define LCD_SHOW_DELAY(a,b)     LCD_ShowDelays(a, b)
  
  #define LED_OK_SWITCH           LedOK_Switch()
  #define LED_FAULT_SWITCH        LedFault_Switch()
#endif


//---------   Настройки детектора напряжения ----------
#define PVD_LEVEL_MIN     PWR_PVDlevel_2V8
#define PVD_LEVEL_MAX     PWR_PVDlevel_3V4
#define PVD_LEVEL_INC     (1 << 3)

BRD_Power_Obj PowerCfg = {
  .LevelPVD =     PVD_LEVEL_MAX,
  .LevelPVDBat =  PWR_PVBDlevel_3V0,
  .EventInvMask = 0,            //  Event On: Ucc > LevelPVD
  .IrqEnaMask =   POWER_PVD_IT
} ;

//---------   Задержка выставления LevelPVD ----------
#define  WAIT_TIME_MAX     100

uint32_t PVD_Level_Top, PVD_Level_Bot;
uint32_t WaitIrqFlag;
uint32_t DoEventInv;

uint32_t SearchForLevelUcc(uint32_t pvdLevel1, uint32_t pvdLevel2, uint32_t delay);
void SetLevelPVD(uint32_t levelPVD);
void SetLevelPVD_D(uint32_t levelPVD, uint32_t delay);
void PVD_InverseEventAndClear(void);

void LedOK_Switch(void);
void LedFault_Switch(void);

void LCD_ShowVolts(uint32_t levelTop, uint32_t levelBot);
void LCD_ShowDelays(uint32_t riseT, uint32_t fallT);

int main(void)
{
  uint32_t cntRise, cntFall;
  
  //  Clock from HSE 80MHz
  BRD_Clock_Init_HSE_PLL(RST_CLK_CPU_PLLmul10);
  
  //  LEDs
  BRD_LEDs_Init();
  
  //  PVD Init
  BRD_Power_Init(&PowerCfg);
  Delay(WAIT_TIME_MAX);
	POWER_ClearFlag(POWER_FLAG_PVD);
	POWER_ClearFlag(POWER_FLAG_PVD);
  
  //  Seek for LevelPVD < Ucc, INV = 0
  PVD_Level_Bot = SearchForLevelUcc(PVD_LEVEL_MAX, PVD_LEVEL_MIN, WAIT_TIME_MAX);
  if (!(PVD_Level_Bot))
  { 
    // Exit
    BRD_LED_Set(LED_OK, 1);
    while (1);
  }

  //  PVD is set now
  //  Invert PVD condition and clear active PVD flag, INV = 1
  BRD_Power_InverseEvent(POWER_PVD_INV);
	POWER_ClearFlag(POWER_FLAG_PVD); 
  POWER_ClearFlag(POWER_FLAG_PVD);
  
  //  Seek for LevelPVD > Ucc, INV = 1  
  PVD_Level_Top = SearchForLevelUcc(PVD_LEVEL_MIN, PVD_LEVEL_MAX, WAIT_TIME_MAX);
  if (!(PVD_Level_Top))
  { 
    // Exit
    BRD_LED_Set(LED_FAULT, 1);
    while (1);
  }
  
  //  Show Levels to LCD
  LCD_INIT;
  LCD_SHOW_LEVELS(PVD_Level_Top, PVD_Level_Bot);
  
  //  Start Testing
  DoEventInv = 1;
  // Set INV = 0
  PVD_InverseEventAndClear();
  BRD_PowerPVD_EnaIRQ(POWER_PVD_IT, 1);    
  while (1)
  {
    cntRise = cntFall = 0;
    
    //  Rise event: PVD on Ucc > LevelPVD, INV = 0
    WaitIrqFlag = 0;
    DoEventInv = 1;

    SetLevelPVD(PVD_Level_Bot); 
    while (!WaitIrqFlag)
      ++cntFall;   
    
    //  Rise event: PVD оn Ucc < LevelPVD, INV = 1
    WaitIrqFlag = 0;
    DoEventInv = 1;

    SetLevelPVD(PVD_Level_Top); 
    while (!WaitIrqFlag)
      ++cntRise;
        
    //  Show Test Result
    LCD_SHOW_DELAY(cntRise, cntFall);
    LED_OK_SWITCH;   
    Delay(LED_OK_SHOW_DELAY);
  }
}

void POWER_IRQHandler(void)
{
  // Stop PVD event, and Clear PVD flag
  if (DoEventInv)
  {
   //  Реализует одинарное или двойное стирание PVD    
   PVD_InverseEventAndClear();
   DoEventInv = 0;
  }
  else
  { 
    //  Лишнее прерывание
    //  Не возникает при двойном сбросе
    //  Дотираем PVD
    POWER_ClearFlag(POWER_FLAG_PVD);
    //  Переключаем светодиод статуса ошибки
    LED_FAULT_SWITCH;
  }
  
  WaitIrqFlag = 1;
  
	NVIC_ClearPendingIRQ(POWER_IRQn);
}

//  -------------   PVD Control ---------------
void PVD_InverseEventAndClear(void)
{
  BRD_Power_InverseEvent(POWER_PVD_INV);
  
	POWER_ClearFlag(POWER_FLAG_PVD); 
#ifdef BUG_PVD_FIX
  POWER_ClearFlag(POWER_FLAG_PVD);    
#endif
}

void SetLevelPVD(uint32_t levelPVD)
{
#ifdef SET_LEVEL_PVD_FASTER  
//  // Не ускоряет выставление уровня
//  if (levelPVD < PWR_PVDlevel_3V4)
//    POWER_PVDlevelConfig(levelPVD + PVD_LEVEL_INC);
  
  // Ускоряет выставление уровня   
  if (levelPVD > PWR_PVDlevel_2V0)
    POWER_PVDlevelConfig(levelPVD - PVD_LEVEL_INC);  
#endif  
  
  POWER_PVDlevelConfig(levelPVD);
}

void SetLevelPVD_D(uint32_t levelPVD, uint32_t delay)
{
  SetLevelPVD(levelPVD);
  Delay(delay);
}

uint32_t SearchForLevelUcc(uint32_t pvdLevel1, uint32_t pvdLevel2, uint32_t delay)
{
  uint32_t level;
  
  if (pvdLevel1 < pvdLevel2)
    for (level = pvdLevel1; level <= pvdLevel2; level += PVD_LEVEL_INC)
    {
      SetLevelPVD_D(level, delay);

      if (POWER_GetFlagStatus(POWER_FLAG_PVD))
        return level;    
    }
  else  
    for (level = pvdLevel1; level >= pvdLevel2; level -= PVD_LEVEL_INC)
    {
      SetLevelPVD_D(level, delay);

      if (POWER_GetFlagStatus(POWER_FLAG_PVD))
        return level;    
    }
  
  return 0;
}

//  -------------   Show status LED and LCD ---------------
void LedFault_Switch(void)
{
  static uint32_t period = 0;
  
  period++;
  if (period > LED_FAULT_PERIOD)
  {
    period = 0;
    BRD_LED_Switch(LED_FAULT);
  }
}

void LedOK_Switch()
{ 
  LED_State = !LED_State;
  BRD_LED_Set(LED_OK, LED_State);
}

void LCD_ShowVolts(uint32_t levelTop, uint32_t levelBot)
{
  static char message[64];
  
  float vt = 2.0 + (levelTop >> 3) * 0.2;
  float vb = 2.0 + (levelBot >> 3) * 0.2;
  
  sprintf(message , "Ub=%.1f Ut=%.1f", vb, vt);
  BRD_LCD_Print (message, 3);
}

void LCD_ShowDelays(uint32_t riseT, uint32_t fallT)
{
  static char message[64];
  
  sprintf(message , "Tf=%d Tr=%d", fallT, riseT);
  BRD_LCD_Print (message, 5);
}


