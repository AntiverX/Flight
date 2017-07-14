/********************************************************************************************************************
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
********************************************************************************************************************/
/*******************************************************************************************************************
* File Name : r_flash_type1.c
* Description  : This module implements the Flash API layer functions for the RX1xx and RX23x
********************************************************************************************************************/
/*******************************************************************************************************************
* History : DD.MM.YYYY Version Description
*           12.11.2014 1.10    Support for RX110, RX111, RX113
*                              Changed WAIT_MAX_ROM_WRITE to WAIT_MAX_ERASE_DF_1K in flash_api_erase().
*           19.12.2014 1.20    Replaced some equate names, modified for FLASH_TYPE_1, changed some wait_cnt settings.
*           12.01.2015 1.30    Updates for RX231
*           01.09.2015 1.40    Modified to use FLASH_NO_DATA_FLASH for RX23T and RX110.
*           13.11.2015 1.50    Added FLASH_CMD_CACHE_xxx for RX24T.
*           22.07.2016 2.00    Modified for BSPless flash.
*           17.11.2016 2.10    Added flash_stop() before issuing a FLASH_CMD_RESET.
*                              Added DF and CF block boundary check to flash_api_erase().
*                              Added CF block boundary check to FLASH_CMD_ACCESSWINDOW_SET.
*                              Added non-NULL argument check for FLASH_CMD_SWAPSTATE_SET and FLASH_CMD_SWAPSTATE_GET.
*                              Added valid SAS value check for FLASH_CMD_SWAPSTATE_SET.
*                              Added check for BUSY in flash_api_control() when in BGO mode.
*                              Added check in Open() for another operation in progress when in BGO mode.
********************************************************************************************************************/

/********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
********************************************************************************************************************/
/* Includes board and MCU related header files. */
#include "r_flash_rx_if.h"
#if (FLASH_TYPE == FLASH_TYPE_1)
#include "r_mcu_config.h"
#include <machine.h>

/* Private header file for this package. */
#include "r_flash_type1_if.h"
#include "r_flash_common.h"
#include "r_dataflash.h"
#include "r_codeflash.h"
#include "r_codeflash_extra.h"

/*********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#if (FLASH_CFG_PARAM_CHECKING_ENABLE == 1)
#define FLASH_RETURN_IF_PCFG_NULL   if ((pcfg == NULL) || (pcfg == FIT_NO_PTR))     \
                                    {                                               \
                                        return FLASH_ERR_NULL_PTR;                  \
                                    }

#define FLASH_RETURN_IF_ROM_LT_32K  if (MCU_ROM_SIZE_BYTES < 32768)                 \
                                    {                                               \
                                        return FLASH_ERR_FAILURE;                   \
                                    }

                                    // See FLASH_SAS_xxx #defines
#define FLASH_RETURN_IF_BAD_SAS     if ((*pSwapInfo == 1) || (*pSwapInfo > 4))      \
                                    {                                               \
                                        return FLASH_ERR_PARAM;                     \
                                    }
#else
#define FLASH_RETURN_IF_PCFG_NULL   // parameter checking disabled
#define FLASH_RETURN_IF_ROM_LT_32K  // parameter checking disabled
#define FLASH_RETURN_IF_BAD_SAS     // parameter checking disabled
#endif


/*********************************************************************************************************************
 Typedef definitions
 *********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
int32_t g_flash_lock;

#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1)) || (FLASH_CFG_DATA_FLASH_BGO == 1)
flash_int_cb_args_t g_flash_int_ready_cb_args;      /* Callback argument structure for flash READY interrupt */
#endif

/*********************************************************************************************************************
 External variables and functions
 *********************************************************************************************************************/
extern flash_states_t g_flash_state;

flash_err_t flash_lock_state (flash_states_t new_state);

/***********************************************************************************************************************
* Function Name: flash_api_open
* Description  : Function will initialize the flash for the RX1xx and RX23x.
* Arguments    : void
* Return Value : FLASH_SUCCESS -
*                    API initialized successfully.
*                FLASH_ERR_FREQUENCY -
*                    MCU_CFG_FCLK_HZ not between 1MHz and 32MHz
*                FLASH_ERR_BUSY -
*                    API has already been initialized and another operation is ongoing.
***********************************************************************************************************************/
flash_err_t flash_api_open(void)
{
#if (FLASH_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((MCU_CFG_FCLK_HZ > 32000000) || (MCU_CFG_FCLK_HZ < 1000000))
    {
        return FLASH_ERR_FREQUENCY;
    }
#endif

    /* Allow Initialization if not initialized or
     * if no operation is ongoing and re-initialization is desired */
    if (FLASH_SUCCESS != flash_lock_state(FLASH_INITIALIZATION))
    {
        /* API already initialized */
        return FLASH_ERR_BUSY;
    }


#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1)) || (FLASH_CFG_DATA_FLASH_BGO == 1)
    g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_INITIALIZED;

    /* Enable Interrupts*/
    flash_interrupt_config(false, FIT_NO_FUNC);
#endif

    /* Release state so other operations can be performed. */
    flash_release_state();

#ifndef FLASH_NO_DATA_FLASH
    /* Make E2 DataFlash Access enable */
     R_DF_Enable_DataFlashAccess();
#endif

    return FLASH_SUCCESS;
}


#if (FLASH_CFG_CODE_FLASH_ENABLE == 1)
#pragma section FRAM
#endif

