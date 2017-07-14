/**********************************************************************************************************************
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
* Copyright (C) 2015 Renesas Electronics Corporation. All rights reserved.
**********************************************************************************************************************/
/**********************************************************************************************************************
* File Name    : r_flash_type1_if.h
* Description  : This file contains the flash API functions for the RX111.
*
**********************************************************************************************************************/
/**********************************************************************************************************************
* History      : DD.MM.YYYY Version Description
*                02.18.2014 1.10    Support for RX110, RX111, RX113
*                12.01.2015 1.20    Updated for RX231
*                12.08.2016 2.00    Modified for BSPless flash.
*                17.11.2016 2.10    Added flash_stop() declaration.
*                                   Added FLASH_FREQ_xx and FLASH_FCU_INT_xxx #defines
**********************************************************************************************************************/

#ifndef R_FLASH_TYPE1_IF_HEADER_FILE
#define R_FLASH_TYPE1_IF_HEADER_FILE

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Fixed width integer support. */
#include <stdint.h>
/* bool support */
#include <stdbool.h>

#include "r_flash_rx.h"          /* Include this here or flash_err_t doesn't get resolved */

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define FLASH_FREQ_LO           (1000000)
#define FLASH_FREQ_HI           (32000000)
#define FLASH_FCU_INT_ENABLE    // no FCU
#define FLASH_FCU_INT_DISABLE   // no FCU

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/


/***********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global functions (to be accessed by other files)
***********************************************************************************************************************/
flash_err_t flash_api_open(void);
flash_err_t flash_api_control(flash_cmd_t cmd,  void  *p_cfg);
flash_err_t flash_api_write(uint32_t src_address, uint32_t dest_address, uint32_t num_bytes);
flash_err_t flash_api_erase(flash_block_address_t block_start_address, uint32_t num_blocks);
flash_err_t flash_api_blankcheck(uint32_t address, uint32_t num_bytes, flash_res_t *result);
extern void (*flash_ready_isr_handler)(void *);          /* Function pointer for Flash Ready ISR */
void r_flash_delay_us (unsigned long us, unsigned long khz);
void flash_stop(void);

#endif /* R_FLASH_TYPE1_IF_HEADER_FILE */

/* end of file */
