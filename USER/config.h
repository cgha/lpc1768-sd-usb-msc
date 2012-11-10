/*
===============================================================================
 Name        : config.h
 Author      :
 Version     :
 Copyright   : Copyright (C)
 Description : Common config settings
===============================================================================
*/
#ifndef __CONFIG_H
#define __CONFIG_H

//#define CFG_MEMORY
#define CFG_SDCARD
#if !defined(CFG_MEMORY) && !defined(CFG_SDCARD)
#error "ERROR: No configuration defined"
#endif

#define BOARD_EA_REVB
//#define BOARD_NGX
#if !defined(BOARD_EA_REVB) && !defined(BOARD_NGX)
#error "ERROR: No board defined"
#endif



#define CFG_CSPORT (1)
#define CFG_CSPIN  (21)


// Card Detect Port/Pin for SD CARD
#define CFG_SDCARD_CDPORT (1)
#define CFG_SDCARD_CDPIN  (25)

// LED Port/Pin (for quick debugging)
#define CFG_LED_PORT (3)
#define CFG_LED_PIN (25)

#endif