/*****************************************************************************
* Function Name: flash_api_control
* Description  : This function executes RX1xx and RX23x specific flash control commands.
*
* Arguments    : flash_cmd_t cmd -
*                 command.
*                void *pcfg -
*                 Pointer to configuration. This argument can be NULL for
*                 commands that do not require a configuration.
*
*                Command                                | Argument
*                FLASH_CMD_RESET------------------------| NULL
*                FLASH_CMD_STATUS_GET-------------------| NULL
*                FLASH_CMD_ACCESSWINDOW_SET-------------| void (*pAccessInfo)(void *)
*                FLASH_CMD_ACCESSWINDOW_GET-------------| void (*pAccessInfo)(void *)
*                FLASH_CMD_SWAPFLAG_TOGGLE--------------| NULL
*                FLASH_CMD_SWAPFLAG_GET-----------------| void (uint8_t *)
*                FLASH_CMD_SWAPSTATE_GET----------------| void (uint8_t *)
*                FLASH_CMD_SWAPSTATE_SET----------------| void (uint8_t *)
*                FLASH_CMD_SET_BGO_CALLBACK-------------| void (flash_interrupt_config_t *)
*                FLASH_CMD_ROM_CACHE_ENABLE-------------| NULL
*                FLASH_CMD_ROM_CACHE_DISABLE------------| NULL
*                FLASH_CMD_ROM_CACHE_STATUS-------------| void (uint8_t *)
*
* Return Value : FLASH_SUCCESS -
*                    Operation completed successfully.
*                FLASH_ERR_FAILURE -
*                    Operation not available for this MCU
*                FLASH_ERR_ADRRESS -
*                    Address is an invalid Code/Data Flash block start address
*                FLASH_ERR_NULL_PTR -
*                    Pointer was NULL for a command that expects a configuration structure
*                FLASH_ERR_BUSY -
*                    Flash peripheral is busy with another operation or not initialized
******************************************************************************/
flash_err_t flash_api_control(flash_cmd_t cmd,  void  *pcfg)
{
    flash_err_t err = FLASH_SUCCESS;
#if (FLASH_CFG_CODE_FLASH_ENABLE == 1)
    uint8_t *pSwapInfo = pcfg;
    flash_access_window_config_t  *pAccessInfo = pcfg;
#if (FLASH_HAS_ROM_CACHE == 1)
    uint8_t *status = pcfg;
#endif
#endif
    /*If the command is to reset the Flash, then no attempt is made to grab the lock
     * before executing the reset since the assumption is that the RESET command
     * is used to terminate any existing operation. */
    if (cmd == FLASH_CMD_RESET)
    {
#if ((FLASH_TYPE == 1) && ((FLASH_CFG_CODE_FLASH_BGO == 1) || (FLASH_CFG_DATA_FLASH_BGO == 1)))
        /* cannot do reset if command in progress */
        if ((g_flash_state == FLASH_ERASING) || (g_flash_state == FLASH_BLANKCHECK))
        {
            flash_stop();   // can abort an outstanding erase or blankcheck command
        }

        while (g_flash_state != FLASH_READY)
            ;
#endif
        flash_release_state();
        flash_reset();           /* reset the flash circuit */
        return err;
    }

    if(g_flash_state != FLASH_READY)
    {
        /* There is an operation in progress. If we don't have some form of BGO enabled, then logic error in driver. */
#if (FLASH_CFG_DATA_FLASH_BGO == 0) && (FLASH_CFG_CODE_FLASH_BGO ==0)
       /* In blocking mode we will return busy if the flash state is busy no matter what */
        return FLASH_ERR_BUSY;  /* API not initialized or busy with another operation*/
#else
        /* We are in polling/BGO mode. We may have an operation running in the background. */
        /* If that's the case then we need to be able to call GetStatus() to kick it along and complete */
        /* the original started operation. Therefore in this case we need to allow a FLASH_CMD_STATUS_GET */
        /* even when the g_flash_stat is not ready. */
        if (cmd == FLASH_CMD_STATUS_GET)    // this is the only permissible cmd when another operation is in progress
        {
            /* Use a software lock to insure no one sneaks in. */
            if (flash_softwareLock(&g_flash_lock) != true)
            {
                return FLASH_ERR_BUSY;
            }
            err = flash_get_status();
            flash_softwareUnlock(&g_flash_lock);
            return err;
        }
        else
        {
            return FLASH_ERR_BUSY;
        }
#endif
     }



    switch (cmd)
    {
#if (FLASH_CFG_CODE_FLASH_ENABLE == 1)

    case FLASH_CMD_SWAPFLAG_TOGGLE:                /* Inverts the start-up program swap flag */
        /* This function is available in products with a 32-Kbyte or larger ROM */
        if (MCU_ROM_SIZE_BYTES < 32768)
        {
            return FLASH_ERR_FAILURE;
        }
#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1))
        if ((FIT_NO_FUNC == flash_ready_isr_handler))
        {
            return FLASH_ERR_FAILURE;
        }
#endif
        /*Grab the current flash state*/
        if (FLASH_SUCCESS != flash_lock_state(FLASH_WRITING))
        {
            /* API busy with another operation*/
            return FLASH_ERR_BUSY;
        }
        err = R_CF_ToggleStartupArea();
#if (FLASH_CFG_CODE_FLASH_BGO == 0)
        flash_release_state();
#endif
        break;

        
#if FLASH_HAS_ROM_CACHE
    case FLASH_CMD_ROM_CACHE_ENABLE:
        FLASH.ROMCIV.BIT.ROMCIV = 1;                // start invalidation
        while (FLASH.ROMCIV.BIT.ROMCIV != 0)        // wait for invalidation to complete
        {
            nop();
        }
        FLASH.ROMCE.BIT.ROMCEN = 1;                 // enable cache
        if (FLASH.ROMCE.BIT.ROMCEN != 1)
        {
            err = FLASH_ERR_FAILURE;
        }
        break;

    case FLASH_CMD_ROM_CACHE_DISABLE:
        FLASH.ROMCE.BIT.ROMCEN = 0;                 // disable cache
        break;

    case FLASH_CMD_ROM_CACHE_STATUS:
        *status = FLASH.ROMCE.BIT.ROMCEN;           // enabled/disabled state
        break;
#endif


    case FLASH_CMD_SWAPFLAG_GET:                   /* Returns the the current value of the start-up program swap flag. */
#if (FLASH_CFG_PARAM_CHECKING_ENABLE == 1)
        if ((pSwapInfo == NULL) || (pSwapInfo == FIT_NO_PTR))
        {
            return FLASH_ERR_NULL_PTR;
        }
#endif
        /* This function is available in products with a 32-Kbyte or larger ROM */
        if (MCU_ROM_SIZE_BYTES < 32768)
        {
            return FLASH_ERR_FAILURE;
        }
        *pSwapInfo = R_CF_GetCurrentStartupArea();
    break;

    
    case FLASH_CMD_SWAPSTATE_GET:                 /* Retrieves the current physical state of the start-up program area(which one is active).. */
        FLASH_RETURN_IF_PCFG_NULL;
        /* This function is available in products with a 32-Kbyte or larger ROM */
        if (MCU_ROM_SIZE_BYTES < 32768)
        {
            return FLASH_ERR_FAILURE;
        }
        *pSwapInfo = R_CF_GetCurrentSwapState();
    break;

    
    case FLASH_CMD_SWAPSTATE_SET:                 /* Swaps the start-up program areas without setting the start-up program swap flag. */
        FLASH_RETURN_IF_PCFG_NULL;
        FLASH_RETURN_IF_BAD_SAS;
        /* This function is available in products with a 32-Kbyte or larger ROM */
        if (MCU_ROM_SIZE_BYTES < 32768)
        {
            return FLASH_ERR_FAILURE;
        }
        R_CF_SetCurrentSwapState(*pSwapInfo);
    break;


        /* When providing block numbers, the start address is the larger block number (lower address) */
        //        access_info.start_addr =  (uint32_t)FLASH_CF_BLOCK_3;     // ie. This will allow writing block 3
        //        access_info.end_addr =  (uint32_t)(FLASH_CF_BLOCK_2);
        //        access_info.start_addr =  (uint32_t)FLASH_CF_BLOCK_8;     // ie. This will allow writing blocks 3 - 8
        //        access_info.end_addr =  (uint32_t)(FLASH_CF_BLOCK_2);
        //        any address in the block can also be provided, it will map to the Block # in which it lives.
        //        ie. access_info.start_addr = (uint32_t)FLASH_CF_BLOCK_3 = 0xFFFFF000 or 0xFFFFF020, etc

    case FLASH_CMD_ACCESSWINDOW_SET:        /* Works on full blocks only. Provide code block numbers or any address within a block */
#if (FLASH_CFG_PARAM_CHECKING_ENABLE == 1)
        if ((pAccessInfo == NULL) || (pAccessInfo == FIT_NO_PTR))
        {
            return FLASH_ERR_NULL_PTR;
        }

        if ((pAccessInfo->start_addr > pAccessInfo->end_addr)  ||
            (pAccessInfo->start_addr < (uint32_t)FLASH_CF_LOWEST_VALID_BLOCK) ||
            (pAccessInfo->start_addr > (uint32_t)FLASH_CF_BLOCK_END)    ||
            (pAccessInfo->end_addr < (uint32_t)FLASH_CF_LOWEST_VALID_BLOCK) ||
            (pAccessInfo->end_addr > (uint32_t)FLASH_CF_BLOCK_END))
        {
            return(FLASH_ERR_ACCESSW);
        }
        else if (((pAccessInfo->start_addr & (FLASH_CF_BLOCK_SIZE-1)) != 0)
              || ((pAccessInfo->end_addr & (FLASH_CF_BLOCK_SIZE-1)) != 0))
        {
            return (FLASH_ERR_ADDRESS);
        }
#endif
#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1))
        if ((FIT_NO_FUNC == flash_ready_isr_handler))
        {
            return FLASH_ERR_FAILURE;
        }
#endif
        /*Grab the current flash state*/
        if (FLASH_SUCCESS != flash_lock_state(FLASH_WRITING))
        {
            /* API busy with another operation*/
            return FLASH_ERR_BUSY;
        }

        err = R_CF_SetAccessWindow(pAccessInfo);
#if (FLASH_CFG_CODE_FLASH_BGO == 0)
        flash_release_state();
#endif
    break;

    
    case FLASH_CMD_ACCESSWINDOW_GET:
        if ((pAccessInfo == NULL) || (pAccessInfo == FIT_NO_PTR))
        {
            return FLASH_ERR_NULL_PTR;
        }
        err = R_CF_GetAccessWindow(pAccessInfo);
    break;
#endif  /* (FLASH_CFG_CODE_FLASH_ENABLE == 1) */


#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1)) || (FLASH_CFG_DATA_FLASH_BGO == 1)
    case FLASH_CMD_SET_BGO_CALLBACK: /* Set a user callback isr (If BGO (Data Flash or Code Flash) is enabled) */
        err = flash_interrupt_config (true, pcfg);
    break;
#endif


    case FLASH_CMD_STATUS_GET:
        err = flash_get_status();
    break;

    default:
        err = FLASH_ERR_PARAM;
    }
    return err;
}


