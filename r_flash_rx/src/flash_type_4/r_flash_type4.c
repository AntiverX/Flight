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
* Copyright (C) 2014 Renesas Electronics Corporation. All rights reserved.
********************************************************************************************************************/
/*******************************************************************************************************************
* File Name : r_flash_type4.c
* Description  : This module implements the Flash API control commands for the Flash Type 4 MCUs
********************************************************************************************************************/
/*******************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 11.02.2016 1.00    Initial version
*         : 12.08.2016 2.00    Modified for BSPless operation
*         : 17.11.2016 2.10    Fixed flash_reset() so enters PE mode before issuing a flash_stop()
*                                when in idle state.
*                              Fixed bug where interrupt was not handled properly when an internal flash_stop()
*                                was issued.
*                              Fixed bug where rounding was not handled in flash_clock_config().
*                              Added CF block boundary check to flash_api_erase().
*                              Added argument present check for FLASH_CMD_CONFIG_CLOCK.
*                              Added check that start address + number blocks does not exceed legal address range
*                                for flash_api_erase().
*                              Added check that callback function is set in BGO mode for flash_api_erase() and
*                                flash_api_write().
*                              Removed commented out code.
********************************************************************************************************************/


/********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
********************************************************************************************************************/
/* Includes board and MCU related header files. */
#include "r_flash_rx_if.h"
#include "r_flash_rx_config.h"
#if (FLASH_TYPE == FLASH_TYPE_4)

/* include definition for NULL*/
#include <stddef.h>


/******************************************************************************
Typedef definitions
******************************************************************************/
/* These typedefs are used for guaranteeing correct accesses to memory. When
   working with the FCU sometimes byte or word accesses are required. */
typedef __evenaccess volatile uint8_t  * FCU_BYTE_PTR;
typedef __evenaccess volatile uint16_t * FCU_WORD_PTR;
typedef __evenaccess volatile uint32_t * FCU_LONG_PTR;

/***********************************************************************************************************************
 Macro definitions
 ***********************************************************************************************************************/
#if (FLASH_CFG_PARAM_CHECKING_ENABLE == 1)
#define FLASH_RETURN_IF_PCFG_NULL   if ((pcfg == NULL) || (pcfg == FIT_NO_PTR))     \
                                    {                                               \
                                        return FLASH_ERR_NULL_PTR;                  \
                                    }
                                    // See FLASH_SAS_xxx #defines
#define FLASH_RETURN_IF_BAD_SAS     if ((*pSwapInfo == 1) || (*pSwapInfo > 4))      \
                                    {                                               \
                                        return FLASH_ERR_PARAM;                     \
                                    }
#else
#define FLASH_RETURN_IF_PCFG_NULL   // parameter checking disabled
#define FLASH_RETURN_IF_BAD_SAS     // parameter checking disabled
#endif

#if (((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1)) || FLASH_CFG_DATA_FLASH_BGO)
#define FLASH_RETURN_IF_BGO_AND_NO_CALLBACK     if ((flash_ready_isr_handler == FIT_NO_FUNC) || (flash_ready_isr_handler == NULL))  \
                                                {                                                                                   \
                                                    return FLASH_ERR_FAILURE;                                                       \
                                                }
#else
#define FLASH_RETURN_IF_BGO_AND_NO_CALLBACK     // in blocking mode (non-BGO)
#endif

#define FCU_COMMAND_AREA          (0x007E0000)
#define FCU_FIRMWARE_STORAGE_AREA (0xFEFFF000)
#define FCU_RAM_AREA              (0x007F8000)
#define FCU_RAM_SIZE              (4096)
//#define FAW_REG_ADDR              (0xFE7F5D64)

/*FACI Commands*/
#define FLASH_FACI_CMD_PROGRAM              0xE8
#define FLASH_FACI_CMD_PROGRAM_CF           0x80
#define FLASH_FACI_CMD_PROGRAM_DF           0x02
#define FLASH_FACI_CMD_BLOCK_ERASE          0x20
#define FLASH_FACI_CMD_PE_SUSPEND           0xB0
#define FLASH_FACI_CMD_PE_RESUME            0xD0
#define FLASH_FACI_CMD_STATUS_CLEAR         0x50
#define FLASH_FACI_CMD_FORCED_STOP          0xB3
#define FLASH_FACI_CMD_BLANK_CHECK          0x71
#define FLASH_FACI_CMD_CONFIG_SET_1         0x40
#define FLASH_FACI_CMD_CONFIG_SET_2         0x08
#define FLASH_FACI_CMD_LOCK_BIT_PGM         0x77
#define FLASH_FACI_CMD_LOCK_BIT_READ        0x71
#define FLASH_FACI_CMD_FINAL                0xD0


/* The number of loops to wait for FENTRYR timeout. */
#define FLASH_FENTRYR_TIMEOUT   (4)

/* The maximum timeout for commands is...   TODO (not in spec yet */
#define FLASH_FRDY_CMD_TIMEOUT  (50000)

/*Time that it would take for the Data Buffer to be empty (DBFULL Flag) is 90 FCLK cycles.
 * Assuming worst case of ICLK at 120 MHz and FCLK at 4 MHz, and optimization set to max such that
 * each count decrement loop takes only 5 cycles, then ((120/4)*90)/5 = 540 */
#define FLASH_DBFULL_TIMEOUT    (50000)       // TODO: 540 is too short or bad chip?


/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
FCU_BYTE_PTR g_pfcu_cmd_area = (uint8_t*) FCU_COMMAND_AREA;


flash_err_t flash_init();
flash_err_t flash_clock_config(uint32_t);
flash_err_t flash_fcuram_codecopy();
flash_err_t flash_reset();
flash_err_t flash_stop();
flash_err_t flash_interrupt_config(bool state, void *pCallback);

flash_err_t flash_write_faw_reg (fawreg_t   faw);
flash_err_t flash_pe_mode_enter(flash_type_t flash_type);
flash_err_t flash_pe_mode_exit();
flash_err_t flash_erase(uint32_t block_address, uint32_t block_count);
flash_err_t flash_write(uint32_t *src_start_address, uint32_t * dest_start_address, uint32_t *num_bytes);
flash_err_t flash_blankcheck(uint32_t start_address, uint32_t num_bytes, flash_res_t *blank_check_result);
flash_err_t flash_lock_state (flash_states_t new_state);
flash_err_t flash_get_status (void);
void flash_release_state (void);
flash_err_t check_cf_block_total(flash_block_address_t block_start_address, uint32_t num_blocks);


/* State variable for the Flash API. */
extern flash_states_t g_flash_state;

flash_int_cb_args_t g_flash_int_ready_cb_args;   // Callback argument structure for flash READY interrupt
flash_int_cb_args_t g_flash_int_error_cb_args;   // Callback argument structure for flash ERROR interrupt


/***********************************************************************************************************************
* Function Name: flash_api_open
* Description  : Function will initialize the flash for the RX64M.
* Arguments    : void
* Return Value : FLASH_SUCCESS -
*                    API initialized successfully.
*                FLASH_ERR_BUSY
*                    API has already been initialized and another operation is ongoing.
*                FLASH_ERR_FAILURE
*                    Initialization failed.
*                    A RESET was performed on the Flash circuit to rectify any internal errors
***********************************************************************************************************************/
flash_err_t flash_api_open(void)
{
    flash_err_t err;
#if (FLASH_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((MCU_CFG_FCLK_HZ > 60000000) || (MCU_CFG_FCLK_HZ < 4000000))
    {
        return FLASH_ERR_FREQUENCY;
    }
#endif
    /* Allow Initialization if not initialized or
     * if no operation is ongoing and re-initialization is desired */
    if ((FLASH_UNINITIALIZED == g_flash_state) || (FLASH_READY == g_flash_state))
    {
        if (FLASH_SUCCESS != flash_lock_state(FLASH_INITIALIZATION))
            {
                /* API already initialized */
                return FLASH_ERR_BUSY;
            }
    }

    /*Initialize the FCU*/
    err = flash_init();
    if (FLASH_SUCCESS != err)
    {
        return err;
    }

    g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_INITIALIZED;

    // Set the parameters struct based on the config file settings
    g_current_parameters.bgo_enabled_cf = FLASH_CFG_CODE_FLASH_BGO;
    g_current_parameters.bgo_enabled_df = FLASH_CFG_DATA_FLASH_BGO;



    /*Interrupts are enabled by default on this MCU.
     * Disable Interrupts*/
    flash_interrupt_config(false, NULL);
    /* Release state so other operations can be performed. */
    flash_release_state();
    return FLASH_SUCCESS;
}


