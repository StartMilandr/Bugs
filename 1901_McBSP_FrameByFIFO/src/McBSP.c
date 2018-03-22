#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_rst_clk.h"
#include "MDR1901VC1T.h"

#include "McBSP.h"

//  ----------    Defines -----------
//#define FRAME_FAULT_ACTIVATE  // Ошибка фреймовой синхронизации от FIFO

#define DATA_MSB                // Выдача старшим битом вперед, закомментарить выдачи младшим битом вперед


void BSP2_PortInit(void)
{	
  // Заводим структуру конфигурации вывода(-ов) порта GPIO
  PORT_InitTypeDef GPIOInitStruct;
	
  //  Включаем тактирование порта C
  RST_CLK_PCLKcmd (RST_CLK_PCLK_PORTC, ENABLE);
	
  //  Инициализируем структуру конфигурации вывода(-ов) порта значениями по умолчанию
  PORT_StructInit(&GPIOInitStruct);
  
  //  Изменяем значения по умолчанию на необходимые нам настройки
  GPIOInitStruct.PORT_Pin        = PORT_Pin_8 | PORT_Pin_9 | PORT_Pin_10 | PORT_Pin_11;
  GPIOInitStruct.PORT_OE         = PORT_OE_OUT;
  GPIOInitStruct.PORT_SPEED      = PORT_SPEED_FAST;
  GPIOInitStruct.PORT_MODE       = PORT_MODE_DIGITAL;
	GPIOInitStruct.PORT_FUNC       = PORT_FUNC_OVERRID;
  
  //  Применяем заполненную нами структуру для PORTC.
  PORT_Init(MDR_PORTC, &GPIOInitStruct);
}	

void Init_CPU_BSP2(void)
{
  MDR_RST_CLK->HS_CONTROL = RST_CLK_HS_CONTROL_HSE_ON;
  while((MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_HSE_RDY) != RST_CLK_CLOCK_STATUS_HSE_RDY);

  MDR_RST_CLK->CPU_CLOCK = 2 << RST_CLK_CPU_CLOCK_CPU_C1_SEL_Pos;   //CPU_C1 = HSE
  MDR_RST_CLK->DSP_CLOCK = 2 << RST_CLK_DSP_CLOCK_DSP_C1_SEL_Pos;   //DSP_C1_SEL = HSE

  MDR_RST_CLK->PLL_CONTROL =  (1 << RST_CLK_PLL_CONTROL_PLL_CPU_MUL_Pos)
                         | RST_CLK_PLL_CONTROL_PLL_CPU_ON
                         | RST_CLK_PLL_CONTROL_PLL_DSP_ON
                         |(1 << RST_CLK_PLL_CONTROL_PLL_DSP_MUL_Pos); 

  while((MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_PLL_CPU_RDY) != RST_CLK_CLOCK_STATUS_PLL_CPU_RDY);

  MDR_RST_CLK->PER_CLOCK |= 1 << 3;    //включение тактирования EEPROM_CNTRL
  MDR_EEPROM->CMD = 2 << 3;            //Delay = 2, рабочая частота до 75 МГц
  MDR_RST_CLK->PER_CLOCK &= ~(1 << 3);    //выключение тактирования EEPROM_CNTRL

  MDR_RST_CLK->CPU_CLOCK |= (RST_CLK_CPU_CLOCK_CPU_C2_SEL)|(1 << RST_CLK_CPU_CLOCK_HCLK_SEL_Pos);     //CPU Clock = 73.728MHz

  while((MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_PLL_DSP_RDY) != RST_CLK_CLOCK_STATUS_PLL_DSP_RDY);
  MDR_RST_CLK->DSP_CLOCK |= RST_CLK_DSP_CLOCK_DSP_C2_SEL | RST_CLK_DSP_CLOCK_DSP_CLK_EN;      //тактирование DSP включено, источник тактирования DSP_PLL

        
	//	BSP Init
	MDR_RST_CLK->DSP_CONTROL_STATUS = 0x0012;  	// Enable  all DSP parts
	MDR_DSP_CORE->CLKCMD |= 0xF<<7;          		// Enable MCBSP2 and MCBSP3 clock
}	