/*****************************************************************************
* Function Name: flash_api_write
* Description  : This function executes RX1xx and RX23x specific functionality to write to
*                 the specified Code or Data Flash memory area.
*                Note that this function does not insure that the block(s) being written are blank.
*                If that information is required, R_FLASH_BlankCheck() should be used.
* Arguments    : uint32_t src_address -
*                 Source buffer address (data being written to Flash)
*                uint32_t dest_address -
*                 Destination address.
*                uint32_t num_bytes
*                 Number of bytes to be written
* Return Value : FLASH_SUCCESS -
*                    Write completed successfully; successfully initialized in case of BGO mode.
*                FLASH_ERR_BYTES -
*                    Number of bytes exceeds max range or is 0 or is not a valid multiple of the minimum programming
*                    size for the specified flash
*                FLASH_ERR_ADRRESS -
*                    src/dest address is an invalid Code/Data Flash block start address or
*                    address is not aligned with the minimum programming size for the Code/Data Flash
*                    For Code Flash programming the src address HAS to be a RAM address since ROM cannot be accessed
*                    during Code Flash programming
*                FLASH_ERR_BUSY -
*                    Flash peripheral is busy with another operation or not initialized
*                FLASH_ERR_FAILURE
*                    Flash Write operation failed for some other reason. This can be a result of trying to write to an area
*                    that has been protected via access control.
******************************************************************************/
flash_err_t flash_api_write(uint32_t src_address, uint32_t dest_address, uint32_t num_bytes)
{
    flash_err_t err;

    /*Check if API is busy*/
    if(g_flash_state != FLASH_READY)
    {
        /* API not initialized or busy with another operation */
        return FLASH_ERR_BUSY;
    }

#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1) || (FLASH_CFG_DATA_FLASH_BGO == 1))
    if ((FIT_NO_FUNC == flash_ready_isr_handler))
    {
        return FLASH_ERR_FAILURE;
    }
#endif

    /*Grab the current flash state*/
    if (FLASH_SUCCESS != flash_lock_state(FLASH_WRITING))
    {
        /* API busy with another operation*/
        return FLASH_ERR_BUSY;
    }


#if (FLASH_CFG_PARAM_CHECKING_ENABLE == 1)
    if (num_bytes == 0)
    {
        flash_release_state();
        return FLASH_ERR_BYTES;
    }

#endif

#if (FLASH_CFG_CODE_FLASH_ENABLE == 1)
    /*Check address validity and that it is on a Code Flash programming boundary*/
    /* Ensure start address is valid and on 256 (for CF) byte boundary */
    if (((dest_address >= (uint32_t)FLASH_CF_LOWEST_VALID_BLOCK)) &&  (!(dest_address & (FLASH_CF_MIN_PGM_SIZE-1))))
    {
        /*Check if there is enough space in the destination, and that num_bytes is a multiple of programming size*/
        if ((num_bytes-1 + (dest_address) > (uint32_t)FLASH_CF_BLOCK_END) ||
            (num_bytes & (FLASH_CF_MIN_PGM_SIZE-1)) ||
            (num_bytes-1 + (dest_address) < (uint32_t)FLASH_CF_BLOCK_INVALID))        /* Overflowed */
        {
            err = FLASH_ERR_BYTES;
        }
        else
        {
            /*There are no parameter errors. Switch to the CF PE mode*/
            err = flash_pe_mode_enter(FLASH_TYPE_CODE_FLASH);
            g_current_parameters.current_operation = FLASH_CUR_CF_WRITE;
            g_current_parameters.wait_cnt = WAIT_MAX_ROM_WRITE;
        }
    }
#endif
#ifndef FLASH_NO_DATA_FLASH       /* RX110/23T has no DF */

    /*Check address validity and that it is on a Data Flash programming boundary*/
    else if (((dest_address >= FLASH_DF_BLOCK_0) && (dest_address < FLASH_DF_BLOCK_INVALID ))
            &&
            (!(dest_address & (FLASH_DF_MIN_PGM_SIZE-1))))
    {
        /*Check if there is enough space in the destination, and that num_bytes is a multiple of programming size*/
        if ((num_bytes + (dest_address) > FLASH_DF_BLOCK_INVALID) || (num_bytes & (FLASH_DF_MIN_PGM_SIZE-1)))
        {
            err = FLASH_ERR_BYTES;
        }
        else
        {
            /*There are no parameter errors. Switch to the DF PE mode*/
            err = flash_pe_mode_enter(FLASH_TYPE_DATA_FLASH);
            g_current_parameters.current_operation = FLASH_CUR_DF_WRITE;
            g_current_parameters.wait_cnt = WAIT_MAX_DF_WRITE;
        }
    }
#endif
    else
    {
        err = FLASH_ERR_ADDRESS;
    }

    /*If parameters passed are not valid or there was an error entering PE mode,
     * then release the state and return the error condition*/
    if (FLASH_SUCCESS != err)
    {
        flash_release_state();
        return err;
    }

    /*Write the data*/
    err = flash_write(src_address, dest_address, num_bytes);
    if (FLASH_SUCCESS != err)
    {
        /*If there is an error, then reset the Flash circuit: This will clear error flags and exit the P/E mode*/
        flash_reset();
        flash_pe_mode_exit();
        /* Release the lock */
        flash_release_state();
        return err;
    }


    /*If in non-BGO (Blocking) mode, then current operation is completed and the result is in result. Exit from PE mode and return status*/
#if (FLASH_CFG_CODE_FLASH_BGO == 0)
    if (g_current_parameters.current_operation == FLASH_CUR_CF_WRITE)
    {
        /*Return to read mode*/
        err = flash_pe_mode_exit();
        if (FLASH_SUCCESS != err)
        {
            /*Reset the Flash circuit: This will stop any existing processes and exit PE mode*/
            flash_reset();
        }
        /*Release lock and reset the state to Idle*/
        flash_release_state();
    }
#endif

#if (FLASH_CFG_DATA_FLASH_BGO == 0)
    if (g_current_parameters.current_operation == FLASH_CUR_DF_WRITE)
    {
        /*Return to read mode*/
        err = flash_pe_mode_exit();
        if (FLASH_SUCCESS != err)
        {
            /*Reset the Flash circuit: This will stop any existing processes and exit PE mode*/
            flash_reset();
        }
        /*Release lock and reset the state to Idle*/
        flash_release_state();
    }
#endif



    /*If BGO is enabled, then
     *return result of started Write process*/
    return err;
}


