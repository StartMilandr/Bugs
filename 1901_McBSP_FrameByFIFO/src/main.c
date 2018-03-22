#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>

#include "McBSP.h"

void Delay(int waitTicks);

int main()
{
	Init_CPU_BSP2();
	BSP2_PortInit();
	//BSP2_InitRX();
	BSP2_InitTX();
	
  while (1)
  {
		BSP2_send(0x7F01);
		Delay(1500);			
  }      
}

void Delay(int waitTicks)
{
  int i;
  for (i = 0; i < waitTicks; i++)
  {
    __NOP();
  }	
}
