#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_bkp.h>

//  Прототипы функций
void CPU_Initialize (void);
void BKP_Init_HSI(void);
void LED_Init(void);

uint32_t a = 0, cnt = 0;

#define HSI_ON (uint32_t) (1<<22)

uint32_t i = 0;
	

int main()
{
	CPU_Initialize(); // Переход тактирования МК на HSE осциллятор
	LED_Init();
	BKP_DeInit();
  BKP_Init_HSI(); // Запуск RTC от HSI
	PORT_SetBits (MDR_PORTC, PORT_Pin_0|PORT_Pin_1); // LED_ON
	 
	while ((MDR_BKP->RTC_CS & BKP_RTC_FLAG_ALRF)==0); // Ождание установки флага ALRF
	
	MDR_BKP->REG_0F &= ~BKP_REG_0F_HSI_ON; // Сброс бита HSI_ON (Выключение HSI генератора)                   
	//BKP_RTC_WaitForUpdate(); // Ожидание завершения записи
	//MDR_BKP->RTC_CS |= BKP_RTC_FLAG_ALRF;  // Сброс флага ALRF (приводит к включению генератора HSI!!!)

	if (!(MDR_BKP->REG_0F& (1<<22))) // Проверка бита HSI_ON 
		PORT_ResetBits (MDR_PORTC, PORT_Pin_0); // LED0_OFF

	if (!(MDR_BKP->REG_0F& (1<<23))) // Проверка бита HSI_RDY   
		PORT_ResetBits (MDR_PORTC, PORT_Pin_1); // LED1_OFF
	
	while (1);

}

void BKP_Init_HSI()
{
	 /* Enables the HSI clock for BKP control */
  RST_CLK_PCLKcmd(RST_CLK_PCLK_BKP,ENABLE);

  /* RTC reset */
  BKP_RTC_Reset(ENABLE);
  BKP_RTC_Reset(DISABLE);

  /* Configure RTC_HSI as RTC clock source */
  RST_CLK_HSIadjust(25);
  RST_CLK_RTC_HSIclkEnable(ENABLE);
	
  RST_CLK_HSIclkPrescaler(RST_CLK_HSIclkDIV8); // Делитель частоты HSI на 8
  BKP_RTCclkSource(BKP_RTC_HSIclk);
	
	/* Configure RTC_LSE as RTC clock source */
	//RST_CLK_LSEconfig(RST_CLK_LSE_ON);
  //while (RST_CLK_LSEstatus()!=SUCCESS);
  //BKP_RTCclkSource(BKP_RTC_LSEclk);
	
  /* Set the RTC counter value */
  BKP_RTC_WaitForUpdate();
  BKP_RTC_SetCounter(0);

  /* Set the RTC prescaler value */
  BKP_RTC_WaitForUpdate();
  BKP_RTC_SetPrescaler(1000000); // Для счёта 1 значения в 1 секунду HSI
	//BKP_RTC_SetPrescaler(32768); // Для счёта 1 значения в 1 секунду LSE

  /* Set the RTC alarm value */
  BKP_RTC_WaitForUpdate();
  BKP_RTC_SetAlarm(4); // 4 секунды

  /* RTC enable */
  BKP_RTC_WaitForUpdate();
  BKP_RTC_Enable(ENABLE);	
}

void LED_Init()
{
	PORT_InitTypeDef GPIOInitStruct; //initialization of ports
	
	RST_CLK_PCLKcmd (RST_CLK_PCLK_PORTC, ENABLE);

	PORT_StructInit (&GPIOInitStruct);
	GPIOInitStruct.PORT_Pin        = PORT_Pin_0|PORT_Pin_1;
  GPIOInitStruct.PORT_OE         = PORT_OE_OUT;
  GPIOInitStruct.PORT_SPEED      = PORT_SPEED_SLOW;
  GPIOInitStruct.PORT_MODE       = PORT_MODE_DIGITAL; 

	PORT_Init(MDR_PORTC, &GPIOInitStruct);
}

// Инициализация системы тактирования микроконтроллера
void CPU_Initialize (void)
{
  /// Сброс настроек системы тактирования
  RST_CLK_DeInit();

  // Инициализация генератора на внешнем кварцевом резонаторе (HSE)
  RST_CLK_HSEconfig (RST_CLK_HSE_ON);
  while (RST_CLK_HSEstatus() != SUCCESS);

   // Инициализация блока PLL
  // Включение тактирования PLL
  RST_CLK_CPU_PLLcmd (ENABLE);

  // Настройка источника и коэффициента умножения PLL
  // (CPU_C1_SEL = HSE)
  RST_CLK_CPU_PLLconfig (RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul10);
  while (RST_CLK_CPU_PLLstatus() != SUCCESS);

   // Подключение PLL к системе тактирования
  // (CPU_C2_SEL = PLLCPUo)
  RST_CLK_CPU_PLLuse (ENABLE);

  // Настройка коэффициента деления блока CPU_C3_SEL
  // (CPU_C3_SEL = CPU_C2)
  RST_CLK_CPUclkPrescaler (RST_CLK_CPUclkDIV1);

  // Использование процессором сигнала CPU_C3
  // (HCLK = CPU_C3)
  RST_CLK_CPUclkSelection (RST_CLK_CPUclkCPU_C3);
}