/***********************************************************************************************************************
* Function Name: flash_api_erase
* Description  : This function executes RX1xx and RX23x specific functionality to erase the specified Code or Data Flash blocks.
* Arguments    : uint32_t block_start_address -
*                 Start address of the first block. Actual address or entry from "flash_block_address_t" enum can be used
*                 Note - An additional 'feature' of the RX100 and RX23x is that any address within a block may be specified and the
*                        entire contents of that block will be erased. The address supplied need not be on an Erase boundary
*                        or even an even boundary.
*                uint32_t num_blocks -
*                 Number of blocks to erase.
* Return Value : FLASH_SUCCESS -
*                    Erase completed successfully; successfully initialized in case of BGO mode.
*                FLASH_ERR_BLOCKS -
*                    Number of blocks exceeds max range or is 0
*                FLASH_ERR_ADRRESS -
*                    Start address is an invalid Code/Data Flash block start address
*                FLASH_ERR_BUSY -
*                    Flash peripheral is busy with another operation
*                FLASH_ERR_FAILURE
*                    Flash Write operation failed for some other reason. This can be a result of trying to erase an area
*                    that has been protected via access control.
***********************************************************************************************************************/
flash_err_t flash_api_erase(flash_block_address_t block_start_address, uint32_t num_blocks)
{
    flash_err_t err;

    /*Check if API is busy*/
    if(g_flash_state != FLASH_READY)
    {
        /* API not initialized or busy with another operation*/
        return FLASH_ERR_BUSY;
    }

#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1)) || (FLASH_CFG_DATA_FLASH_BGO == 1)
    if ((FIT_NO_FUNC == flash_ready_isr_handler))
    {
        return FLASH_ERR_FAILURE;
    }
#endif

    /*Grab the current flash state*/
    if (FLASH_SUCCESS != flash_lock_state(FLASH_ERASING))
    {
        /* API busy with another operation*/
        return FLASH_ERR_BUSY;
    }


#ifndef FLASH_NO_DATA_FLASH       /* RX110/23T has no DF */
    if ((block_start_address >= FLASH_DF_BLOCK_0) && (block_start_address < FLASH_DF_BLOCK_INVALID ))
    {
        if ((num_blocks > FLASH_NUM_BLOCKS_DF) ||
            (num_blocks <= 0) ||
            ((num_blocks * FLASH_DF_BLOCK_SIZE)-1  + (block_start_address) >= (uint32_t)FLASH_DF_BLOCK_INVALID))        /* Overflowed) */
        {
            err = FLASH_ERR_BLOCKS;
        }
        else if ((block_start_address & (FLASH_DF_BLOCK_SIZE-1)) != 0)
        {
            err = FLASH_ERR_ADDRESS;
        }
        else
        { /*No errors in parameters. Enter Data Flash PE mode*/
            err = flash_pe_mode_enter(FLASH_TYPE_DATA_FLASH);
            g_current_parameters.current_operation = FLASH_CUR_DF_ERASE;
            g_current_parameters.wait_cnt = WAIT_MAX_ERASE_DF_1K;
        }
    }
#endif

#if (FLASH_CFG_CODE_FLASH_ENABLE == 1)
#ifndef FLASH_NO_DATA_FLASH       /* RX110/23T has no DF */
    else
#endif

    /*Check address and num_blocks validity and switch to Code Flash or Data Flash P/E mode*/
    if ((block_start_address <= FLASH_CF_BLOCK_0) && (block_start_address >= FLASH_CF_LOWEST_VALID_BLOCK))
    {
#if (MCU_ROM_SIZE_BYTES > 0x40000L) && !defined(MCU_RX23_ALL)
        /* For parts with ROM > 256K, Blankcheck requests may not span the 256K boundary */
        /* Is the address in the first 256K? */
        if (block_start_address < (uint32_t)FLASH_CF_256KBOUNDARY) /* for 384 part range is 0xFFFA0000 -  0xFFFFFFFF */
        {
            if ((uint32_t)(block_start_address + (num_blocks * FLASH_CF_BLOCK_SIZE) - 1) > (uint32_t)FLASH_CF_256KBOUNDARY)
            {
                err = FLASH_ERR_ADDRESS;
            }
        }
#endif
        if ((num_blocks >= FLASH_NUM_BLOCKS_CF) ||
            (num_blocks <= 0) ||
            ((num_blocks * FLASH_CF_BLOCK_SIZE)-1  + (block_start_address) < (uint32_t)FLASH_CF_BLOCK_INVALID))        /* Overflowed */
        {
            err = FLASH_ERR_BLOCKS;
        }
        else if ((block_start_address & (FLASH_CF_BLOCK_SIZE-1)) != 0)
        {
            err = FLASH_ERR_ADDRESS;
        }
        else
        { /*No errors in parameters. Enter Code Flash PE mode*/
            err = flash_pe_mode_enter(FLASH_TYPE_CODE_FLASH);
            g_current_parameters.current_operation = FLASH_CUR_CF_ERASE;
            g_current_parameters.wait_cnt = WAIT_MAX_ERASE_CF_1K;
        }
    }
#endif
    else
    {
        err = FLASH_ERR_ADDRESS;
    }

    if (FLASH_SUCCESS != err)
    {
        flash_release_state();
        return err;
    }


    /*Erase the Blocks*/
    err = flash_erase((uint32_t)block_start_address, num_blocks);
    if (FLASH_SUCCESS != err)
    {
        /*If there is an error, then reset the Flash circuit: This will clear error flags and exit the P/E mode*/
        flash_reset();
        flash_pe_mode_exit();
        flash_release_state();
        return err;
    }

    /*If in non-BGO (Blocking) mode, then current operation is completed and the result is in result. Exit from PE mode and return status*/
#if (FLASH_CFG_CODE_FLASH_BGO == 0)
    if (g_current_parameters.current_operation == FLASH_CUR_CF_ERASE)
    {
        /*Return to read mode*/
        err = flash_pe_mode_exit();
        if (FLASH_SUCCESS != err)
        {
            /*Reset the Flash circuit: This will stop any existing processes and exit PE mode*/
            flash_reset();
        }
        /*Release lock and reset the state to Idle*/
        flash_release_state();
    }
#endif

#if (FLASH_CFG_DATA_FLASH_BGO == 0)
    if (g_current_parameters.current_operation == FLASH_CUR_DF_ERASE)
    {
        /*Return to read mode*/
        err = flash_pe_mode_exit();
        if (FLASH_SUCCESS != err)
        {
            /*Reset the Flash circuit: This will stop any existing processes and exit PE mode*/
            flash_reset();
        }
        /*Release lock and reset the state to Idle*/
        flash_release_state();
    }
#endif



    /*If BGO is enabled, then return result of started Erase process*/
    return err;
}