/*All the functions below need to be placed in RAM if Code Flash programming is to be supported*/
#pragma section FRAM

/***********************************************************************************************************************
* Function Name: flash_api_write
* Description  : Function will write to the specified Code or Data Flash memory area.
* Arguments    : uint32_t src_address -
*                    Source buffer address.
*                uint32_t dest_address -
*                    Destination address.
*                uint32_t num_bytes
*                    Number of bytes to be written
* Return Value : FLASH_SUCCESS -
*                    Write completed successfully; successfully initialized in case of BGO mode.
*                FLASH_ERR_BYTES -
*                    Number of bytes exceeds max range or is 0 or is not a valid multiple of the minimum programming
*                    size for the specified flash
*                FLASH_ERR_ADDRESS -
*                    src/dest address is an invalid Code/Data Flash block start address or
*                    address is not aligned with the minimum programming size for the Code/Data Flash
*                    For Code Flash programming the src address HAS to be a RAM address since ROM cannot be accessed
*                    during Code Flash programming
*                FLASH_ERR_BUSY -
*                    Flash peripheral is busy with another operation or not initialized
*                FLASH_ERR_CMD_LOCKED -
*                    The FCU entered a command locked state and cannot process the operation.
*                    A RESET was performed on the FCU to recover from this state.
*                FLASH_ERR_FAILURE
*                    Code Flash Write operation attempted in BGO mode. This is temporarily not supported
*                    Operation failed for some other reason; RESET was performed on the FCU to recover from this state.
***********************************************************************************************************************/
flash_err_t flash_api_write(uint32_t src_address, uint32_t dest_address, uint32_t num_bytes)
{
    flash_err_t err;

#if (FLASH_CFG_PARAM_CHECKING_ENABLE == 1)
    /*See if number of bytes is non-zero*/
    if (num_bytes == 0)
    {
        return FLASH_ERR_BYTES;
    }
#endif

    FLASH_RETURN_IF_BGO_AND_NO_CALLBACK;

    /*Check if API is busy*/
    if(g_flash_state != FLASH_READY)
    {
        /* API not initialized or busy with another operation*/
        return FLASH_ERR_BUSY;
    }
    /*Grab the current flash state*/
    if (FLASH_SUCCESS != flash_lock_state(FLASH_WRITING))
    {
        /* API busy with another operation*/
        return FLASH_ERR_BUSY;
    }

    /*Check address validity and that it is on a Code Flash programming boundary*/
    // Ensure start address is valid and on 256 (for CF) byte boundary
    if (((dest_address > (uint32_t)FLASH_CF_BLOCK_INVALID)) &&  (!(dest_address & (FLASH_CF_MIN_PGM_SIZE-1))))
    {
         /*Check if there is enough space in the destination, and that num_bytes is a multiple of programming size*/
        if ((num_bytes-1 + (dest_address) > (uint32_t)FLASH_CF_BLOCK_END) ||
            (num_bytes & (FLASH_CF_MIN_PGM_SIZE-1)) ||
            (num_bytes-1 + (dest_address) < (uint32_t)FLASH_CF_BLOCK_INVALID))        // Overflowed

        {
            err = FLASH_ERR_BYTES;
        }
        else
        {
            if (g_current_parameters.bgo_enabled_cf == true)
            {
                //flash_release_state();
                //return FLASH_ERR_FAILURE;
                g_current_parameters.current_operation = FLASH_CUR_CF_BGO_WRITE;
            }
            else
            {
                g_current_parameters.current_operation = FLASH_CUR_CF_WRITE;
                g_current_parameters.wait_cnt = WAIT_MAX_ROM_WRITE;
            }
            g_current_parameters.min_pgm_size = (FLASH_CF_MIN_PGM_SIZE >> 1);
            /*There are no parameter errors. Switch to the CF PE mode*/
            err = flash_pe_mode_enter(FLASH_TYPE_CODE_FLASH);
        }

    }
#ifndef FLASH_NO_DATA_FLASH
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
            if (g_current_parameters.bgo_enabled_df == true)
            {
                g_current_parameters.current_operation = FLASH_CUR_DF_BGO_WRITE;
            }
            else
            {
                g_current_parameters.current_operation = FLASH_CUR_DF_WRITE;
            }
            g_current_parameters.wait_cnt = WAIT_MAX_DF_WRITE;
            g_current_parameters.min_pgm_size = (FLASH_DF_MIN_PGM_SIZE >> 1);
            /*There are no parameter errors. Switch to the DF PE mode*/
            err = flash_pe_mode_enter(FLASH_TYPE_DATA_FLASH);
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
    err = flash_write(&src_address, &dest_address, &num_bytes);
    if (FLASH_SUCCESS != err)
    {
        /*If there is an error, then reset the FCU: This will clear error flags and exit the P/E mode*/
        flash_reset();
        /*Release the lock*/
        flash_release_state();
        return err;
    }

    /*If in non-BGO mode, then current operation is completed.
     * Exit from PE mode and return status*/
    if ((g_current_parameters.current_operation == FLASH_CUR_CF_WRITE)
     || (g_current_parameters.current_operation == FLASH_CUR_DF_WRITE))
    {
        /*Return to read mode*/
        err = flash_pe_mode_exit();
        if (FLASH_SUCCESS != err)
        {
            /*Reset the FCU: This will stop any existing processes and exit PE mode*/
            flash_reset();
        }
        /*Release lock and reset the state to idle*/
        flash_release_state();
    }

    /*If BGO is enabled, then
     *return result of started Write process*/
    return err;
}


