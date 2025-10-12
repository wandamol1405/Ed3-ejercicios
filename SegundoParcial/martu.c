#include "LPC17xx.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"

void config_port();
void config_eint();
void config_dac();
void config_dma();
void funcion1();
void funcion2();
void funcion3();

int main(){
	SystemInit(); //cloack configurado a 100M
	config_port();
	config_eint();
	while(1){
		
	}
	return 0;
}

void config_port(){
	PINSEL_CFG_Type config = {0};
	
	config.Portnum = PINSEL_PORT_2;
	config.Pinnum = PINSEL_PIN_10;
	config.Funcnum = PINSEL_FUNC_1;
	config.Pinmode = PINSEL_PINMODE_PULLDOWN;
	
	PINSEL_ConfigPin(&config);
	
	config.Portnum = PINSEL_PORT_0;
	config.Pinnum = PINSEL_PIN_26;
	config.Funcnum = PINSEL_FUNC_2;
	config.Pinmode = PINSEL_PINMODE_TRISTATE;
}

void config_eint(){
	EXTI_InitTypeDef config = {0};
	
	config.EXTI_Line = EXTI_EINT0;
	config.EXTI_Mode = EXTI_MODE_LEVEL_SENSITIVE;
	config.EXTI_polarity = EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
	
	EXTI_Cofig(&config);
	NVIC_EnableIRQ(EINT0_IRQn);
}

void config_dac(){
	DAC_CONVERTER_CFG_Type config = {0};
	
	config.CNT_ENA = 1;
	config.DMA_ENA = 1;
	
	DAC_ConfigDAConverterControl(LPC_DAC, &config);
	DAC_SetBias(LPC_DAC, 1); //set BIAS para menor consumo

}

void config_dma(){
	
	GPDMA_LLI_Type lli = {0};
	
	lli.SrcAddr = DIRECCION_BLOQUE_0;
	lli.DstAddr = &LPC_DAC->DACR;
	lli.NextLLI = &lli;
	lli.Control = (1024)| (0b01010010 << 18);
	
	GPDMA_Channel_CFG_Type config = {0};
	
	config.ChannelNum = GPDMA_CHANNEL_0;
	config.SrcMemAddr = DIRECCION_BLOQUE_0;
	config.DstConn = GPDMA_CONN_DAC;
	config.TransferSize = 4000;
	config.TransferType = GPDMA_TRANSFERTYPE_M2P;
	config.DMALLI = &lli;
	
	GPDMA_Init();
	GPDMA_Setup(&config);
	
	lli.SrcAddr = DIRECCION_BLOQUE_1;
	config.ChannelNum = GPDMA_CHANNEL_1;
	config.SrcMemAddr = DIRECCION_BLOQUE_1;
	GPDMA_Setup(&config);
	
	lli.SrcAddr = DIRECCION_BLOQUE_;
	config.ChannelNum = GPDMA_CHANNEL_2;
	config.SrcMemAddr = DIRECCION_BLOQUE_0;
	GPDMA_Setup(&config);
}