/***********************************************************************************************************************
* Function Name: flash_api_blankcheck
* Description  : This function executes RX1xx and RX23x specific functionality to blank check the specified Flash area.
*                The function will incrementally check the area from the start address onward.
*                The minimum number of bytes is 4 for CF and 1 for DF.
*                The max is 256K for CF, 8192 for DF.
*                For parts that may support > 256K CF, then the address range provided may not span a 256K boundary.
*                For example: with a 512K part one could not specify a range of 255K - 257K even though that is
*                a 2K range. That would require two seperate BlankCheck calls. One for 255-256, one for 256-257
* Arguments    : uint32_t address -
*                 Start address to perform blank check. Actual address or entry from "flash_block_address_t" enum can be used
*                uint32_t num_blocks -
*                 Number of bytes to perform blank check operation for.
*                flash_res_t *result
*                 Returns the result of the blank check operation. This field is valid only in non-BGO mode
*                 operation
* Return Value : FLASH_SUCCESS
*                 Operation was completed successfully
*                FLASH_ERR_BYTES -
*                    Number of bytes exceeds max range (1-65535) or is not a multiple of the minimum
*                    Code or Data programming size.
*                FLASH_ERR_ADRRESS -
*                    Start address is an invalid Data Flash Address
*                FLASH_ERR_BUSY
*                    Flash peripheral is busy with another operation or not initialized
*                FLASH_ERR_FAILURE -
*                    Operation failed for some other reason.*
***********************************************************************************************************************/
flash_err_t flash_api_blankcheck(uint32_t address, uint32_t num_bytes, flash_res_t *result)
{
    flash_err_t err = FLASH_SUCCESS;

    /*Check if API is busy*/
    if(g_flash_state != FLASH_READY)
    {
        /* API not initialized or busy with another operation*/
        return FLASH_ERR_BUSY;
    }

#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1) || (FLASH_CFG_DATA_FLASH_BGO == 1))
    if ((FIT_NO_FUNC == flash_ready_isr_handler))
    {
        return FLASH_ERR_FAILURE;
    }
#endif

    /*Grab the current flash state*/
    if (FLASH_SUCCESS != flash_lock_state(FLASH_BLANKCHECK))
    {
        /* API busy with another operation*/
        return FLASH_ERR_BUSY;
    }
#ifndef FLASH_NO_DATA_FLASH       /* RX110/23T has no DF */
    if (((address >= FLASH_DF_BLOCK_0) && (address < FLASH_DF_BLOCK_INVALID )))
    {
        /*Check if the range is valid, num_bytes is a multiple of the programming size, is larger than 0 and less than 65536*/
        if (((uint32_t)(num_bytes + address) > FLASH_DF_BLOCK_INVALID) ||
            (num_bytes & (FLASH_DF_MIN_PGM_SIZE-1)) ||
            (num_bytes == 0) ||
            (num_bytes > 8192))
        {
            err = FLASH_ERR_BYTES;
        }
        else
        {
            /*There are no parameter errors. Switch to the CF PE mode*/
            err = flash_pe_mode_enter(FLASH_TYPE_DATA_FLASH);
            g_current_parameters.current_operation = FLASH_CUR_DF_BLANKCHECK;
            g_current_parameters.wait_cnt = (WAIT_MAX_BLANK_CHECK * (num_bytes >> 2));
        }
    }
#endif

#if (FLASH_CFG_CODE_FLASH_ENABLE == 1)
#ifndef FLASH_NO_DATA_FLASH       /* RX110/23T has no DF */
    else
#endif
    if (address >= (uint32_t)(FLASH_CF_LOWEST_VALID_BLOCK))
    {
#if (MCU_ROM_SIZE_BYTES > 0x40000L) && !defined(MCU_RX23_ALL)
        /* For parts with ROM > 256K, Blankcheck requests may not span the 256K boundary */
        /* Is the address in the first 256K? */
        if (address < (uint32_t)FLASH_CF_256KBOUNDARY) /* for 384 part range is 0xFFFA0000 -  0xFFFFFFFF */
        {
            if ((uint32_t)(address + num_bytes) > (uint32_t)FLASH_CF_256KBOUNDARY)
            {
                err = FLASH_ERR_ADDRESS;
            }
        }
#endif
        /*Check if the range is valid, num_bytes is a multiple of 4, is larger than 0 and less than 65536*/
        if ((FLASH_SUCCESS == err) &&
            (((uint32_t)((num_bytes-1) + address) < (uint32_t)FLASH_CF_LOWEST_VALID_BLOCK) ||
            (num_bytes & (FLASH_CF_MIN_PGM_SIZE-1)) ||
            (num_bytes == 0) ||
            (num_bytes > MCU_ROM_SIZE_BYTES)))
        {
            err = FLASH_ERR_BYTES;
        }
        else
        {
            /*There are no parameter errors. Switch to the CF PE mode*/
            err = flash_pe_mode_enter(FLASH_TYPE_CODE_FLASH);
            g_current_parameters.current_operation = FLASH_CUR_CF_BLANKCHECK;
            g_current_parameters.wait_cnt = (WAIT_MAX_BLANK_CHECK * (num_bytes >> 2));
        }
    }
#endif /* (FLASH_CFG_CODE_FLASH_ENABLE == 1)*/
    else
    {
        err = FLASH_ERR_ADDRESS;
    }


    if (FLASH_SUCCESS != err)
    {
        flash_reset();
        flash_release_state();
        return err;
    }


    /*Initialize the result*/
    *result = FLASH_RES_INVALID;

    /*Start the blankcheck operation*/
    err = flash_blankcheck(address, num_bytes, result);
    if (FLASH_SUCCESS != err)
    {
        /*If there is an error, then reset the flash circuit: This will clear error flags and exit the P/E mode*/
        flash_reset();
        flash_pe_mode_exit();
        flash_release_state();
        return err;
    }

    /*If in non-BGO (Blocking) mode, then current operation is completed and the result is in result. Exit from PE mode and return status*/
#if (FLASH_CFG_CODE_FLASH_BGO == 0)
    if (g_current_parameters.current_operation == FLASH_CUR_CF_BLANKCHECK)
    {
        /*Return to read mode*/
        err = flash_pe_mode_exit();
        if (FLASH_SUCCESS != err)
        {
            /*Reset the Flash circuit: This will stop any existing processes and exit PE mode*/
            flash_reset();
        }
        /*Release lock and reset the state to Idle*/
        flash_release_state();
    }
#endif

#if (FLASH_CFG_DATA_FLASH_BGO == 0)
    if (g_current_parameters.current_operation == FLASH_CUR_DF_BLANKCHECK)
    {
        /*Return to read mode*/
        err = flash_pe_mode_exit();
        if (FLASH_SUCCESS != err)
        {
            /*Reset the Flash circuit: This will stop any existing processes and exit PE mode*/
            flash_reset();
        }
        /*Release lock and reset the state to Idle*/
        flash_release_state();
    }
#endif

    /*If BGO is enabled, then return result of started Erase process*/
    return err;
}