/***********************************************************************************************************************
* Function Name: flash_api_erase
* Description  : Function will erase the specified Code or Data Flash blocks.
* Arguments    : uint32_t block_start_address -
*                    Start address of the first block. Actual address or entry from "flash_block_address_t" enum can be used
*                uint32_t num_blocks -
*                    Number of blocks to erase.
* Return Value : FLASH_SUCCESS -
*                    Erase completed successfully; successfully initialized in case of BGO mode.
*                FLASH_ERR_BLOCKS -
*                    Number of blocks exceeds max range or is 0
*                FLASH_ERR_ADDRESS -
*                    Start address is an invalid Code/Data Flash block start address
*                FLASH_ERR_BUSY -
*                    Flash peripheral is busy with another operation
*                FLASH_ERR_FAILURE
*                    Code Flash Erase operation attempted in BGO mode. This is temporarily not supported
***********************************************************************************************************************/
flash_err_t flash_api_erase(flash_block_address_t block_start_address, uint32_t num_blocks)
{
    flash_err_t err;

    FLASH_RETURN_IF_BGO_AND_NO_CALLBACK;

    /*Check if API is busy*/
    if(g_flash_state != FLASH_READY)
    {
        /* API not initialized or busy with another operation*/
        return FLASH_ERR_BUSY;
    }

    /*Grab the current flash state*/
    if (FLASH_SUCCESS != flash_lock_state(FLASH_WRITING))
    {
        /* API busy with another operation*/
        return FLASH_ERR_BUSY;
    }

#if (FLASH_CFG_PARAM_CHECKING_ENABLE == 1)

#endif

    /*Check address and num_blocks validity and switch to Code Flash or Data Flash P/E mode*/
    if ((block_start_address <= FLASH_CF_BLOCK_0) && (block_start_address > FLASH_CF_BLOCK_INVALID))
    {
        if ((num_blocks > FLASH_NUM_BLOCKS_CF) || (num_blocks <= 0))
        {
            err = FLASH_ERR_BLOCKS;
        }
        else if ((err=check_cf_block_total(block_start_address, num_blocks)) == FLASH_SUCCESS)
        { /*No errors in parameters. Enter Code Flash PE mode*/
            err = flash_pe_mode_enter(FLASH_TYPE_CODE_FLASH);
            if (g_current_parameters.bgo_enabled_cf == true)
            {
                g_current_parameters.current_operation = FLASH_CUR_CF_BGO_ERASE;
            }
            else
            {
                g_current_parameters.current_operation = FLASH_CUR_CF_ERASE;
            }
        }
    }
#ifndef FLASH_NO_DATA_FLASH
    else if ((block_start_address >= FLASH_DF_BLOCK_0) && (block_start_address < FLASH_DF_BLOCK_INVALID ))
    {
        if ((num_blocks > FLASH_NUM_BLOCKS_DF) || (num_blocks <= 0))
        {
            err = FLASH_ERR_BLOCKS;
        }
        else if ((block_start_address & (FLASH_DF_BLOCK_SIZE-1)) != 0)
        {
            err = FLASH_ERR_BOUNDARY;
        }
        else if ((block_start_address + (num_blocks * FLASH_DF_BLOCK_SIZE) - 1) >= FLASH_DF_BLOCK_INVALID)
        {
            err = FLASH_ERR_BLOCKS;
        }
        else
        { /*No errors in parameters. Enter Data Flash PE mode*/
            err = flash_pe_mode_enter(FLASH_TYPE_DATA_FLASH);
            if (g_current_parameters.bgo_enabled_df == true)
            {
                g_current_parameters.current_operation = FLASH_CUR_DF_BGO_ERASE;
            }
            else
            {
                g_current_parameters.current_operation = FLASH_CUR_DF_ERASE;
            }
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

    /* Access Window protection not checked here
     * User Should configure it before calling function*/

    /*Erase the Blocks*/
    err = flash_erase((uint32_t)block_start_address, num_blocks);
    if (FLASH_SUCCESS != err)
    {
        /*If there is an error, then reset the FCU: This will clear error flags and exit the P/E mode*/
        flash_reset();
        flash_release_state();
        return err;
    }

    /*If in non-BGO mode, then current operation is completed.
     * Exit from PE mode and return status*/
    if ((g_current_parameters.current_operation == FLASH_CUR_CF_ERASE)
     || (g_current_parameters.current_operation == FLASH_CUR_DF_ERASE))
    {
        /*Return to read mode*/
        err = flash_pe_mode_exit();
        if (FLASH_SUCCESS != err)
        {
            /*Reset the FCU: This will stop any existing processes and exit PE mode*/
            flash_reset();
        }
        /*Release lock and reset the state to Idle*/
        flash_release_state();
    }
    /*If BGO is enabled, then return result of started Erase process*/
    return err;
}

/***********************************************************************************************************************
* Function Name: check_cf_block_total
* Description  : This function checks to see if the start address plus the number of blocks
*                remain in valid code flash address range.
* Arguments    : block_start_address -
*                    Start address for erase or lock bit
*                num_blocks -
*                    Number of blocks to erase or lock
* Return Value : FLASH_SUCCESS -
*                    Valid start address and number of blocks
*                FLASH_ERR_BLOCKS -
*                    Number of blocks would exceed legal address range
*                FLASH_ERR_ADRRESS -
*                    Start address is not on block boundary
***********************************************************************************************************************/
flash_err_t check_cf_block_total(flash_block_address_t block_start_address, uint32_t num_blocks)
{
#if (FLASH_CFG_CODE_FLASH_ENABLE == 1)
#if FLASH_ERASE_ASCENDING_BLOCK_NUMS
#if FLASH_CFG_PARAM_CHECKING_ENABLE
    uint32_t    tmp, cf_block_size;


    /* Legal number of blocks. See if start address is on block boundary */
    cf_block_size = (block_start_address >= FLASH_CF_BLOCK_7) ? FLASH_CF_SMALL_BLOCK_SIZE : FLASH_CF_MEDIUM_BLOCK_SIZE;
    if ((block_start_address & (cf_block_size - 1)) != 0)
    {
        return FLASH_ERR_ADDRESS;
    }


    /* See if valid (does not go pass address boundary */

    if (block_start_address >= FLASH_CF_BLOCK_7)
    {
        /* set tmp = available number of small blocks to erase */
        tmp = ((block_start_address - FLASH_CF_BLOCK_7) / FLASH_CF_SMALL_BLOCK_SIZE) + 1;
        /* set num_blocks = number of large blocks to erase */
        num_blocks -= (tmp < num_blocks) ? tmp : num_blocks;
        /* adjusted start address for number of large blocks to erase */
        block_start_address = FLASH_CF_BLOCK_8;
    }

    /* set num_blocks = large blocks to erase below start address */
    num_blocks--;
    /* determine if num_blocks goes pass address limit */
    if ((block_start_address - (num_blocks*FLASH_CF_MEDIUM_BLOCK_SIZE)) <= (uint32_t)FLASH_CF_BLOCK_INVALID)
    {
        return FLASH_ERR_BLOCKS;
    }
#endif  // param checking
#endif  // ascending block numbers
    return FLASH_SUCCESS;
#else   // code flash disabled
    return FLASH_ERR_FAILURE;
#endif
}


/*******************************************************************************
* Function Name: R_CF_ToggleStartupArea
* Description  : Check the current start-up area setting
*              : and specifies the area currently not used as the start-up area.
* Arguments    : none
* Return Value : FLASH_SUCCESS -
*                Switched successfully.
*                FLASH_ERR_FAILURE -
*                Unable to Switch to P/E Mode.
*                FLASH_ERR_PARAM -
*                Illegal parameter passed
*******************************************************************************/
flash_err_t R_CF_ToggleStartupArea (void)
{
    flash_err_t err;
    fawreg_t    faw;

    g_current_parameters.current_operation = FLASH_CUR_CF_TOGGLE_STARTUPAREA;

    faw.LONG = FLASH.FAWMON.LONG;

    faw.BIT.BTFLG ^= 1;

    err = flash_write_faw_reg(faw);

    return err;
}


/*******************************************************************************
* Function Name: R_CF_GetCurrentStartupArea
* Description  : Return which startup area (Default or Alternate) is active
* Arguments    : none
* Return Value : startup_area_flag - 0 ==> Alternate area
*                                    1 ==> Default area
*******************************************************************************/
uint8_t R_CF_GetCurrentStartupArea(void)
{
    fawreg_t    faw;

    faw.LONG = FLASH.FAWMON.LONG;

    return(faw.BIT.BTFLG);
}


/*******************************************************************************
* Function Name: R_CF_GetCurrentSwapState
* Description  : Return which startup area (Default or Alternate) is active
* Arguments    : none
* Return Value : startup_area_select - 0 ==> The start-up area is selected
*                                            according to the start-up area
*                                            settings of the extra area
*                                      2 ==> The start-up area is switched to
*                                            the default area temporarily.
*                                      3 ==> The start-up area is switched to
*                                            the alternate area temporarily.
*******************************************************************************/
uint8_t R_CF_GetCurrentSwapState(void)
{

    return(FLASH.FSUACR.BIT.SAS);
}


/*******************************************************************************
* Function Name: R_CF_SetCurrentSwapState
* Description  : Setting for switching the start-up area
* Arguments    : value for SAS bits; switch startup area if value = SAS_SWITCH_AREA
* Return Value : none
*******************************************************************************/
void R_CF_SetCurrentSwapState(uint8_t value)
{
    uint8_t     sas_flag;
    uint16_t    reg_value;


    if (FLASH_SAS_SWITCH_AREA == value)     /* to switch startup areas */
    {
        if (FLASH_SAS_SWAPFLG == FLASH.FSUACR.BIT.SAS)
        {
            if (1 == FLASH.FAWMON.BIT.BTFLG)     // 1 = default
            {
                sas_flag = FLASH_SAS_ALTERNATE;
            }
            else
            {
                sas_flag = FLASH_SAS_DEFAULT;
            }
        }
        else
        {
            if (FLASH_SAS_ALTERNATE == FLASH.FSUACR.BIT.SAS)
            {
                sas_flag = FLASH_SAS_DEFAULT;
            }
            else
            {
                sas_flag = FLASH_SAS_ALTERNATE;
            }
        }
    }
    else
    {
        sas_flag = value;       /* to set SAS to desired area */
    }


    reg_value = 0x6600 | (uint16_t)sas_flag;
    FLASH.FSUACR.WORD = reg_value;

    while(sas_flag != FLASH.FSUACR.BIT.SAS)
    {
        /* Confirm that the written value can be read correctly. */
    }
}


/*******************************************************************************
* Function Name: R_CF_SetAccessWindow
* Description  : Specifies the setting for the access window.
* Arguments    : pAccessInfo->start_addr -
*                   start address of Access Window Setting
*              : pAccessInfo->end_addr -
*                   end address of Access Window Setting. This should be one
*                   beyond the actual last byte to allow write access for.
*
* Return Value : FLASH_SUCCESS            - Command executed successfully
*              : FLASH_ERR_ACCESSW        - AccessWindow setting error
*******************************************************************************/
flash_err_t R_CF_SetAccessWindow (flash_access_window_config_t  *pAccessInfo)
{
    flash_err_t err = FLASH_SUCCESS;
    fawreg_t    faw;

#if (FLASH_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((pAccessInfo->start_addr < (uint32_t)FLASH_CF_LOWEST_VALID_BLOCK)
     || (pAccessInfo->start_addr > (uint32_t)FLASH_CF_BLOCK_0)
     || (pAccessInfo->end_addr < (uint32_t)FLASH_CF_LOWEST_VALID_BLOCK)
     || (pAccessInfo->end_addr > (uint32_t)FLASH_CF_BLOCK_0)
     || (pAccessInfo->end_addr < pAccessInfo->start_addr))
    {
        return FLASH_ERR_ADDRESS;
    }
#endif

    g_current_parameters.current_operation = FLASH_CUR_CF_ACCESSWINDOW;

    faw.LONG = FLASH.FAWMON.LONG;

    faw.BIT.FAWS = (pAccessInfo->start_addr & 0x00FFE000) >> 13;
    faw.BIT.FAWE = (pAccessInfo->end_addr & 0x00FFE000) >> 13;

    err = flash_write_faw_reg(faw);

    return err;
}


/*******************************************************************************
* Function Name: flash_write_faw_reg
* Description  : Writes the contents of the argument to the FAW register.
* Arguments    : start_addr : start address of Access Window Setting
*              : end_addr   : end address of Access Window Setting. This should be one
*                             beyond the actual last byte to allow write access for.
*                             here as required by the spec.
* Return Value : FLASH_SUCCESS            - Command executed successfully
*              : FLASH_ERR_ACCESSW        - AccessWindow setting error
*******************************************************************************/
flash_err_t flash_write_faw_reg (fawreg_t   faw)
{
    flash_err_t err = FLASH_SUCCESS;

    err = flash_pe_mode_enter(FLASH_TYPE_CODE_FLASH);
    if (FLASH_SUCCESS != err)
    {
        return(err);
    }

    faw.BIT.FSPR = 1;          // p/e enabled (allow rewrite of flash; 0 locks chip forever)

#ifdef __BIG    // Big endian
    uint32_t    swapped;
    swapped = (faw.LONG << 16) | ((faw.LONG >> 16) & 0x0000FFFF);
    faw.LONG = swapped;
#endif

    FLASH.FSADDR.BIT.FSADDR = 0x00FF5D60;       // FSADDR reg specific addr for FAW register
    *g_pfcu_cmd_area = (uint8_t) 0x40;
    *g_pfcu_cmd_area = (uint8_t) 0x08;

    *((uint16_t *)g_pfcu_cmd_area) = 0xFFFF;    // data for 0x00FF5D60
    *((uint16_t *)g_pfcu_cmd_area) = 0xFFFF;    // data for 0x00FF5D62
    *((uint16_t *)g_pfcu_cmd_area) = (uint16_t) (faw.LONG & 0x000FFFF);         // data for 0x00FF5D64
    *((uint16_t *)g_pfcu_cmd_area) = (uint16_t) ((faw.LONG >> 16) & 0x000FFFF); // data for 0x00FF5D66
    *((uint16_t *)g_pfcu_cmd_area) = 0xFFFF;    // data for 0x00FF5D68
    *((uint16_t *)g_pfcu_cmd_area) = 0xFFFF;    // data for 0x00FF5D6A
    *((uint16_t *)g_pfcu_cmd_area) = 0xFFFF;    // data for 0x00FF5D6C
    *((uint16_t *)g_pfcu_cmd_area) = 0xFFFF;    // data for 0x00FF5D6E
    *g_pfcu_cmd_area = (uint8_t) 0xD0;


#if (FLASH_CFG_CODE_FLASH_BGO == 0)
    g_current_parameters.wait_cnt = FLASH_FRDY_CMD_TIMEOUT;
    while (1 != FLASH.FSTATR.BIT.FRDY)
    {
        /* Wait until FRDY is 1 unless timeout occurs. */
        if (g_current_parameters.wait_cnt-- <= 0)
        {
            /* if FRDY is not set to 1 after max timeout, issue the stop command*/
            err = flash_stop();
            return err;
        }
    }

    flash_pe_mode_exit();
#endif

    return err;
}



/*******************************************************************************
* Function Name: R_CF_GetAccessWindow
* Description  : Return the read address form of the current access window area setting
* Arguments    : none
* Return Value : FLASH_SUCCESS
*******************************************************************************/
flash_err_t R_CF_GetAccessWindow (flash_access_window_config_t  *pAccessInfo)
{

    pAccessInfo->start_addr = ((FLASH.FAWMON.BIT.FAWS << 13) | 0xFF000000);
    pAccessInfo->end_addr = ((FLASH.FAWMON.BIT.FAWE << 13) | 0xFF000000);

    return FLASH_SUCCESS;
}



/***********************************************************************************************************************
* Function Name: flash_api_blankcheck
* Description  : N/A
* Arguments    : uint32_t address -
*                 Start address to perform blank check. Actual address or entry from "flash_block_address_t" enum can be used
*                uint32_t num_blocks -
*                 Number of bytes to perform blank check operation for.
*                flash_res_t *result
*                 Returns the result of the blank check operation. This field is valid only in non-BGO mode
*                 operation
* Return Value : FLASH_ERR_UNSUPPORTED
***********************************************************************************************************************/
flash_err_t flash_api_blankcheck(uint32_t address, uint32_t num_bytes, flash_res_t *result)
{
    return FLASH_ERR_UNSUPPORTED;
}


/***********************************************************************************************************************
* Function Name: flash_api_control
* Description  : This function supports additional configuration operations.
*                The supported commands are listed in the flash_cmd_t enum.
* Arguments    : flash_cmd_t cmd -
*                   command
*                void *pcfg -
*                   Pointer to configuration. This argument can be NULL for
*                   commands that do not require a configuration.
*
*                Command                                | Argument
*                FLASH_CMD_RESET------------------------| NULL
*                FLASH_CMD_STATUS_GET-------------------| NULL
*                FLASH_CMD_SET_BGO_CALLBACK-------------| flash_interrupt_config_t *
*                FLASH_CMD_SWAPFLAG_TOGGLE--------------| NULL
*                FLASH_CMD_SWAPFLAG_GET-----------------| uint8_t *
*                FLASH_CMD_SWAPSTATE_GET----------------| uint8_t *
*                FLASH_CMD_SWAPSTATE_SET----------------| uint8_t *
*                FLASH_CMD_ACCESSWINDOW_SET-------------| flash_access_window_config_t *
*                FLASH_CMD_ACCESSWINDOW_GET-------------| flash_access_window_config_t *
*                FLASH_CMD_CONFIG_CLOCK-----------------| uint32_t *
*                FLASH_CMD_ROM_CACHE_ENABLE-------------| NULL
*                FLASH_CMD_ROM_CACHE_DISABLE------------| NULL
*                FLASH_CMD_ROM_CACHE_STATUS-------------| uint8_t *
*
* Return Value : FLASH_SUCCESS -
*                    Operation completed successfully.
*                FLASH_ERR_ADDRESS -
*                    Address is an invalid block address
*                FLASH_ERR_NULL_PTR -
*                    Pointer was NULL for a command that expects a configuration structure
*                FLASH_ERR_BUSY -
*                    Flash peripheral is busy with another operation or not initialized
*                FLASH_ERR_LOCKED -
*                    The FCU was in a command locked state and has been reset
*                FLASH_ERR_FREQUENCY
*                    Illegal Frequency parameter passed for FLASH_CMD_CONFIG_CLOCK command
*                FLASH_ERR_UNSUPPORTED
*                    Unrecognized commands
***********************************************************************************************************************/
flash_err_t flash_api_control(flash_cmd_t cmd, void *pcfg)
{
    flash_err_t err = FLASH_SUCCESS;
    uint8_t *status = pcfg;
    uint8_t *pSwapInfo = pcfg;
    flash_access_window_config_t  *pAccessInfo = pcfg;
    uint32_t *plocal_pcfg;

    /*If the command is to reset the FCU, then no attempt is made to grab the lock
     * before executing the reset since the assumption is that the RESET command
     * is used to terminate any existing operation*/
    if (cmd == FLASH_CMD_RESET)
    {
        err = flash_reset();
        flash_release_state();
        return err;
    }

    /*Check if API is busy*/
    if(g_flash_state != FLASH_READY)
    {
        /* API not initialized or busy with another operation*/
        return FLASH_ERR_BUSY;
    }

    switch (cmd)
    {
        // If BGO (Data Flash or Code Flash) is enabled, then allow user to set a callback isr
#if (FLASH_CFG_CODE_FLASH_BGO == 1)  || (FLASH_CFG_DATA_FLASH_BGO == 1)
        case FLASH_CMD_SET_BGO_CALLBACK:
            err = flash_interrupt_config(true, pcfg);
        break;
#endif


    case FLASH_CMD_SWAPFLAG_TOGGLE:                /* Inverts the start-up program swap flag */
        /* This function is available in products with a 32-Kbyte or larger ROM */
        if (MCU_ROM_SIZE_BYTES < 32768)
        {
            return FLASH_ERR_FAILURE;
        }
        FLASH_RETURN_IF_BGO_AND_NO_CALLBACK;

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

        if ((pAccessInfo->start_addr >= FLASH_CF_BLOCK_7)
         && ((pAccessInfo->start_addr & (FLASH_CF_SMALL_BLOCK_SIZE-1)) != 0))
        {
            return FLASH_ERR_BOUNDARY;
        }
        else if ((pAccessInfo->start_addr < FLASH_CF_BLOCK_7)
         && ((pAccessInfo->start_addr & (FLASH_CF_MEDIUM_BLOCK_SIZE-1)) != 0))
        {
            return FLASH_ERR_BOUNDARY;
        }
        else if ((pAccessInfo->end_addr >= FLASH_CF_BLOCK_7)
         && ((pAccessInfo->end_addr & (FLASH_CF_SMALL_BLOCK_SIZE-1)) != 0))
        {
            return FLASH_ERR_BOUNDARY;
        }
        else if ((pAccessInfo->end_addr < FLASH_CF_BLOCK_7)
         && ((pAccessInfo->end_addr & (FLASH_CF_MEDIUM_BLOCK_SIZE-1)) != 0))
        {
            return FLASH_ERR_BOUNDARY;
        }
#endif
        FLASH_RETURN_IF_BGO_AND_NO_CALLBACK;

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


    case FLASH_CMD_STATUS_GET:
        err = flash_get_status();
        break;

    case FLASH_CMD_CONFIG_CLOCK:
        FLASH_RETURN_IF_PCFG_NULL;
        plocal_pcfg =pcfg;
        if ((*plocal_pcfg >= 4000000) && (*plocal_pcfg <=60000000))
        {
            err = flash_clock_config(*plocal_pcfg);
        }
        else
        {
            err = FLASH_ERR_FREQUENCY;
        }
        break;


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

    default:
        err = FLASH_ERR_UNSUPPORTED;
    }
    flash_release_state();
    return err;
}

#pragma section // end FLASH_SECTION FRAM; Control() needs to be in RAM due to FAW operation commands


/***********************************************************************************************************************
 * Function Name: flash_init
 * Description  : Function will initialize the FCU, copy the FCU firmware to FCURAM and
 *                set the FCU Clock based on the current FCLK frequency
 * Arguments    :
 * Return Value : FLASH_SUCCESS -
 *                    Peripheral Initialized successfully.
 *                FLASH_ERR_TIMEOUT
 *                  Timed out while attempting to switch to P/E mode or
 *                  while trying to issue a STOP or an ongoing flash operation timed out.
 *                FLASH_ERR_LOCKED
 *                 Switch to Read mode timed out and STOP was attempted to recover. Stop operation failed.
 *                 Peripheral in locked state.
 ***********************************************************************************************************************/
flash_err_t flash_init()
{
    flash_err_t err = FLASH_SUCCESS;

    g_current_parameters.current_operation = FLASH_CUR_FCU_INIT;

    /*Allow Access to the Flash registers*/
    FLASH.FWEPROR.BYTE = 0x01;
    /*Set the Clock*/
    FLASH.FPCKAR.WORD = ((0x1E00) + ((MCU_CFG_FCLK_HZ/1000000)));

    return err;
}


/***********************************************************************************************************************
 * Function Name: flash_interrupt_config
 * Description  : Function to enable of disable flash interrupts.
 * Arguments    : bool
 *               - true: Enable Interrupts (BGO mode)
 *               - false: Disable interrupts (non-BGO mode)
 *               *pCallback
 *               - Callback function that needs to be called from the ISR in case of an error or after operation
 *               is complete.
 *               - This argument can be a NULL if interrupts are being disabled (non-BGO mode)
 * Return Value : FLASH_SUCCESS -
 *                Interrupts configured successfully.
 *                FLASH_ERR_NULL_PTR -
 *                Invalid callback function pointer passed when enabling interrupts
 * Note     :
 ***********************************************************************************************************************/
flash_err_t flash_interrupt_config(bool state, void *pcfg)
{
    flash_interrupt_config_t *int_cfg = pcfg;

    /*Enable the Interrupts*/
    if (true == state)
    {
        /* Assign the callback if not NULL*/
        if ((NULL == int_cfg->pcallback) || (FIT_NO_FUNC == int_cfg->pcallback))
        {
            return FLASH_ERR_NULL_PTR;
        }
        flash_ready_isr_handler = int_cfg->pcallback;
        flash_error_isr_handler = int_cfg->pcallback;

        FLASH.FAEINT.BYTE = 0x90;
        FLASH.FRDYIE.BYTE = 0x01;
        /*Clear Flash Ready Interrupt Request*/
        IR(FCU,FRDYI)= 0;
        /*Clear Flash Error Interrupt Request*/
        IR(FCU,FIFERR)= 0;
        /*Set Flash Error Interrupt Priority*/
        IPR(FCU,FIFERR)= int_cfg->int_priority;
        /*Set Flash Ready Interrupt Priority*/
        IPR(FCU,FRDYI)= int_cfg->int_priority;
        /*Enable Flash Error Interrupt*/
        IEN (FCU,FIFERR)= 1;
        /*Enable Flash Ready Interrupt*/
        IEN (FCU,FRDYI)= 1;

    }
    /*Disable the Interrupts*/
    else if (false == state)
    {
        FLASH.FAEINT.BYTE = 0x00;
        FLASH.FRDYIE.BYTE = 0x00;
        /*Clear any pending Flash Ready Interrupt Request*/
        IR(FCU,FRDYI) = 0;
        /*Clear any pending Flash Error Interrupt Request*/
        IR(FCU,FIFERR) = 0;
    }

    return FLASH_SUCCESS;
}

#if (FLASH_CFG_CODE_FLASH_ENABLE == 1)
/*All the functions below need to be placed in RAM if Code Flash programming is to be supported
 * in non-BGO mode*/
#pragma section FRAM
#endif

/***********************************************************************************************************************
 * Function Name: flash_clock_config
 * Description  : Function sets the internal FCu register to the current FCLK frequency
 * Arguments    : uint32_t : Specify current FCLK frequency
 * Return Value : FLASH_SUCCESS -
 *                Switched successfully.
 * NOTE         : Legal values are 4000000 - 60000000 (4 MHz to 60 MHz)
 ***********************************************************************************************************************/
flash_err_t flash_clock_config(uint32_t cur_fclk)
{
    uint32_t    speed_mhz;

    speed_mhz = cur_fclk / 1000000;
    if ((cur_fclk % 1000000) != 0)
    {
        speed_mhz++;    // must round up to nearest MHz
    }

    /*Set the Clock*/
    FLASH.FPCKAR.WORD = (uint16_t)(0x1E00) + (uint16_t)speed_mhz;
    return FLASH_SUCCESS;
}


/***********************************************************************************************************************
 * Function Name: flash_pe_mode_enter
 * Description  : Function switches the peripheral to P/E mode for Code Flash or Data Flash.
 * Arguments    : flash_type_t : Specify Code Flash or Data Flash
 * Return Value : FLASH_SUCCESS -
 *                Switched successfully.
 *                FLASH_ERR_FAILURE -
 *                Unable to Switch to P/E Mode.
 *                FLASH_ERR_PARAM -
 *                Illegal parameter passed
 * NOTE         : This function must run from RAM if Code Flash non-BGO is used
 ***********************************************************************************************************************/
flash_err_t flash_pe_mode_enter(flash_type_t flash_type)
{
    flash_err_t err = FLASH_SUCCESS;

    if (flash_type == FLASH_TYPE_DATA_FLASH)
    {
        FLASH.FENTRYR.WORD = 0xAA80;        //Transition to DF P/E mode
        if (FLASH.FENTRYR.WORD == 0x0080)
        {
            err = FLASH_SUCCESS;
        }
        else
        {
            err = FLASH_ERR_FAILURE;
        }
    }
    else if (flash_type == FLASH_TYPE_CODE_FLASH)
    {
        FLASH.FENTRYR.WORD = 0xAA01;        //Transition to CF P/E mode
        while (FLASH.FENTRYR.WORD != 0x0001)
            ;

        if (FLASH.FENTRYR.WORD == 0x0001)
        {
            err = FLASH_SUCCESS;
        }
        else
        {
            err = FLASH_ERR_FAILURE;
        }
    }
    else
    {
        err = FLASH_ERR_PARAM;
    }

    return err;
}


/***********************************************************************************************************************
 * Function Name: flash_pe_mode_exit
 * Description  : Function switches the peripheral from P/E mode for Code Flash or Data Flash to Read mode
 *
 * Arguments    : None.
 * Return Value : FLASH_SUCCESS -
 *                Switched successfully.
 *                FLASH_ERR_TIMEOUT
 *                Operation timed out. Ongoing flash operation failed.
 *                FLASH_ERR_LOCKED
 *                Peripheral in locked state. Operation failed.
 * Notes     :    -When performing Code Flash operations, this function must be run from RAM or from the
 *                separate flash block for MCUs that are larger than 2MB. It can be run from ROM if
 *                only a Data Flash operation is ongoing.
 *                -This function should generally be called only after any current operation is complete.
 *                To allow for the worst case where this function is called immediately after a 32K code Flash
 *                erase operation has started a timeout of 580 ms is used.
 ***********************************************************************************************************************/
flash_err_t flash_pe_mode_exit()
{
    flash_err_t err = FLASH_SUCCESS;
    flash_err_t temp_err = FLASH_SUCCESS;
    /* Timeout counter. */
    volatile uint32_t wait_cnt = FLASH_FRDY_CMD_TIMEOUT;

    /* Read FRDY bit until it has been set to 1 indicating that the current
     * operation is complete.*/
    while (1 != FLASH.FSTATR.BIT.FRDY)
    {
        /* Wait until FRDY is 1 unless timeout occurs. */
        if (wait_cnt-- <= 0)
        {
            /* if FRDY is not set to 1 after max timeout, return timeout status*/
            return FLASH_ERR_TIMEOUT;
        }
    }

    /*Check if Command Lock bit is set*/
    if (0 != FLASH.FASTAT.BIT.CMDLK)
    {
        /*Issue a Status Clear to clear Command Locked state*/
        *g_pfcu_cmd_area = (uint8_t) FLASH_FACI_CMD_STATUS_CLEAR;
        temp_err = FLASH_ERR_CMD_LOCKED;
    }

    /*Transition to Read mode*/
    FLASH.FENTRYR.WORD = 0xAA00;
    while (FLASH.FENTRYR.WORD != 0x0000)
        ;


    if (FLASH.FENTRYR.WORD == 0x0000)
    {
        err = FLASH_SUCCESS;
    }
    else
    {
        err = FLASH_ERR_FAILURE;
    }

    /*If a command locked state was detected earlier, then return that error*/
    if (FLASH_ERR_CMD_LOCKED == temp_err)
    {
        return temp_err;
    }
    else
    {
        return err;
    }

}


/***********************************************************************************************************************
 * Function Name: flash_stop
 * Description  : Function issue the STOP command and check the state of the Command Lock bit.
 * Arguments    :
 * Return Value : FLASH_SUCCESS -
 *                Stop issued successfully.
 *                FLASH_ERR_TIMEOUT
 *                Timed out
 *                FLASH_ERR_LOCKED
 *                Peripheral in locked state
 ***********************************************************************************************************************/
flash_err_t flash_stop()
{
    /* Timeout counter. */
    volatile uint32_t wait_cnt = FLASH_FRDY_CMD_TIMEOUT;

    g_current_parameters.current_operation = FLASH_CUR_STOP;

    /*Issue stop command to flash command area*/
    *g_pfcu_cmd_area = (uint8_t) FLASH_FACI_CMD_FORCED_STOP;

    /* Read FRDY bit until it has been set to 1 indicating that the current
     * operation is complete. ALWAYS WAIT HERE! BGO OR NOT. DO NOT COUNT ON USER TO DETECT THIS! */
    while (1 != FLASH.FSTATR.BIT.FRDY)
    {
        /* Wait until FRDY is 1 unless timeout occurs. */
        if (wait_cnt-- <= 0)
        {
            /* This should not happen normally.
             * FRDY should get set in 15-20 ICLK cycles on STOP command*/
            return FLASH_ERR_TIMEOUT;
        }
    }
    /*Check that Command Lock bit is cleared*/
    if (0 != FLASH.FASTAT.BIT.CMDLK)
    {
        return FLASH_ERR_CMD_LOCKED;
    }

    return FLASH_SUCCESS;
}


/***********************************************************************************************************************
 * Function Name: flash_erase
 * Description  : Function erases a block of Code or Data Flash
 ** Arguments   : uint32_t block address
 * Return Value : FLASH_SUCCESS -
 *                Block Erased successfully.
 *                FLASH_ERR_TIMEOUT
 *                Erase operation timed out. The function issued a STOP to reset the peripheral.
 *                FLASH_ERR_LOCKED
 *                Erase operation timed out and STOP was attempted to recover. Stop operation failed.
 *                Peripheral in locked state.
 *                FLASH_ERR_LOCKBIT_SET
 *                Failed while trying to erase a block because lockbit was set and lockbit protection was enabled
 *                FLASH_ERR_FAILURE
 *                Erase operation failed for some other reason
 * Notes     :   -When using Code Flash in BGO mode, this function must be run from RAM or from the
 *                separate flash block for MCUs that are larger than 2MB. It can be run from ROM if
 *                only a Data Flash operation is ongoing.
 *              - The timeout condition is set to allow for the worst case erasing time as follows
 *                32KB Code Flash 580 ms
 *                8KB Code Flash  145 ms
 *                64B Data Flash 40 ms
 ***********************************************************************************************************************/
flash_err_t flash_erase(uint32_t block_address, uint32_t num_blocks)
{
    flash_err_t err = FLASH_SUCCESS;

    /*Set current operation parameters */
    g_current_parameters.dest_addr = block_address;
    g_current_parameters.total_count = num_blocks;
    g_current_parameters.wait_cnt = WAIT_MAX_ERASE_CF_32K;

    /*Set Erasure Priority Mode*/
    FLASH.FCPSR.WORD = 0x0001;
    for (g_current_parameters.current_count = 0;
      g_current_parameters.current_count    < g_current_parameters.total_count;
      g_current_parameters.current_count++)
    {
        /*Set block start address*/
        FLASH.FSADDR.LONG = g_current_parameters.dest_addr;
        /*Issue two part Block Erase commands*/
        *g_pfcu_cmd_area = (uint8_t) FLASH_FACI_CMD_BLOCK_ERASE;
        *g_pfcu_cmd_area = (uint8_t) FLASH_FACI_CMD_FINAL;

        /*if in BGO mode, exit here; remaining processing if any will be done in ISR*/
        if ((g_current_parameters.current_operation == FLASH_CUR_CF_BGO_ERASE)
        ||  (g_current_parameters.current_operation == FLASH_CUR_DF_BGO_ERASE))
        {
            return err;
        }
        /* Read FRDY bit until it has been set to 1 indicating that the current
         * operation is complete.*/
        while (1 != FLASH.FSTATR.BIT.FRDY)
        {
            /* Wait until FRDY is 1 unless timeout occurs. */
            if (g_current_parameters.wait_cnt-- <= 0)
            {
                /* if FRDY is not set to 1 after max timeout, issue the stop command*/
                err = flash_stop();
                return err;
            }
        }

        /*Check if there were any errors
         * Check if Command Lock bit is set*/
        if (0 != FLASH.FASTAT.BIT.CMDLK)
        {
            if ((FLASH.FSTATR.BIT.ILGCOMERR) || (FLASH.FSTATR.BIT.ILGLERR))
            {
                err = FLASH_ERR_ACCESSW;
            }
            else if (FLASH.FSTATR.BIT.SECERR)
            {
                err = FLASH_ERR_SECURITY;
            }
            else if ((FLASH.FSTATR.BIT.PRGERR) || (FLASH.FSTATR.BIT.ERSERR))
            {
                err = FLASH_ERR_FAILURE;
            }
            else
            {
                err = FLASH_ERR_CMD_LOCKED;
            }
            return err;
        }

        /*If current mode is Data Flash PE, increment to the next block starting address
         * by adding 64 (DF Block Size)
         * Else if the current mode is Code Flash PE, increment to the next block
         * starting address by adding 32K or 8K depending on the current block address
         * Also adjust the timeout depending on the block size*/
        if (FLASH.FENTRYR.WORD == 0x0080)
        {
#ifndef FLASH_NO_DATA_FLASH
            g_current_parameters.dest_addr += FLASH_DF_BLOCK_SIZE;
            g_current_parameters.wait_cnt = WAIT_MAX_ERASE_DF;
#endif
        }
        else if (FLASH.FENTRYR.WORD == 0x0001)
        {
            if (g_current_parameters.dest_addr <= (uint32_t) FLASH_CF_BLOCK_7)
            {
                g_current_parameters.dest_addr -= FLASH_CF_MEDIUM_BLOCK_SIZE;
                g_current_parameters.wait_cnt = WAIT_MAX_ERASE_CF_32K;
            }
            else
            {
                g_current_parameters.dest_addr -= FLASH_CF_SMALL_BLOCK_SIZE;
                g_current_parameters.wait_cnt = WAIT_MAX_ERASE_CF_8K;
            }

        }
        else
        {
            //should never get here
            return FLASH_ERR_FAILURE;
        }
    }

    return err;
}


/***********************************************************************************************************************
 * Function Name: flash_write
 * Description  : Function writes a  of Code or Data Flash
 ** Arguments   : uint32_t block address
 * Return Value : FLASH_SUCCESS -
 *                Block Erased successfully.
 *                FLASH_ERR_TIMEOUT
 *                Erase operation timed out. The function issued a STOP to reset the peripheral.
 *                FLASH_ERR_LOCKED
 *                Erase operation timed out and STOP was attempted to recover. Stop operation failed.
 *                Peripheral in locked state.
 *                FLASH_ERR_LOCKBIT_SET
 *                Failed while trying to write to a block because lockbit was set and lockbit protection was enabled
 *                FLASH_ERR_FAILURE
 *                Erase operation failed for some other reason
 * Notes     :   -When using Code Flash in BGO mode, this function must be run from RAM or from the
 *                separate flash block for MCUs that are larger than 2MB. It can be run from ROM if
 *                only a Data Flash operation is ongoing.
 *               -The timeout condition is dynamically switched to allow for the worst case
 *                erasing time as follows
 *                32KB Code Flash 580 ms
 *                8KB Code Flash  145 ms
 *                64B Data Flash 40 ms
 ***********************************************************************************************************************/
flash_err_t flash_write(uint32_t *src_start_address,
    uint32_t * dest_start_address, uint32_t *num_bytes)
{
    flash_err_t err = FLASH_SUCCESS;
    uint32_t wait_cnt = FLASH_DBFULL_TIMEOUT;

    g_current_parameters.total_count = (*num_bytes) >> 1; //Since two bytes will be written at a time
    g_current_parameters.dest_addr = *dest_start_address;
    g_current_parameters.src_addr = *src_start_address;
    g_current_parameters.current_count = 0;

    /* Iterate through the number of data bytes */
    while (g_current_parameters.total_count > 0)
    {
        /*Set block start address*/
        FLASH.FSADDR.LONG = g_current_parameters.dest_addr;
        /*Issue two part Write commands*/
        *g_pfcu_cmd_area = (uint8_t) FLASH_FACI_CMD_PROGRAM;
        *g_pfcu_cmd_area = (uint8_t) g_current_parameters.min_pgm_size;
        /*Write one line (4 bytes for DF, 256 bytes for CF)*/
        while (g_current_parameters.current_count++ < g_current_parameters.min_pgm_size)
        {
            /* Copy data from source address to destination area */
            *(FCU_WORD_PTR) g_pfcu_cmd_area = *(uint16_t *) g_current_parameters.src_addr;

            while (FLASH.FSTATR.BIT.DBFULL == 1)
            {
                /* Wait until DBFULL is 0 unless timeout occurs. */
                if (wait_cnt-- <= 0)
                {
                    /* if DBFULL is not set to 0 after max timeout, issue the stop command*/
                    err = flash_stop();
                    /*If the STOP was not successfully issued, then return the error value for the stop action
                     * Otherwise return the FLASH_ERR_TIMEOUT error*/
                    if (FLASH_SUCCESS != err)
                    {
                        return err;
                    }
                    else
                    {
                        return FLASH_ERR_TIMEOUT;
                    }
                }
            }
            g_current_parameters.src_addr += 2;
            g_current_parameters.dest_addr += 2;
            g_current_parameters.total_count--;
        }
        /*Reset line count*/
        g_current_parameters.current_count = 0;
        /*Issue write end command*/
        *g_pfcu_cmd_area = (uint8_t) FLASH_FACI_CMD_FINAL;
        /*if in BGO mode, exit here; remaining processing if any will be done in ISR*/
        if ((g_current_parameters.current_operation == FLASH_CUR_CF_BGO_WRITE)
         || (g_current_parameters.current_operation == FLASH_CUR_DF_BGO_WRITE))
        {
            return err;
        }
        /* Read FRDY bit until it has been set to 1 indicating that the current
         * operation is complete.*/
        while (1 != FLASH.FSTATR.BIT.FRDY)
        {
            /* Wait until FRDY is 1 unless timeout occurs. */
            if (g_current_parameters.wait_cnt-- <= 0)
            {
                /* if FRDY is not set to 1 after max timeout, issue the stop command*/
                err = flash_stop();
                return err;
            }
        }

        /*Check if there were any errors
         * Check if Command Lock bit is set*/
        if (0 != FLASH.FASTAT.BIT.CMDLK)
        {
            if ((FLASH.FSTATR.BIT.ILGCOMERR) || (FLASH.FSTATR.BIT.ILGLERR))
            {
                err = FLASH_ERR_ACCESSW;
            }
            else if (FLASH.FSTATR.BIT.SECERR)
            {
                err = FLASH_ERR_SECURITY;
            }
            else if ((FLASH.FSTATR.BIT.PRGERR) || (FLASH.FSTATR.BIT.ERSERR))
            {
                err = FLASH_ERR_FAILURE;
            }
            else
            {
                err = FLASH_ERR_CMD_LOCKED;
            }
            return err;
        }
    }

  return err;
}


/***********************************************************************************************************************
 * Function Name: flash_reset
 * Description  : Function resets the Flash peripheral
 ** Arguments   : None
 * Return Value : FLASH_SUCCESS -
 *                Flash Peripheral successfully reset.
 * Notes     :   -This function will reset the peripheral by stopping any ongoing operations,
 *                clearing the DFAE and CFAE flags and changing the PE mode to Read mode.
 ***********************************************************************************************************************/
flash_err_t flash_reset()
{

    /* Cannot release sequencer from the command-locked state with status clear
     * or forced-stop commands if CFAE or DFAE is set. Must read those bits
     * before can set to 0.
     */
    if (FLASH.FASTAT.BIT.CFAE == 1)
    {
        FLASH.FASTAT.BIT.CFAE = 0;
    }
#ifndef FLASH_NO_DATA_FLASH
    if (FLASH.FASTAT.BIT.DFAE == 1)
    {
        FLASH.FASTAT.BIT.DFAE = 0;
    }
#endif

    /* Possible FLASH_CMD_RESET is called when no outstanding command is in progress.
     * In that case, enter pe mode so flash_stop() can write to the sequencer.
     */
    if (g_flash_state == FLASH_READY)
    {
        flash_pe_mode_enter(FLASH_TYPE_CODE_FLASH);
    }

    /*Issue a forced stop */
    flash_stop();

    /*Transition to Read mode*/
    FLASH.FENTRYR.WORD = 0xAA00;
    while (FLASH.FENTRYR.WORD != 0x0000)
        ;

    return FLASH_SUCCESS;
}


/***********************************************************************************************************************
 * Function Name: Excep_FCU_FRDYI
 * Description  : ISR for Flash Ready Interrupt
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
#pragma interrupt Excep_FCU_FRDYI(vect=VECT(FCU,FRDYI))
static void Excep_FCU_FRDYI(void)
{
    /*Wait counter used for DBFULL flag*/
    uint32_t wait_cnt = FLASH_DBFULL_TIMEOUT;
    if ((FLASH_CUR_DF_BGO_WRITE == g_current_parameters.current_operation)
     || (FLASH_CUR_CF_BGO_WRITE == g_current_parameters.current_operation))
    {
        /*If there are still bytes to write*/
        if (g_current_parameters.total_count > 0)
        {
            /*Set block start address*/
            FLASH.FSADDR.LONG = g_current_parameters.dest_addr;
            /*Issue two part Write commands*/
            *g_pfcu_cmd_area = (uint8_t) FLASH_FACI_CMD_PROGRAM;
            *g_pfcu_cmd_area = (uint8_t) g_current_parameters.min_pgm_size;
            /* Write one line (256 for CF, 4 for DF) */
            while (g_current_parameters.current_count++   < g_current_parameters.min_pgm_size)
            {
                /* Copy data from source address to destination area */
                *(FCU_WORD_PTR) g_pfcu_cmd_area =   *(uint16_t *) g_current_parameters.src_addr;
                while (FLASH.FSTATR.BIT.DBFULL == 1)
                {
                    /* Wait until DBFULL is 0 unless timeout occurs. */
                    if (wait_cnt-- <= 0)
                    {
                        /* if DBFULL is not set to 0 after max timeout, reset thr FCU*/
                        flash_reset();
                        g_flash_int_error_cb_args.event = FLASH_INT_EVENT_ERR_FAILURE;
                        return;
                    }
                }
                g_current_parameters.src_addr += 2;
                g_current_parameters.dest_addr += 2;
                g_current_parameters.total_count--;
            }
            /*Reset line count*/
            g_current_parameters.current_count = 0;
            /*Issue write end command*/
            *g_pfcu_cmd_area = (uint8_t) FLASH_FACI_CMD_FINAL;
            /*Exit ISR until next FRDY interrupt to continue operation*/
            return;
        }
        /*Done writing all bytes*/
        else
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_WRITE_COMPLETE;
        }
    }
    else if ((FLASH_CUR_DF_BGO_ERASE == g_current_parameters.current_operation)
     || (FLASH_CUR_CF_BGO_ERASE == g_current_parameters.current_operation))
    {
        g_current_parameters.current_count++;
        if (g_current_parameters.current_count < g_current_parameters.total_count)
        {
#ifdef FLASH_NO_DATA_FLASH
            if (FLASH.FENTRYR.WORD == 0x0001)
#else
            /*If current mode is Data Flash PE, increment to the next block starting address
             * by adding 64 (DF Block Size) else decrement to the next flash block start address*/
            if (FLASH.FENTRYR.WORD == 0x0080)
            {
                g_current_parameters.dest_addr += FLASH_DF_BLOCK_SIZE;
            }
            else if (FLASH.FENTRYR.WORD == 0x0001)
#endif
            {
                if (g_current_parameters.dest_addr <= (uint32_t) FLASH_CF_BLOCK_7)
                {
                    g_current_parameters.dest_addr -= FLASH_CF_MEDIUM_BLOCK_SIZE;
                }
                else
                {
                    g_current_parameters.dest_addr -= FLASH_CF_SMALL_BLOCK_SIZE;
                }
            }
            /*Set block start address*/
            FLASH.FSADDR.LONG = g_current_parameters.dest_addr;
            /*Issue two part Block Erase commands*/
            *g_pfcu_cmd_area = (uint8_t) FLASH_FACI_CMD_BLOCK_ERASE;
            *g_pfcu_cmd_area = (uint8_t) FLASH_FACI_CMD_FINAL;
            /*Exit ISR until next FRDY interrupt to continue operation*/
            return;
        }
        /*If all blocks are erased*/
        else
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_ERASE_COMPLETE;
        }
    }