void BSP2_InitTX()
{
//	RESET All
  MDR_MCBSP2->SPSA  = 0x0000;
  MDR_MCBSP2->SPCR = 0x00000000;

	
// XCRL(4) (Регистр управления передатчиком)
//	FIG[15]					- 0 - Start by FrameSync, 1 - Ignore All except for firts FrameSync
//	FLEN_P0[14..8]	- Words in Phrase (Cnt - 1)
//	MSB1st[7]				- 0 - LSB, 1 - MSB
//	DELAY[6..5]			- Delay after FrameSync
//	WLEN_P0[4..0]		- Bits in Word (Cnt - 1)
   MDR_MCBSP2->SPSA  = 0x0004;
#ifdef DATA_MSB  
   MDR_MCBSP2->SPCR = 0x008F;  //Фраза 1 = 1 слово 16 бит
#else
   MDR_MCBSP2->SPCR = 0x000F;  
#endif

	
// XCRH(5) (Регистр управления передатчиком)
//		MPHASE[15] 		 - 0 - Single phrase, - Multy phrase
//		FLEN_P1[14..8] - Words in prase 2
//		CODEC[7..5] 	 - 0 - OFF
//		WLEN_P1[4..0]  - Bits in Word (Cnt - 1), phrase 2
   MDR_MCBSP2->SPSA  = 0x0005;
   MDR_MCBSP2->SPCR = 0x00000000; //Фраза 2 = 0 слов

// SRGRL(6) (Регистр управления генераторами битовой и кадровой синхронизации)
//		FWID[15..8]	  - Framesync Duration (Bits - 1)
//		CLKGDV[7..0]	- Clock Div (Bits - 1)
   MDR_MCBSP2->SPSA  = 0x0006;
   MDR_MCBSP2->SPCR = 0x007F;    // Frame Sync = 1 Clock, Freq = CPU_Clock / 128
	 //MDR_MCBSP2->SPCR = 0x0000;  // Frame Sync = 1 Clock, Freq = CPU_Clock
	
// SRGRH(7) (Регистр управления генераторами битовой и кадровой синхронизации)
//		GSYNC[15] 	- 0 - FreeRun, 1 - External FrameSync	
//		FSGM[14]  	- FrameSync: 0 - Gen, 1 - FIFO
//		CLKSM[13] 	- 0 - Gen from External, 1 - by Clock
//		CLKSP[12]   - 0 - Frame & Bit Sync by Front, 1 - by Fall
//		FPER[11..0] - FrameSync Period (Cnt - 1)
   MDR_MCBSP2->SPSA  = 0x0007;
#ifndef FRAME_FAULT_ACTIVATE	 
   MDR_MCBSP2->SPCR = 0x200F0000;   // FrameSync by Gen
#else
	 MDR_MCBSP2->SPCR = 0x600F0000;	 // FrameSync by FIFO
#endif	
   

// PCRL(E) (Регистр управления выводами)
//		XIOEN[13] - 0 - pin as GPIO, 1 - PAD
//		RIOEN[12] - 0 - pin as GPIO, 1 - PAD
//		FSXM[11]  - 0 - Ext FrameSync, 1 - Internal
//		FSRM[10]  - 0 - Ext FrameSync, 1 - Internal
//		CLKXM[9]  - 0 - Ext BitSync, 1 - Internal
//		CLKRM[8]  - 0 - Ext BitSync, 1 - Internal
//		RESERV[7] 
//		CLKS_ST[6]- RO: CLKS_ST
//		DX_ST[5]  - RO: DX_ST
//		DR_XT[4]  - RO: DR_ST
//		FSXP[3]  	- 0 - Active Frame High
//		FSRP[2]  	- 0 - Active Frame High
//		CLKXP[1]  - 0 - Active BitSync - Front
//		CLKRP[1]  - 0 - Active BitSync - Front

   MDR_MCBSP2->SPSA  = 0x000E; 
   MDR_MCBSP2->SPCR = 0x2F00;	

// MCRL(8) (Регистр управления многоканальными режимами приема и передачи)
//    BLK_EN[15..8] - 16 channels per bit, 0 - active, 1 - blocked
//    CUR_BLK[7..5] - номер текущего блока в кадре
//    MCM[1..0]     - 00 - All channels ON
   MDR_MCBSP2->SPSA  = 0x0008;    
	 MDR_MCBSP2->SPCR = 0x00000000;

//  MCRH(9) (Регистр управления многоканальными режимами приема и передачи)
//    BLK_EN[15..8] - 16 channels per bit, 0 - active, 1 - blocked
//    CUR_BLK[7..5] - номер текущего блока в кадре
//    MCM[1..0]     - 00 - All channels ON
   MDR_MCBSP2->SPSA  = 0x0009;	
   MDR_MCBSP2->SPCR = 0x00000000;

// XCERL(4) (Регистр маски передатчика)
   MDR_MCBSP2->SPSA  = 0x000A;
   MDR_MCBSP2->SPCR = 0x0000;
// XCERH(5) (Регистр маски передатчика)
   MDR_MCBSP2->SPSA  = 0x000B;
   MDR_MCBSP2->SPCR = 0;

//	SPCR_L(0): (Регистр общего управления)
//    DXENT[15]      - 0 - DX On, 1 - DX OFF
//    LW_ACC[14]     - Готовность слова: 0 - по любому WR/RD, 1 - все байты
//    JBOUND[13..12] - Выравнивание байт: b00 - 1 байт, b01 - 2 байта, b1х - 4 байта
//    XJUST[11..10]  - Выравнивание данных: b0x - по правой границе
//    RJUST[9..8]    - b0x - по правой границе
//    DLB[7]         - 0 = КЗ выкл
//    ABIS[6]        - 0 = A-bis Off
//    CLKSTP[5..4]   - b0x - Stop Off, b10 - Stop without delay, b11 - Stop with Delay of half period
//    nXRST[3]       - 0 - TX_OFF, 1 - TX_ON
//    nRRST[2]       - 0 - RX_OFF, 1 - RX_ON
//    nFGRTS[1]      - 0 - Frame Sync Gen Off, 1 - Frame Gen On
//    nFGRTS[0]      - 0 - Bit Sync Gen Off, 1 - On

   MDR_MCBSP2->SPSA  = 0x0000;
   MDR_MCBSP2->SPCR = 0x100B;

//	SPCR_H(1): (Регистр общего управления)
//    FREE[15]       - FreeRun OFF  = 0
//    XINTM[14..12]  - XRDY = 0
//    XEVNT1[11..10] - XRDY = 0
//    XEVNT0[9..8]   - XRDY = 0
//    XEVNT0[7]      - HALT_OFF = 0
//    RINTM[6..4]    - RRDY = 0
//    REVNT1[3..2]   - RRDY = 0
//    REVNT0[1..0]   - RRDY = 0

   MDR_MCBSP2->SPSA  = 0x0001;
   MDR_MCBSP2->SPCR = 0x00000000;
}