/***********************************************************************************************************
* Function Name: flash_get_status
* Description  : Returns the current state of the flash
*                NOTE1: This function does not have to execute from in RAM. It must be in RAM though if
*                CF BGO is enabled and this function is called during a ROM P/E operation.
* Arguments    : none
* Return Value : FLASH_SUCCESS -
*                    Flash is ready to use
*                FLASH_ERR_BUSY -
*                    Flash is busy with another operation or is uninitialized
***********************************************************************************************************/
flash_err_t flash_get_status (void)
{

    /* Return flash status */
    if( g_flash_state == FLASH_READY )
    {
        return FLASH_SUCCESS;
    }
    else
    {
        return FLASH_ERR_BUSY;
    }
}


/*******************************************************************************
* Outline      : Intrinsic function to specify the number of loops
* Header       : none
* Function Name: r_df_delay
* Description  : Wait processing that loops at a fixed five cycles.
* Arguments    : R1 : Waiting loop counter
* Return Value : none
*******************************************************************************/
#pragma inline_asm r_flash_delay
static void r_flash_delay  (unsigned long loop_cnt)
{
    BRA     ?+
    NOP
?:
    NOP
    SUB     #01H,R1
    BNE     ?-

}

/*******************************************************************************
* Outline      : Function that specifies the execution time
* Header       : none
* Function Name: r_flash_delay_us
* Description  : The number of loops is calculated based on the execution time
*              : and the sytem clock (ICLK) frequency, and the intrinsic function
*              : that specifies the number of loops is called.
* Arguments    : us  : Execution time
               : khz : ICLK frequency when calling the function
* Return Value : none
*******************************************************************************/
void r_flash_delay_us (unsigned long us, unsigned long khz)
{

    signed long loop_cnt; /* Argument of R_DELAY function */

    /* Calculation of a loop count */
    loop_cnt = us * khz;
    loop_cnt = (loop_cnt / WAIT_DIV_LOOP_CYCLE );      /* Division about cycle of 1 loop */
    loop_cnt = loop_cnt - WAIT_OVERHEAD_COUNT;         /* Overhead is reduced from a loop count. */

    /* R_DELAY function is performed when loop_cnt is 1 or more. */
    if(loop_cnt > 0)
    {
        r_flash_delay((unsigned long)loop_cnt);
    }
}

