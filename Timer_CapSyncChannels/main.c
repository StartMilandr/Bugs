#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_timer.h>

//  Внесение задержки перед чтением регистра CAP2
#define FIX_CAP_BY_DELAY  1
//  Уменьшение разницы частот CPU и TIM_Clock
#define FIX_CAP_BY_FREQ   0

//  Сигнал захвата формируется на выводе PD11 (разъем светодиодов - VD6 (XP19)) - уровень переключается по нажатию на кнопку Down (PC12).
//  Захватывается сигнал с вывода PD12 (разъем светодиодов - VD7 (XP20))
//  Поэтому для захвата необходимо отключить джамперы светодиодов и замкнуть VD6 <-> VD7.

//  PC12_Port - Key Down
#define KEY_PORT_CLK  RST_CLK_PCLK_PORTC
#define KEY_PORT      MDR_PORTC
#define KEY_PIN       PORT_Pin_12

#define KEY_PRESSED   (!PORT_ReadInputDataBit(KEY_PORT, KEY_PIN))

#if FIX_CAP_BY_FREQ
  #define PSC   4  // div16 - РАБОТАЕТ
#else
  #define PSC   5  // div32 - НЕ РАБОТАЕТ БЕЗ FIX_CAP_BY_DELAY = 1
#endif


void GPIO_Init(void);
void ChangeCapLevel(void);

int main(void)
{
  GPIO_Init();
  
	/* Init TIMER1 for Capture */
  RST_CLK_PCLKcmd (RST_CLK_PCLK_TIMER1, ENABLE);
  
	MDR_RST_CLK->TIM_CLOCK &= ~RST_CLK_TIM_CLOCK_TIM1_BRG_Msk;
	MDR_RST_CLK->TIM_CLOCK |= (PSC << RST_CLK_TIM_CLOCK_TIM1_BRG_Pos) | RST_CLK_TIM_CLOCK_TIM1_CLK_EN;

	MDR_TIMER1->CCR2 = 0;
	MDR_TIMER1->CCR3 = 0;
	MDR_TIMER1->CNT = 0;
	MDR_TIMER1->PSG = 0;
	MDR_TIMER1->ARR = 65535;
	MDR_TIMER1->CNTRL = (0x0 << TIMER_CNTRL_FDTS_Pos);
	MDR_TIMER1->CH1_CNTRL = 0;
	MDR_TIMER1->CH2_CNTRL = (0x1 << TIMER_CH_CNTRL_CAP_NPWM_Pos) | (0x2 << TIMER_CH_CNTRL_CHFLTR_Pos) | (0x2 << TIMER_CH_CNTRL_CHSEL_Pos);
	MDR_TIMER1->CH3_CNTRL = (0x1 << TIMER_CH_CNTRL_CAP_NPWM_Pos) | (0x2 << TIMER_CH_CNTRL_CHFLTR_Pos) | (0x0 << TIMER_CH_CNTRL_CHSEL_Pos);
	MDR_TIMER1->CH4_CNTRL = 0;
	MDR_TIMER1->CH1_CNTRL1 = MDR_TIMER1->CH2_CNTRL1 = MDR_TIMER1->CH3_CNTRL1 = MDR_TIMER1->CH4_CNTRL1 = 0;
	MDR_TIMER1->BRKETR_CNTRL = 0;
	MDR_TIMER1->DMA_RE = 0;

  MDR_TIMER1->IE = (0x6 << TIMER_IE_CCR_CAP_EVENT_IE_Pos);

	NVIC_SetPriority (Timer1_IRQn, 1); // Приоритет прерывания
	NVIC_EnableIRQ(Timer1_IRQn);
  
  MDR_TIMER1->CNTRL |= TIMER_CNTRL_CNT_EN;
  
  while (1)
  {
    if (KEY_PRESSED)
    {
      while (KEY_PRESSED) {};
        
      ChangeCapLevel();
    }
  }
}

#define JTAG_B_PROTECT 0xFFE0UL

void ChangeCapLevel(void)
{
  uint32_t regVal = MDR_PORTD->RXTX;
  regVal ^= PORT_Pin_11;
  MDR_PORTD->RXTX = regVal & JTAG_B_PROTECT;
}

#define  LOG_LEN   100
uint32_t logData[LOG_LEN];
uint32_t logWr = 0;

uint32_t delay = 1;

void Delay(uint32_t ticks)
{
  while (ticks--){};
}

void Timer1_IRQHandler(void) 
{
#if FIX_CAP_BY_DELAY  
  Delay(delay);
#endif
  
  uint32_t STATUS = MDR_TIMER1->STATUS;
  uint32_t ccr1 = MDR_TIMER1->CCR2;    
  uint32_t ccr2 = MDR_TIMER1->CCR3;  

	MDR_TIMER1->STATUS = 0;

  if (logWr >= LOG_LEN)
    logWr = 0;
  
  //logData[logWr++] = 0xAAAA;
  logData[logWr++] = ccr1;
  logData[logWr++] = ccr2;
  //logData[logWr++] = 0xBBBB;
    
  NVIC_ClearPendingIRQ(Timer1_IRQn);
}

void GPIO_Init(void)
{ 
  PORT_InitTypeDef GPIOInitStruct;

  RST_CLK_PCLKcmd (KEY_PORT_CLK | RST_CLK_PCLK_PORTD, ENABLE);

  PORT_StructInit (&GPIOInitStruct);
  GPIOInitStruct.PORT_MODE      = PORT_MODE_DIGITAL;  
  
  //  PD12_Main - TIM1_CH3 Capture input
  GPIOInitStruct.PORT_Pin       = PORT_Pin_12;
  GPIOInitStruct.PORT_FUNC      = PORT_FUNC_MAIN;
  PORT_Init (MDR_PORTD, &GPIOInitStruct);
  
  //  PC12_Port - Key Down
  GPIOInitStruct.PORT_Pin       = KEY_PIN;
  GPIOInitStruct.PORT_FUNC      = PORT_FUNC_PORT;
  PORT_Init (KEY_PORT, &GPIOInitStruct);
  
  //  PD11_Port - Make Front by Key Down
  GPIOInitStruct.PORT_Pin        = PORT_Pin_11;
  GPIOInitStruct.PORT_OE         = PORT_OE_OUT;
  GPIOInitStruct.PORT_SPEED      = PORT_SPEED_FAST;
  GPIOInitStruct.PORT_FUNC       = PORT_FUNC_PORT;
	PORT_Init(MDR_PORTD, &GPIOInitStruct);  
}

