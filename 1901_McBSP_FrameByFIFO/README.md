# 1901_McBSP_FrameByFIFO

Пример реализован для микроконтроллера 1901ВЦ1Т, блок McBSP.

Пример показывает отсутствие сигнала кадровой синхронизации при настройке формирования этого сигнала по поступлению данных из FIFO в передатчик.

Регистр SRGRH бит FSGM = 1.

Отсутсвие сигнала наблюдается осциллографом при активации макрокопределения 

   #define FRAME_FAULT_ACTIVATE

## Код активирующий ошибку
    // SRGRH(7) (Регистр управления генераторами битовой и кадровой синхронизации)
    //		GSYNC[15]	- 0 - FreeRun, 1 - External FrameSync	
    //		FSGM[14]	- FrameSync: 0 - Gen, 1 - FIFO
    //		CLKSM[13]	- 0 - Gen from External, 1 - by Clock
    //		CLKSP[12] 	- 0 - Frame & Bit Sync by Front, 1 - by Fall
    //		FPER[11..0]	- FrameSync Period (Cnt - 1)

        MDR_MCBSP2->SPSA  = 0x0007;
    #ifndef FRAME_FAULT_ACTIVATE	 
        MDR_MCBSP2->SPCR = 0x200F0000;           // FrameSync by Gen
    #else
        MDR_MCBSP2->SPCR = 0x600F0000;           // FrameSync by FIFO
    #endif