#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1)) || (FLASH_CFG_DATA_FLASH_BGO == 1)
/******************************************************************************
* Function Name: Excep_FCU_FRDYI
* Description  : ISR that is called when FCU is done with flash operation
*                NOTE: This function MUST execute from RAM only when
*                      FLASH_CFG_CODE_FLASH_BGO is enabled.
* Arguments    : none
* Return Value : none
******************************************************************************/
#pragma interrupt Excep_FCU_FRDYI(vect=VECT(FCU, FRDYI))
static void Excep_FCU_FRDYI(void)
{
    flash_err_t err = FLASH_SUCCESS;

#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1))

    if (FLASH_CUR_CF_ERASE  == g_current_parameters.current_operation)
    {
        err = R_CF_Erase_Check();
        if (FLASH_SUCCESS == err)
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_ERASE_COMPLETE;
        }
        else
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_ERR_FAILURE;
        }
    }
    else if (FLASH_CUR_CF_WRITE  == g_current_parameters.current_operation)
    {
        err = R_CF_Write_Check();
        if (FLASH_SUCCESS == err)
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_WRITE_COMPLETE;
        }
        else if ((FLASH_ERR_FAILURE == err) || (FLASH_ERR_TIMEOUT == err))
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_ERR_FAILURE;
        }
        else
        {
            /* Nothing to do */
        }
    }
    else if (FLASH_CUR_CF_BLANKCHECK == g_current_parameters.current_operation)
    {
        err = R_CF_BlankCheck_Check();
        if (FLASH_SUCCESS == err)
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_BLANK;
        }
        else
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_NOT_BLANK;
        }
    }
    else if (FLASH_CUR_CF_ACCESSWINDOW == g_current_parameters.current_operation)
    {
        err = r_cf_extra_check();
        if (FLASH_SUCCESS == err)
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_SET_ACCESSWINDOW;
        }
        else
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_ERR_FAILURE;
        }
    }
    else if (FLASH_CUR_CF_TOGGLE_STARTUPAREA == g_current_parameters.current_operation)
    {
        err = r_cf_extra_check();
        if (FLASH_SUCCESS == err)
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_TOGGLE_STARTUPAREA;
        }
        else
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_ERR_FAILURE;
        }
    }
    else
    {
        /* Nothing to do */
    }
