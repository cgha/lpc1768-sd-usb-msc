#ifdef __USE_CMSIS
#include "LPC11Uxx.h"
#endif

#include "lpc_types.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_clkpwr.h"

#include "ssp.h"
#include "gpio.h"
#include "config.h"

#define inline __inline
enum speed_setting { INTERFACE_SLOW, INTERFACE_FAST };

static void spi_set_speed( enum speed_setting speed )
{
	if ( speed == INTERFACE_SLOW ) {
		setSSPclock(LPC_SSP0, 400000);
	} else {
		setSSPclock(LPC_SSP0, 25000000);
	}
}

void sspClockFast()
{
	spi_set_speed(INTERFACE_FAST);
}

void sspClockSlow()
{
	spi_set_speed(INTERFACE_SLOW);
}

static inline void select_card()
{
	// SSEL1 P0.6 low
	GPIO_ClearValue(CFG_CSPORT, (1 << CFG_CSPIN)); // LPC_GPIO0->FIOCLR = (1<<6);
}

static inline void de_select_card()
{
	// SSEL1 high
	GPIO_SetValue(CFG_CSPORT, (1 << CFG_CSPIN)); // LPC_GPIO0->FIOSET = (1<<6);
}

void sspInit(void)
{
	PINSEL_CFG_Type PinCfg;
	SSP_CFG_Type SSP_ConfigStruct;

	// SSEL1 P0.6 as GPIO, pull-up mounted but driven push/pull here
	PinCfg.Funcnum   = PINSEL_FUNC_0;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode   = PINSEL_PINMODE_PULLUP;
	PinCfg.Pinnum    = CFG_CSPIN;
	PinCfg.Portnum   = CFG_CSPORT;
	de_select_card();
	GPIO_SetDir(CFG_CSPORT, (1 << CFG_CSPIN), 1);
	PINSEL_ConfigPin(&PinCfg);
	// SCK P0.7 alternate function 0b11
	PinCfg.Funcnum   = PINSEL_FUNC_3;
	PinCfg.Pinnum    = 20;
	PINSEL_ConfigPin(&PinCfg);
	// MISO P0.8
	PinCfg.Pinnum    = 23;
	PINSEL_ConfigPin(&PinCfg);
	// MOSI P0.9
	PinCfg.Pinnum    = 24;
	PINSEL_ConfigPin(&PinCfg);

	SSP_ConfigStructInit(&SSP_ConfigStruct);
	SSP_Init(LPC_SSP0, &SSP_ConfigStruct);

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_SSP0, CLKPWR_PCLKSEL_CCLK_DIV_2);

	SSP_Cmd(LPC_SSP0, ENABLE);

	/* wait for busy gone */
	while( LPC_SSP0->SR & SSP_SR_BSY ) { ; }

	/* drain SPI RX FIFO */
	while( LPC_SSP0->SR & SSP_SR_RNE ) {
		volatile uint32_t dummy = LPC_SSP0->DR;
		(void)dummy;
	}

//#if ( SPI_SD_ACCESS_MODE == SPI_SD_USE_DMA )
//	GPDMA_Init();
//	LPC_GPDMA->DMACConfig = 0x01;
//#endif
}


/**************************************************************************/
/*!
    @brief Sends a block of data using SSP0

    @param[in]  buf
                Pointer to the data buffer
    @param[in]  length
                Block length of the data buffer
*/
/**************************************************************************/
void sspSend (uint8_t *buf, uint32_t length)
{
  uint32_t i;
  uint8_t Dummy = Dummy;

  for (i = 0; i < length; i++)
  {
    /* Move on only if NOT busy and TX FIFO not full. */
    while ((LPC_SSP0->SR & (SSP_SR_TNF_NOTFULL | SSP_SR_BSY_BUSY)) != SSP_SR_TNF_NOTFULL);
    LPC_SSP0->DR = *buf;
    buf++;

    while ( (LPC_SSP0->SR & (SSP_SR_BSY_BUSY|SSP_SR_RNE_NOTEMPTY)) != SSP_SR_RNE_NOTEMPTY );
    /* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
    on MISO. Otherwise, when sspReceive is called, previous data byte
    is left in the FIFO. */
    Dummy = LPC_SSP0->DR;
  }

  return;
}

/**************************************************************************/
/*!
    @brief Receives a block of data using SSP0

    @param[in]  buf
                Pointer to the data buffer
    @param[in]  length
                Block length of the data buffer
*/
/**************************************************************************/
void sspReceive(uint8_t *buf, uint32_t length)
{
  uint32_t i;

  for ( i = 0; i < length; i++ )
  {
    /* As long as the receive FIFO is not empty, data can be received. */
	  LPC_SSP0->DR = 0xFF;

    /* Wait until the Busy bit is cleared */
    while ( (LPC_SSP0->SR & (SSP_SR_BSY_BUSY|SSP_SR_RNE_NOTEMPTY)) != SSP_SR_RNE_NOTEMPTY );

    *buf = LPC_SSP0->DR;
    buf++;
  }

  return;
}
