#include "spec.h"
#include <mdr32f8_port.h>
#include <mdr32f8_clkctrl.h>

PORT_InitTypeDef PORT_InitStructure;

#define TO_ULIM_PVD_LEVEL(Vx10) (((Vx10) - 12) / 2)

#define PVD_LEVEL_MIN  TO_ULIM_PVD_LEVEL(28)   // Uref = 2.8V
#define PVD_LEVEL_MAX  TO_ULIM_PVD_LEVEL(38)   // Uref = 3.8V

#define WAIT_PVD_LEVEL_TICKS 1000000           // Определить в зависимости от частоты ядра
#define PVDP_CHECK_COUNT  20

#define USE_PVD_HACK

#define LED_VD7       PORT_Pin_16
#define LED_PERIOD    500000

void LED_Init(void);

void     PVD_SetLevel(uint32_t pvdLevel, uint32_t delayTicks);
void     PVD_Init(uint32_t pvdLevel, uint32_t delayTicks);
uint32_t PVD_SetLevelOfUccFall(uint32_t formLevelMax, uint32_t toLevelMin, uint32_t flagCheckCount, uint32_t delayTicks);

void Delay(uint32_t ticks);

volatile uint32_t pvdLevelUccFall;

int main()
{  
	POR_disable();

  //  Clock
	CLKCTRL_DeInit();
	CLKCTRL_HSEconfig(CLKCTRL_HSE0_CLK_ON);
	while(CLKCTRL_HSEstatus(CLKCTRL_HSEn_STAT_HSE0_RDY) != SUCCESS){}
		
	CLKCTRL_MAX_CLKSelection(CLKCTRL_MAX_CLK_HSE0div1);	  

  // Индикация прерывания
  LED_Init();
    
    
  // 1 -  PVD Init, Uref выставляем в заведомо > Ucc
  PVD_Init(PVD_LEVEL_MAX, WAIT_PVD_LEVEL_TICKS);
    
  // 2 -  Понижаем Uref пока не появится флаг PVDP - (Uref < Ucc) - Это значение и будет порогом срабатывания при просадке питания
  pvdLevelUccFall = PVD_SetLevelOfUccFall(PVD_LEVEL_MAX, PVD_LEVEL_MIN, PVDP_CHECK_COUNT, WAIT_PVD_LEVEL_TICKS);
    
  // 3 - Инверсия флага PVDP - теперь прерывания будут срабатывать по (Ucc < Uref)    
  PWR->CNTR2 = 1;
  PWR->STAT = 1;

  // 4 - Включение прерывания по Ucc < Uref
  NVIC_EnableIRQ(PVD_IF_IRQn);   
  PWR->CNTR1 = 1;

     
  // 5 - Ожидание прерывания    
  while (1)    
  {
 //  Отладка 
    //  Индикация
    PORT_SetBits(PORTC, LED_VD7);
    Delay(LED_PERIOD);
    
    // Генерация прерывания по Ucc < Uref    
#ifdef USE_PVD_HACK    
    PVD_SetLevel(pvdLevelUccFall + 1, WAIT_PVD_LEVEL_TICKS);
#else
    PVD_SetLevel(pvdLevelUccFall + 2, WAIT_PVD_LEVEL_TICKS);  // Выставляется неточно, поэтому ставится с запасом. При +1 не работает.
#endif
    Delay(LED_PERIOD);
  }  
}

void PVD_IF_Handler(void)
{
  //  Сброс Ucc < Uref
  if (pvdLevelUccFall != PWR->ULIMIT)
  {  
    PVD_SetLevel(pvdLevelUccFall, 0);
    
    PORT_ResetBits(PORTC, LED_VD7);
  }  
  PWR->STAT = 1;
}  

void PVD_SetLevel(uint32_t pvdLevel, uint32_t delayTicks)  
{
#ifdef USE_PVD_HACK  
  if (pvdLevel < 0x1F)
    PWR->ULIMIT = pvdLevel + 1;
#endif 

  PWR->ULIMIT = pvdLevel;

  if (delayTicks > 0)  
    Delay(delayTicks);  
}

void PVD_Init(uint32_t pvdLevel, uint32_t delayTicks)
{
  // Clock for Power
	CLKCTRL_PER0_CLKcmd(CLKCTRL_PER0_CLK_MDR_PWR_EN, ENABLE);
	PWR->KEY =_KEY_;
    
  PWR->CNTR0 = 0x1;
  PWR->CNTR1 = 0;
  PWR->CNTR2 = 0;
  
  PWR->CLIMIT = 0x0003FFFF;

  PVD_SetLevel(pvdLevel, delayTicks);
}  

uint32_t PVD_SetLevelOfUccFall(uint32_t formLevelMax, uint32_t toLevelMin, uint32_t flagCheckCount, uint32_t delayTicks)
{
  uint32_t iLevel;
  uint32_t i;
  uint32_t eventCount;
  
  for (iLevel = formLevelMax; iLevel >= toLevelMin; iLevel -= 1)
  {
    //PVD_SetLevel(iLevel, delayTicks);  - При снижении Uref хак не требуется
    PWR->ULIMIT = iLevel;
    
    eventCount = 0;
    for (i = 0; i < flagCheckCount; ++i)
    {
      PWR->STAT = 1;
      Delay(10);    
      
      if (PWR->STAT & 0x1)
        eventCount++;       
    }  
    
    if (eventCount ==  flagCheckCount)
      return iLevel;
  }
  
  //  Fault result
  return 0;
}

void Delay(uint32_t ticks)
{
	uint32_t i;
	for(i = 0; i < ticks; i++){}
}

void LED_Init(void)
{  
	CLKCTRL_PER0_CLKcmd(CLKCTRL_PER0_CLK_MDR_PORTC_EN, ENABLE);
	PORTC->KEY = _KEY_;  
  
  PORT_StructInit(&PORT_InitStructure);
	PORT_InitStructure.PORT_Pin      = LED_VD7;	
        
  PORT_InitStructure.PORT_SOE      = PORT_SOE_OUT;
  PORT_InitStructure.PORT_SANALOG  = PORT_SANALOG_DIGITAL;
	PORT_InitStructure.PORT_SPD      = PORT_SPD_OFF;
	PORT_InitStructure.PORT_SPWR     = PORT_SPWR_10;

  PORT_Init(PORTC, &PORT_InitStructure);
}