#endif

#ifndef FLASH_NO_DATA_FLASH     /* RX110/23T has no DF */
#if (FLASH_CFG_DATA_FLASH_BGO == 1)
    if (FLASH_CUR_DF_ERASE == g_current_parameters.current_operation)
    {
        err = R_DF_Erase_Check();
        if (FLASH_SUCCESS == err)
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_ERASE_COMPLETE;
        }
        else
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_ERR_FAILURE;
        }
    }
    else if (FLASH_CUR_DF_WRITE  == g_current_parameters.current_operation)
    {
        err = R_DF_Write_Check();
        if (FLASH_SUCCESS == err)
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_WRITE_COMPLETE;
        }
        else if ((FLASH_ERR_FAILURE == err) || (FLASH_ERR_TIMEOUT == err))
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_ERR_FAILURE;
        }
        else
        {
            /* Nothing to do */
        }
    }
    else if (FLASH_CUR_DF_BLANKCHECK  == g_current_parameters.current_operation)
    {
        err = R_DF_BlankCheck_Check();
        if (FLASH_SUCCESS == err)
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_BLANK;
        }
        else
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_NOT_BLANK;
        }
    }
    else
    {
        /* Nothing to do */
    }
#endif
#endif

    if (FLASH_ERR_BUSY != err)
    {
        /* Release lock and Set current state to Idle */
        flash_pe_mode_exit();
        flash_release_state();
        g_current_parameters.current_operation = FLASH_CUR_IDLE;

        /* call back function execute */
        flash_ready_isr_handler((void *) &g_flash_int_ready_cb_args);
    }
}

#endif
#endif

#pragma section /* end FLASH_SECTION_ROM */

/* end of file */