#ifndef FLASH_NO_BLANK_CHECK
    else if (FLASH_CUR_DF_BGO_BLANKCHECK == g_current_parameters.current_operation)
    {
        if (FLASH.FBCSTAT.BYTE == 0x01)
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_NOT_BLANK;
        }
        else
        {
            g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_BLANK;
        }

    }
#endif
    else if (FLASH_CUR_CF_ACCESSWINDOW == g_current_parameters.current_operation)
    {
        g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_SET_ACCESSWINDOW;
    }
    else if (FLASH_CUR_CF_TOGGLE_STARTUPAREA == g_current_parameters.current_operation)
    {
        g_flash_int_ready_cb_args.event = FLASH_INT_EVENT_TOGGLE_STARTUPAREA;
    }
    else
    {
        nop();  //un-handled FRDY case
    }

    if (g_current_parameters.current_operation != FLASH_CUR_STOP)
    {
        /*finished current operation. Exit P/E mode*/
        flash_pe_mode_exit();
        /*Release lock and Set current state to Idle*/
        flash_release_state();
        if ((FIT_NO_FUNC != flash_ready_isr_handler)
                && (NULL != flash_ready_isr_handler))
        {
            flash_ready_isr_handler((void *) &g_flash_int_ready_cb_args);
        }
    }
}


