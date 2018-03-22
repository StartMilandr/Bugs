#ifndef _MCBSP_H
#define _MCBSP_H

void Init_CPU_BSP2(void);
void BSP2_PortInit(void);
//void BSP2_InitRX(void);
void BSP2_InitTX(void);
void BSP2_send(uint32_t data);
void BSP2_WaitXRDY(void);
void Log_BSP2_REGs(void);

#endif //_MCBSP_H
