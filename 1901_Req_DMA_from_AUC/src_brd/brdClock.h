#ifndef _BRD_CLOCK_H
#define _BRD_CLOCK_H

#include <stdint.h>

#include "brdDef.h"

#ifdef USE_BOARD_VE_8 
  #include  <mdr32f8_clkctrl.h>
  #include  "spec.h"

  #include "brdVE8_toVEx.h"
  
#else  
  #include <MDR32F9Qx_port.h>
  #include <MDR32F9Qx_rst_clk.h>
  #include <MDR32F9Qx_eeprom.h>  
#endif

extern uint32_t BRD_CPU_CLK;

void BRD_Clock_Init_HSE_PLL(uint32_t PLL_Mul_sub1);

#ifndef USE_MDR1986VE3
  void BRD_Clock_Init_HSE_dir(void);
#endif

#ifdef USE_MDR1901VC1T
  void BRD_ClockDSP_Init_HSE_PLL(uint32_t PLL_Mul_sub1);
  
  //  При включении тактирования блока DSP, к DMA RISC ядра висит запрос на обработку от FIFO ЦАПа аудиокодека.
  //  Чтобы погасить запрос, функция заполняет это FIFO данными.
  void BRD_ClockDSP_Init_HSE_PLL_fixDMAIRQ(uint32_t PLL_Mul_sub1);
#endif


#endif  //_BRD_CLOCK_H