/***********************************************************************************************************************
 * Function Name: Excep_FCU_FIFERR
 * Description  : ISR for Flash Error Interrupt
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
#pragma interrupt Excep_FCU_FIFERR(vect=VECT(FCU,FIFERR))
static void Excep_FCU_FIFERR(void)
{
    /*Check if Command Lock bit is set*/
    if (1 == FLASH.FASTAT.BIT.CMDLK)
    {
#ifndef FLASH_NO_DATA_FLASH
        if (1 == FLASH.FASTAT.BIT.DFAE)
        {
            g_flash_int_error_cb_args.event = FLASH_INT_EVENT_ERR_DF_ACCESS;
        }
#endif
        if ((FLASH.FSTATR.BIT.ILGCOMERR) || (FLASH.FSTATR.BIT.ILGLERR))
        {
            g_flash_int_error_cb_args.event = FLASH_INT_EVENT_ERR_CF_ACCESS;    // outside access window
        }
        else if ((1 == FLASH.FASTAT.BIT.CFAE) || (1 == FLASH.FSTATR.BIT.SECERR))
        {
            g_flash_int_error_cb_args.event = FLASH_INT_EVENT_ERR_CF_ACCESS;
        }
        else if ((FLASH.FSTATR.BIT.PRGERR) || (FLASH.FSTATR.BIT.ERSERR))
        {
            g_flash_int_error_cb_args.event = FLASH_INT_EVENT_ERR_FAILURE;
        }
        else
        {
            g_flash_int_error_cb_args.event = FLASH_INT_EVENT_ERR_CMD_LOCKED;
        }
    }
    else
    {
        nop();
    }

    /*Reset the FCU: This will stop any existing processes and exit PE mode*/
    flash_reset();
    IR(FCU,FRDYI)= 0;  //Clear any pending Flash Ready interrupt request
    flash_release_state();
    if ((FIT_NO_FUNC != flash_error_isr_handler)
     && (NULL != flash_error_isr_handler))
    {
        flash_error_isr_handler((void *) &g_flash_int_error_cb_args);
    }

}


#if ((FLASH_CFG_CODE_FLASH_ENABLE == 1) && (FLASH_CFG_CODE_FLASH_BGO == 1))
#pragma section //end FLASH_SECTION_ROM
#endif
#endif