//-------------------------------------------------------------------------------------------------
uint32_t McBSP_GetReg(MDR_MCBSP_TypeDef *port, uint32_t num)
{
	while (port->SPSA != num)
		port->SPSA = num;
	return	port->SPCR;
}
//-------------------------------------------------------------------------------------------------
void McBSP_SetReg(MDR_MCBSP_TypeDef *port, uint32_t num, uint32_t value)
{
	while (port->SPSA != num)
		port->SPSA = num;
	port->SPCR = value;
}	

void BSP2_WaitXRDY(void)
{
	volatile uint32_t value;
	do
	{	
		value = McBSP_GetReg(MDR_MCBSP2, 0xF);
	}	
	while (!(value & (1 << 8)));
}	

void BSP2_send(uint32_t data)
{
	BSP2_WaitXRDY();
	
	MDR_MCBSP2->DXR = data;

//    Альтернативный вариант	
//*(uint16_t*)(0x30000058)=0x0f;       //BSP2 SPSRH addr
//while (!((*(uint16_t*)(0x3000005C))&0x0100) );
//*(uint16_t*)(0x30000056)=0x0000;       //BSP2 addr
//*(uint16_t*)(0x30000054)=data;       //BSP2 addr
}
	
uint16_t LogBuff[16];

void Log_BSP2_REGs(void)
{
	int i;
	for (i=0; i<16; i++)
	{
		MDR_MCBSP2->SPSA  = i;
    LogBuff[i] = McBSP_GetReg(MDR_MCBSP2, i);
	}
}


