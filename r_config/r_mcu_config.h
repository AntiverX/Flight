/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2016 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * File Name    : r_mcu_config_reference.h
 * Description  : Defines the FLASH MCU configuration when not using the FIT BSP.
 ***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*           22.07.2016 1.00    First Release
***********************************************************************************************************************/
#ifndef R_MCU_CONFIG_HEADER_FILE
#define R_MCU_CONFIG_HEADER_FILE

#include "r_flash_rx_config.h"
#define MCU_RX23T
#define MCU_RX23_ALL

/****************************************************************************************
 MCU Configuration when using r_bsp:
 Configuration options are set in r_bsp_config.h
 Do not change any equates in this file.
****************************************************************************************/

#if (FLASH_CFG_USE_FIT_BSP == 1)
#include "platform.h"

#define MCU_CFG_PART_MEMORY_SIZE    BSP_CFG_MCU_PART_MEMORY_SIZE
#define MCU_ROM_SIZE_BYTES          BSP_ROM_SIZE_BYTES
#define MCU_RAM_SIZE_BYTES          BSP_RAM_SIZE_BYTES
#define MCU_DATA_FLASH_SIZE_BYTES   BSP_DATA_FLASH_SIZE_BYTES

#define MCU_CFG_ICLK_HZ             BSP_ICLK_HZ
#define MCU_CFG_FCLK_HZ             BSP_FCLK_HZ

#else
#include <stdint.h>     // typedefs
#include <stdbool.h>
#include <stddef.h>     // NULL
#include <machine.h>    // nop(), xchg()
#include "iodefine.h"

/****************************************************************************************
 MCU Configuration when not using r_bsp:
 Configuration options are set here
 Change the values of the following defines as needed.
 DON'T FORGET TO SELECT CLOCK (if LOCO not desired) AND
 ENABLE GLOBAL INTERRUPTS IN YOUR MAIN APPLICATION (normally done by BSP).
****************************************************************************************/

#define MCU_CFG_ICLK_HZ             (40000000)
#define MCU_CFG_FCLK_HZ             (20000000)

/* ROM, RAM, and Data Flash Capacity. 
   Character(s) = Value for macro = ROM Size/Ram Size
   5            = 0x5             = 128KB/10KB
   3            = 0x3             =  64KB/10KB
*/
#define MCU_CFG_PART_MEMORY_SIZE    (0x5)


/****************************************************************************************
 Do not change the following values.
****************************************************************************************/

#if MCU_CFG_PART_MEMORY_SIZE == 0x5
    #define MCU_ROM_SIZE_BYTES              (131072)
    #define MCU_RAM_SIZE_BYTES              (10240)
#elif MCU_CFG_PART_MEMORY_SIZE == 0x3
    #define MCU_ROM_SIZE_BYTES              (65536)
    #define MCU_RAM_SIZE_BYTES              (10240)
#else
    #error "ERROR - MCU_CFG_PART_MEMORY_SIZE - Unknown memory size chosen in r_mcu_config.h"
#endif


/* FIT equates */
#define FIT_NO_PTR      ((void *)0x10000000)    // Reserved space on RX
#define FIT_NO_FUNC     ((void *)0x10000000)    // Reserved space on RX

/* RSKRX23T equates for reference. Change to #if 1 if want to use. */
#if 0
#define LED_ON          (0)
#define LED_OFF         (1)
#define LED0            PORTA.PODR.BIT.B3
#define LED1            PORT7.PODR.BIT.B1
#define LED2            PORT7.PODR.BIT.B2
#define LED3            PORT7.PODR.BIT.B3
#define SW1             PORTD.PIDR.BIT.B6
#define SW2             PORT0.PIDR.BIT.B0
#define SW3             PORTA.PIDR.BIT.B4
#endif

#endif /* R_BSP Module used/not used */
#endif /* R_MCU_CONFIG_HEADER_FILE */
