#ifndef _BRD_DEF_VE92
#define _BRD_DEF_VE92


#ifdef USE_BOARD_VC_1

//  --------------  Buttons Definition  ------------
    #define BRD_BTN_PORT_UP         MDR_PORTC    // UP
    #define BRD_BTN_PIN_UP          PORT_Pin_3
	
    #define BRD_BTN_PORT_RIGHT      MDR_PORTC    // Right
    #define BRD_BTN_PIN_RIGHT       PORT_Pin_6
	
    #define BRD_BTN_PORT_DOWN       MDR_PORTC    // Down
    #define BRD_BTN_PIN_DOWN        PORT_Pin_4
	
    #define BRD_BTN_PORT_LEFT       MDR_PORTC    // Left
    #define BRD_BTN_PIN_LEFT        PORT_Pin_5

    #define BRD_BTN_PORT_SEL        MDR_PORTC    // Select
    #define BRD_BTN_PIN_SEL         PORT_Pin_0

    // for Initialization
    #define BRD_BTNs_PORT_CLK       RST_CLK_PCLK_PORTC

    #define BRD_BTNs_PORT_MASK         MDR_PORTC
    #define BRD_BTNs_PIN_MASK          (PORT_Pin_0 | PORT_Pin_3 | PORT_Pin_4 | PORT_Pin_5 | PORT_Pin_6)
    
    // Active buttons Level
    #define BRD_BTNs_PUSH_TO_GND


//  ----------    LEDs Definition -------------
    #define BRD_LED_1 	            PORT_Pin_15    // VD3
    #define BRD_LED_2 	            PORT_Pin_14    // VD4
    #define BRD_LED_3 	            PORT_Pin_13    // VD5
    #define BRD_LED_4 	            PORT_Pin_12    // VD6
    #define BRD_LED_5 	            PORT_Pin_11    // VD7

    #define BRD_LED_PORT_CLK       RST_CLK_PCLK_PORTB
    #define BRD_LED_PORT           MDR_PORTB
    #define BRD_LED_Pins          (BRD_LED_1 | BRD_LED_2 | BRD_LED_3 | BRD_LED_4 | BRD_LED_5)

//  ---------------  ADC Definition ---------------- 
    #define BRD_ADC_7_PIN           PORT_Pin_7
    #define BRD_ADC_7_PORT          MDR_PORTD
    #define BRD_ADC_7_CLOCK         RST_CLK_PCLK_PORTD

//  ----------    DMA Definition -------------
    #define BRD_DMA_CLOCK_SELECT  (RST_CLK_PCLK_SSP1 | RST_CLK_PCLK_SSP2 | RST_CLK_PCLK_SSP3 | RST_CLK_PCLK_SSP4 | RST_CLK_PCLK_DMA)



#else
   Please, select board in brdSelect.h!

#endif 


#endif // _BRD_DEF_VE92
