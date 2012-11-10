/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The USB Device application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2012-3-17
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "lpc_types.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "mscuser.h"

#include "lpc17xx_gpio.h"
#include "usbhost_lpc17xx.h"
#include "debug_frmwrk.h"
#include <stdio.h>

#include "memory.h"
//#ifdef CFG_SDCARD
#include "diskio.h"
//#endif

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#define KEY1_PIN	((GPIO_ReadValue(2)&(1<<11)))

extern uint8_t Memory[MSC_MemorySize];         /* MSC Memory in RAM */

void delay(uint32_t t)
{
	while(t--);
}

typedef enum{
   USB_DeviceMode,
   USB_HostMode,
}USB_MODE_T;
volatile uint8_t UsbMode = USB_HostMode; //0=deviceMode 1=hostMode

volatile uint32_t mmcTicks = 0;
volatile uint32_t msTicks;                             /* counts 1ms timeTicks */

/*----------------------------------------------------------------------------
  SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void)
{
  // Increment systick counter for Delay();
  msTicks++;

  // Increment SD card timer counter
  mmcTicks++;
  // Run MMC timer every 10 ms (mmc.c)
  if (mmcTicks == 10)
  {
    mmcTicks = 0;
    disk_timerproc();
  }
}

/*----------------------------------------------------------------------------
  Tries to initialise the SD card using SSP0.  The code will hang if
  a card is not inserted (STA_NODISK) or if the init failed (STA_NOINIT)
 *----------------------------------------------------------------------------*/
void SDInit(void)
{
	// Initialise SD Card
	DSTATUS stat;
	stat = disk_initialize(0);
	if (stat & STA_NODISK)
	{
		// No SD Card Present ... hold
		while(1);
	}
	if (stat & STA_NOINIT)
	{
		// SD Init Failed ... hold
		while(1);
	}
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int main (void) {
	uint32_t n;
	uint32_t ret;

	SysTick_Config(SystemCoreClock / 1000);
	#ifdef CFG_SDCARD
  	/* Setup SD Card/FATFS */
  	SDInit();
	#endif
	debug_frmwrk_init();
	printf("\r\n hello");
	//for (n = 0; n < MSC_ImageSize; n++) {     /* Copy Initial Disk Image */
	//	Memory[n] = DiskImage[n];             /* from Flash to RAM */
	//}
	
	if(!KEY1_PIN)
	{
		UsbMode = 0;
		while(!KEY1_PIN);
		delay(100000);
		while(!KEY1_PIN);
		
	}
	if(UsbMode == 0){
		USB_Init();                               /* USB Initialization */
		USB_Connect(TRUE);                        /* USB Connect */
	}
	else{
		
	
	    Host_Init();               // Initialize the lpc17xx host controller                                    
	
		if( Host_EnumDev() == 0 )	/* Enumerate the device connected                                            */
	    {
		  printf("-- USB device detected OK \r\n");
	    }
	    else
	    {
	      printf("-- Please connect a USB device \r\n");
	      while( Host_EnumDev() == 0 );
	      printf("-- USB device connection detected \r\n");
		  Delay(0x00ffff);
	    }
	}

	while (1){
		if(!KEY1_PIN)
		{
			ret = Host_CtrlSend(0x40, 0x50, 0, 0, 0, NULL);
			printf("\r\n sent switch cmd,ret= %x ",ret);
			delay(100);
			NVIC_SystemReset();
		}
	}
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
	/* wait for current transmission complete - THR must be empty */
	while (UART_CheckBusy(DEBUG_UART_PORT) == SET);
	
	UARTPutChar(DEBUG_UART_PORT, ch);
	
	return ch;
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