//void BSP2_InitRX(uint8_t  param)
//{
//   //Configuration McBSP1 for transmitt data
//   MDR_MCBSP2->SPSA = 0x0000;
//   MDR_MCBSP2->SPCR = 0x6000;

//   if(param == 0)
//   {
//   //config RCRL (Регистр управления приемником)
//      MDR_MCBSP2->SPSA = 0x0002;
//      MDR_MCBSP2->SPCR = 0x009F;   //1 слово, 32 бита (I -> 16 бит, Q -> 16 бит)
//   //config RCRH (Регистр управления приемником)
//      MDR_MCBSP2->SPSA = 0x0003;
//      MDR_MCBSP2->SPCR = 0x0000;
//   }
//   else
//   {
//   //config RCRL (Регистр управления приемником)
//      MDR_MCBSP2->SPSA = 0x0002;
////      DSP_BSP1->SPCR = 0x01B7;   //2 слова, по 24 бита   (I -> 24 бита, Q -> 24 бита)
//      MDR_MCBSP2->SPCR = 0x0197;   //2 слова, по 24 бита   (I -> 24 бита, Q -> 24 бита)
//   //config RCRH (Регистр управления приемником)
//      MDR_MCBSP2->SPSA = 0x0003;
//      MDR_MCBSP2->SPCR = 0x0000;
//   }
////config SRGRL (Регистр управления генераторами битовой и кадровой синхронизации)
////config SRGRH (Регистр управления генераторами битовой и кадровой синхронизации)
//   MDR_MCBSP2->SPSA = 0x0007;
//   MDR_MCBSP2->SPCR = 0x80000000;   //синхронизация от внешнего источника

////config PCRL (Регистр управления выводами)
//   MDR_MCBSP2->SPSA = 0x000E;
//   MDR_MCBSP2->SPCR = 0x1000;   //0x00001F00 - для внутреннего тактирования

////config MCRL (Регистр управления многоканальными режимами приема и передачи)
//   MDR_MCBSP2->SPSA = 0x0008;
//   MDR_MCBSP2->SPCR = 0x00000000;
////config MCRH (Регистр управления многоканальными режимами приема и передачи)
//   MDR_MCBSP2->SPSA = 0x0009;
//   MDR_MCBSP2->SPCR = 0xFF000000;

////config RCERL (Регистр маски приемника)
//   MDR_MCBSP2->SPSA = 0x000C;
//   MDR_MCBSP2->SPCR = 0xFFFF;
////config RCERH (Регистр маски приемника)
//   MDR_MCBSP2->SPSA = 0x000D;
//   MDR_MCBSP2->SPCR = 0xFFFF0000;

////config SPCRL (Регистр общего управления)
//   MDR_MCBSP2->SPSA = 0x0000;
//   MDR_MCBSP2->SPCR = 0x6007;   //ПРМ вкл., слово - 4 байта.
////config SPCRH (Регистр общего управления)
//   MDR_MCBSP2->SPSA = 0x0001;
//   MDR_MCBSP2->SPCR = 0x80500000;       //прерывание - заполненный наполовину FIFO буфер приемника
////Configuration of McBSP complete   
//}


