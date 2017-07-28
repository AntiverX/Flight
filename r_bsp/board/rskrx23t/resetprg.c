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
* Copyright (C) 2015 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : resetprg.c
* Device(s)    : RX23T
* Description  : Defines post-reset routines that are used to configure the MCU prior to the main program starting. 
*                This is were the program counter starts on power-up or reset.
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version  Description
*         : 30.09.2015 1.00     First Release
*         : 30.09.2015 1.01     Changed Minor version to 3.01
*         : 13.04.2016 1.02     Added the setting of High Speed On-Chip Oscillator (HOCO).
*                               Changed the method of setting the following registers.
*                                - Main Clock Oscillator Forced Oscillation Control Register (MOFCR.MOSEL)
*                                - Main Clock Oscillator Wait Control Register (MOSCWTCR)
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Defines MCU configuration functions used in this file */
#include    <_h_c_lib.h>
/* This macro is here so that the stack will be declared here. This is used to prevent multiplication of stack size. */
#define     BSP_DECLARE_STACK
/* Define the target platform */
#include    "platform.h"

/* BCH - 01/16/2013 */
/* 0602: Defect for macro names with '_[A-Z]' is also being suppressed since these are default names from toolchain.
   3447: External linkage is not needed for these toolchain supplied library functions. */
/* PRQA S 0602, 3447 ++ */

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/* If the user chooses only 1 stack then the 'U' bit will not be set and the CPU will always use the interrupt stack. */
#if (BSP_CFG_USER_STACK_ENABLE == 1)
    #define PSW_init  (0x00030000)
#else
    #define PSW_init  (0x00010000)
#endif
#define FPSW_init (0x00000000)  /* Currently nothing set by default. */

/***********************************************************************************************************************
Pre-processor Directives
***********************************************************************************************************************/
/* Set this as the entry point from a power-on reset */
#pragma entry PowerON_Reset_PC

/***********************************************************************************************************************
External function Prototypes
***********************************************************************************************************************/
/* Functions to setup I/O library */
extern void _INIT_IOLIB(void);
extern void _CLOSEALL(void);

#if BSP_CFG_USER_WARM_START_CALLBACK_PRE_INITC_ENABLED != 0
/* If user is requesting warm start callback functions then these are the prototypes. */
void BSP_CFG_USER_WARM_START_PRE_C_FUNCTION(void);
#endif

#if BSP_CFG_USER_WARM_START_CALLBACK_POST_INITC_ENABLED != 0
/* If user is requesting warm start callback functions then these are the prototypes. */
void BSP_CFG_USER_WARM_START_POST_C_FUNCTION(void);
#endif

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
/* Power-on reset function declaration */
void PowerON_Reset_PC(void);

#if BSP_CFG_RUN_IN_USER_MODE==1
    #if __RENESAS_VERSION__ < 0x01010000
        /* Declare the contents of the function 'Change_PSW_PM_to_UserMode' as assembler to the compiler */
        #pragma inline_asm Change_PSW_PM_to_UserMode

    /* MCU user mode switcher function declaration */
    static void Change_PSW_PM_to_UserMode(void);
    #endif
#endif

/* Main program function declaration */
void main(void);
static void operating_frequency_set(void);
static void clock_source_select(void);
static void usb_lpc_clock_source_select(void);

/***********************************************************************************************************************
* Function name: PowerON_Reset_PC
* Description  : This function is the MCU's entry point from a power-on reset.
*                The following steps are taken in the startup code:
*                1. The User Stack Pointer (USP) and Interrupt Stack Pointer (ISP) are both set immediately after entry 
*                   to this function. The USP and ISP stack sizes are set in the file bsp_config.h.
*                   Default sizes are USP=4K and ISP=1K.
*                2. The interrupt vector base register is set to point to the beginning of the relocatable interrupt 
*                   vector table.
*                3. The MCU operating frequency is set by configuring the Clock Generation Circuit (CGC) in
*                   operating_frequency_set.
*                4. Calls are made to functions to setup the C runtime environment which involves initializing all 
*                   initialed data, zeroing all uninitialized variables, and configuring STDIO if used
*                   (calls to _INITSCT and _INIT_IOLIB).
*                5. Board-specific hardware setup, including configuring I/O pins on the MCU, in hardware_setup.
*                6. Global interrupts are enabled by setting the I bit in the Program Status Word (PSW), and the stack 
*                   is switched from the ISP to the USP.  The initial Interrupt Priority Level is set to zero, enabling 
*                   any interrupts with a priority greater than zero to be serviced.
*                7. The processor is optionally switched to user mode.  To run in user mode, set the macro 
*                   BSP_CFG_RUN_IN_USER_MODE above to a 1.
*                8. The bus error interrupt is enabled to catch any accesses to invalid or reserved areas of memory.
*
*                Once this initialization is complete, the user's main() function is called.  It should not return.
* Arguments    : none
* Return value : none
***********************************************************************************************************************/
void PowerON_Reset_PC(void)
{
    /* Stack pointers are setup prior to calling this function - see comments above */    
    
    /* Initialise the MCU processor word and Exception Table Register */
#if __RENESAS_VERSION__ >= 0x01010000    
    set_intb((void *)__sectop("C$VECT"));
    set_extb((void *)__sectop("EXCEPTVECT"));
#else
    set_intb((unsigned long)__sectop("C$VECT"));
    set_extb((unsigned long)__sectop("EXCEPTVECT"));
#endif    


    /* Initialize FPSW for floating-point operations */
#ifdef __ROZ
#define FPU_ROUND 0x00000001  /* Let FPSW RMbits=01 (round to zero) */
#else
#define FPU_ROUND 0x00000000  /* Let FPSW RMbits=00 (round to nearest) */
#endif
#ifdef __DOFF
#define FPU_DENOM 0x00000100  /* Let FPSW DNbit=1 (denormal as zero) */
#else
#define FPU_DENOM 0x00000000  /* Let FPSW DNbit=0 (denormal as is) */
#endif

    set_fpsw(FPSW_init | FPU_ROUND | FPU_DENOM);
    
    /* Switch to high-speed operation */
    operating_frequency_set();
    usb_lpc_clock_source_select();

    /* If the warm start Pre C runtime callback is enabled, then call it. */
#if BSP_CFG_USER_WARM_START_CALLBACK_PRE_INITC_ENABLED == 1
     BSP_CFG_USER_WARM_START_PRE_C_FUNCTION();
#endif

    /* Initialize C runtime environment */
    _INITSCT();

    /* If the warm start Post C runtime callback is enabled, then call it. */
#if BSP_CFG_USER_WARM_START_CALLBACK_POST_INITC_ENABLED == 1
     BSP_CFG_USER_WARM_START_POST_C_FUNCTION();
#endif

#if BSP_CFG_IO_LIB_ENABLE == 1
    /* Comment this out if not using I/O lib */
    _INIT_IOLIB();
#endif

//    /* Initialize MCU interrupt callbacks. */
//    bsp_interrupt_open();
//
//    /* Initialize register protection functionality. */
//    bsp_register_protect_open();
//
//    /* Configure the MCU and board hardware */
//    hardware_setup();

    HardwareSetup();
    /* Change the MCU's user mode from supervisor to user */
    nop();
    set_psw(PSW_init);      
#if BSP_CFG_RUN_IN_USER_MODE==1
    /* Use chg_pmusr() intrinsic if possible. */
    #if __RENESAS_VERSION__ >= 0x01010000
    chg_pmusr() ;
    #else
    Change_PSW_PM_to_UserMode();
    #endif
#endif

    /* Enable the bus error interrupt to catch accesses to illegal/reserved areas of memory */
    R_BSP_InterruptControl(BSP_INT_SRC_BUS_ERROR, BSP_INT_CMD_INTERRUPT_ENABLE, FIT_NO_PTR);

    /* Call the main program function (should not return) */
    main();
    
#if BSP_CFG_IO_LIB_ENABLE == 1
    /* Comment this out if not using I/O lib - cleans up open files */
    _CLOSEALL();
#endif

    /* BCH - 01/16/2013 */
    /* Infinite loop is intended here. */    
    while(1) /* PRQA S 2740 */
    {
        /* Infinite loop. Put a breakpoint here if you want to catch an exit of main(). */
    }
}



/***********************************************************************************************************************
* Function name: operating_frequency_set
* Description  : Configures the clock settings for each of the device clocks
* Arguments    : none
* Return value : none
***********************************************************************************************************************/
static void operating_frequency_set (void)
{
    /* Used for constructing value to write to SCKCR register. */
    uint32_t temp_clock = 0;
    
    /* 
    Default settings:
    Clock Description              Frequency
    ----------------------------------------
    Input Clock Frequency............  10 MHz
    PLL frequency (/2  x8)............ 40 MHz
    Internal Clock Frequency.........  40 MHz
    Peripheral Clock A Frequency...... 40 MHz
    Peripheral Clock B Frequency...... 40 MHz
    Peripheral Clock D Frequency...... 40 MHz
    Flash IF Clock Frequency.......... 20 MHz
    */
            
    /* Protect off. */
    SYSTEM.PRCR.WORD = 0xA50F;

    /* Select the clock based upon user's choice. */
    clock_source_select();

    /* Figure out setting for FCK bits. */
#if   BSP_CFG_FCK_DIV == 1
    /* Do nothing since FCK bits should be 0. */
#elif BSP_CFG_FCK_DIV == 2
    temp_clock |= 0x10000000;
#elif BSP_CFG_FCK_DIV == 4
    temp_clock |= 0x20000000;
#elif BSP_CFG_FCK_DIV == 8
    temp_clock |= 0x30000000;
#elif BSP_CFG_FCK_DIV == 16
    temp_clock |= 0x40000000;
#elif BSP_CFG_FCK_DIV == 32
    temp_clock |= 0x50000000;
#elif BSP_CFG_FCK_DIV == 64
    temp_clock |= 0x60000000;
#else
    #error "Error! Invalid setting for BSP_CFG_FCK_DIV in r_bsp_config.h"
#endif

    /* Figure out setting for ICK bits. */
#if   BSP_CFG_ICK_DIV == 1
    /* Do nothing since ICK bits should be 0. */
#elif BSP_CFG_ICK_DIV == 2
    temp_clock |= 0x01000000;
#elif BSP_CFG_ICK_DIV == 4
    temp_clock |= 0x02000000;
#elif BSP_CFG_ICK_DIV == 8
    temp_clock |= 0x03000000;
#elif BSP_CFG_ICK_DIV == 16
    temp_clock |= 0x04000000;
#elif BSP_CFG_ICK_DIV == 32
    temp_clock |= 0x05000000;
#elif BSP_CFG_ICK_DIV == 64
    temp_clock |= 0x06000000;
#else
    #error "Error! Invalid setting for BSP_CFG_ICK_DIV in r_bsp_config.h"
#endif

    /* Figure out setting for PCKA bits. */
#if   BSP_CFG_PCKA_DIV == 1
    /* Do nothing since PCKA bits should be 0. */
#elif BSP_CFG_PCKA_DIV == 2
    temp_clock |= 0x00001000;
#elif BSP_CFG_PCKA_DIV == 4
    temp_clock |= 0x00002000;
#elif BSP_CFG_PCKA_DIV == 8
    temp_clock |= 0x00003000;
#elif BSP_CFG_PCKA_DIV == 16
    temp_clock |= 0x00004000;
#elif BSP_CFG_PCKA_DIV == 32
    temp_clock |= 0x00005000;
#elif BSP_CFG_PCKA_DIV == 64
    temp_clock |= 0x00006000;
#else
    #error "Error! Invalid setting for BSP_CFG_PCKA_DIV in r_bsp_config.h"
#endif

    /* Figure out setting for PCKB bits. */
#if   BSP_CFG_PCKB_DIV == 1
    /* Do nothing since PCKB bits should be 0. */
#elif BSP_CFG_PCKB_DIV == 2
    temp_clock |= 0x00000100;
#elif BSP_CFG_PCKB_DIV == 4
    temp_clock |= 0x00000200;
#elif BSP_CFG_PCKB_DIV == 8
    temp_clock |= 0x00000300;
#elif BSP_CFG_PCKB_DIV == 16
    temp_clock |= 0x00000400;
#elif BSP_CFG_PCKB_DIV == 32
    temp_clock |= 0x00000500;
#elif BSP_CFG_PCKB_DIV == 64
    temp_clock |= 0x00000600;
#else
    #error "Error! Invalid setting for BSP_CFG_PCKB_DIV in r_bsp_config.h"
#endif

    /* Figure out setting for PCKD bits. */
#if   BSP_CFG_PCKD_DIV == 1
    /* Do nothing since PCKD bits should be 0. */
#elif BSP_CFG_PCKD_DIV == 2
    temp_clock |= 0x00000001;
#elif BSP_CFG_PCKD_DIV == 4
    temp_clock |= 0x00000002;
#elif BSP_CFG_PCKD_DIV == 8
    temp_clock |= 0x00000003;
#elif BSP_CFG_PCKD_DIV == 16
    temp_clock |= 0x00000004;
#elif BSP_CFG_PCKD_DIV == 32
    temp_clock |= 0x00000005;
#elif BSP_CFG_PCKD_DIV == 64
    temp_clock |= 0x00000006;
#else
    #error "Error! Invalid setting for BSP_CFG_PCKD_DIV in r_bsp_config.h"
#endif

    /* -------------------------------------------------------------------
       SCKCR registor b19-b16 bshould be set biggest value of ICK/PCKB
     ----------------------------------------------------------------------*/
#if (BSP_CFG_ICK_DIV > BSP_CFG_PCKB_DIV )
  #if   BSP_CFG_ICK_DIV == 1
  #elif BSP_CFG_ICK_DIV == 2
      temp_clock |= 0x00010000;
  #elif BSP_CFG_ICK_DIV == 4
      temp_clock |= 0x00020000;
  #elif BSP_CFG_ICK_DIV == 8
      temp_clock |= 0x00030000;
  #elif BSP_CFG_ICK_DIV == 16
      temp_clock |= 0x00040000;
  #elif BSP_CFG_ICK_DIV == 32
      temp_clock |= 0x00050000;
  #elif BSP_CFG_ICK_DIV == 64
      temp_clock |= 0x00060000;
  #else
      #error "Error! Invalid setting for BSP_CFG_ICK_DIV in r_bsp_config.h"  
  #endif
#else
  #if   BSP_CFG_PCKB_DIV == 1
  #elif BSP_CFG_PCKB_DIV == 2
      temp_clock |= 0x00010000;
  #elif BSP_CFG_PCKB_DIV == 4
      temp_clock |= 0x00020000;
  #elif BSP_CFG_PCKB_DIV == 8
      temp_clock |= 0x00030000;
  #elif BSP_CFG_PCKB_DIV == 16
      temp_clock |= 0x00040000;
  #elif BSP_CFG_PCKB_DIV == 32
      temp_clock |= 0x00050000;
  #elif BSP_CFG_PCKB_DIV == 64
      temp_clock |= 0x00060000;
  #else
      #error "Error! Invalid setting for BSP_CFG_PCKB_DIV in r_bsp_config.h" 
  #endif
#endif


    /* b7 to b4 should be 0x0.
       b22 to b20 should be 0x0. */

    /* Set SCKCR register. */
    SYSTEM.SCKCR.LONG = temp_clock;

    while (SYSTEM.SCKCR.LONG != temp_clock)
    {
        /* Confirm that previous write to SCKCR register has completed. RX MCU's have a 5 stage pipeline architecture
         * and therefore new instructions can be executed before the last instruction has completed. By reading the
         * value of register we will ensure that the value has been written. Not all registers require this
         * verification but the SCKCR is marked as such in the HW manual.
         */
    }

    /* Actually set selected clock source here. Default for r_bsp_config.h is PLL. */
    SYSTEM.SCKCR3.WORD = ((uint16_t)BSP_CFG_CLOCK_SOURCE) << 8;

#if (BSP_CFG_CLOCK_SOURCE != 0)
    /* We can now turn LOCO off since it is not going to be used. */
    SYSTEM.LOCOCR.BYTE = 0x01;
#endif

    /* Protect on. */
    SYSTEM.PRCR.WORD = 0xA500;          
}

/***********************************************************************************************************************
* Function name: clock_source_select
* Description  : Enables and disables clocks as chosen by the user.
* Arguments    : none
* Return value : none
***********************************************************************************************************************/
static void clock_source_select (void)
{
    volatile uint8_t read_verify;
     /* Declared volatile for software delay purposes. */
    volatile uint32_t i;

    /* Set to High-speed operating mode if ICLK is > 12MHz. */
    if( BSP_ICLK_HZ  > BSP_MIDDLE_SPEED_MAX_FREQUENCY )
    {
        SYSTEM.OPCCR.BYTE = 0x00;   // set to high-speed mode
        while(SYSTEM.OPCCR.BIT.OPCMTSF == 1)
        {
            /* Wait for transition to finish. */
        }
    }
    /* Set to memory wait if ICLK is > 32MHz. */
    if( BSP_ICLK_HZ > BSP_MEMORY_NO_WAIT_MAX_FREQUENCY )
    {
        SYSTEM.MEMWAIT.BYTE = 0x01; // Use wait states
        while (SYSTEM.MEMWAIT.BYTE != 0x01)
        {
            /* wait for bit to set */
        }
    }

    /* Configure the main clock oscillator drive capability. */
    if (BSP_CFG_XTAL_HZ >= 10000000)
    {
        SYSTEM.MOFCR.BIT.MODRV21 = 1;  // BSP_CFG_XTAL_HZ >= 10 MHz  Uses 10MHz external oscillator
    }
    else  //  BSP_CFG_XTAL_HZ < 10 MHz
    {
        SYSTEM.MOFCR.BIT.MODRV21 = 0;  // 
    }

    /* At this time the MCU is still running on the 4MHz LOCO. */

#if (BSP_CFG_CLOCK_SOURCE == 1)
    /* HOCO is chosen. Start it operating if it is not already operating. */
    if (1 == SYSTEM.HOCOCR.BIT.HCSTP)
    {
        while (0 != SYSTEM.OSCOVFSR.BIT.HCOVF)
        {
            /* wait for bit to set */
        }

        /* Set the oscillation stabilization wait time of the HOCO. */
        SYSTEM.HOCOWTCR.BYTE = BSP_CFG_HOCO_WAIT_TIME;

        /* Start HOCO operating. */
        SYSTEM.HOCOCR.BYTE = 0x00;

        while (0 == SYSTEM.OSCOVFSR.BIT.HCOVF)
        {
            /* Make sure clock has stabilized. */
        }
    }
#else
    /* HOCO is not chosen. If HOCO is already operating, it doesn't stop.  */
#endif


#if (BSP_CFG_CLOCK_SOURCE == 2) || (BSP_CFG_CLOCK_SOURCE == 4) // MAIN or PLL

    /* Set the oscillation source of the main clock oscillator. */
    SYSTEM.MOFCR.BIT.MOSEL = BSP_CFG_MAIN_CLOCK_SOURCE;

    #if   (BSP_CFG_MAIN_CLOCK_SOURCE == 0) /* Resonator */
        /* Set the oscillation stabilization wait time of the main clock oscillator. */
        SYSTEM.MOSCWTCR.BYTE =  BSP_CFG_MOSC_WAIT_TIME;
    #elif (BSP_CFG_MAIN_CLOCK_SOURCE == 1) /* External oscillator input */
        SYSTEM.MOSCWTCR.BYTE = 0x00;
    #else
        #error "Error! Invalid setting for BSP_CFG_MAIN_CLOCK_SOURCE in r_bsp_config.h"
    #endif

    /* Set the main clock to operating. */
    SYSTEM.MOSCCR.BYTE = 0x00;          
    while (SYSTEM.OSCOVFSR.BIT.MOOVF != 1)
    {
        ;   // wait for stabilization
    }
#else
    /* Set the main clock to stopped. */
    SYSTEM.MOSCCR.BYTE = 0x01;          
#endif


#if (BSP_CFG_CLOCK_SOURCE == 4) // PLL

    /* Set PLL Input Divisor. */
    SYSTEM.PLLCR.BIT.PLIDIV = BSP_CFG_PLL_DIV >> 1;

    /* Set PLL Multiplier. */
    SYSTEM.PLLCR.BIT.STC = ((uint8_t)((float)BSP_CFG_PLL_MUL * 2.0)) - 1;

    /* Set the PLL to operating. */
    SYSTEM.PLLCR2.BYTE = 0x00;          
    while (SYSTEM.OSCOVFSR.BIT.PLOVF != 1)
    {
        ;   // wait for stabilization
    }
#else
    /* Set the PLL to stopped. */
    SYSTEM.PLLCR2.BYTE = 0x01;          
#endif

    /* LOCO is saved for last since it is what is running by default out of reset. This means you do not want to turn
       it off until another clock has been enabled and is ready to use. */
#if (BSP_CFG_CLOCK_SOURCE == 0)
    /* LOCO is chosen. This is the default out of reset. */
    SYSTEM.LOCOCR.BYTE = 0x00;
#else
    /* LOCO is not chosen but it cannot be turned off yet since it is still being used. */
#endif

    /* Make sure a valid clock was chosen. */
#if (BSP_CFG_CLOCK_SOURCE > 4) || (BSP_CFG_CLOCK_SOURCE < 0) || (BSP_CFG_CLOCK_SOURCE == 3)
    #error "Error! Invalid setting for BSP_CFG_CLOCK_SOURCE in r_bsp_config.h"
#endif 
}


/***********************************************************************************************************************
* Function name: usb_lpc_clock_source_select
* Description  : Enables clock sources for the usb and lpc (if not already done) as chosen by the user.
*                This function also implements the software delays needed for the clocks to stabilize.
* Arguments    : none
* Return value : none
***********************************************************************************************************************/
static void usb_lpc_clock_source_select (void)
{
    /* Declared volatile for software delay purposes. */
    volatile uint32_t i;


    /* Protect off. DO NOT USE R_BSP_RegisterProtectDisable()! (not initialized yet) */
    SYSTEM.PRCR.WORD = 0xA50F;



    /* INITIALIZE AND SELECT LPC CLOCK SOURCE */

    SYSTEM.PRCR.WORD = 0xA500;
    return;
}

/***********************************************************************************************************************
* Function name: Change_PSW_PM_to_UserMode
* Description  : Assembler function, used to change the MCU's usermode from supervisor to user.
* Arguments    : none
* Return value : none
***********************************************************************************************************************/
#if BSP_CFG_RUN_IN_USER_MODE==1
    #if __RENESAS_VERSION__ < 0x01010000
static void Change_PSW_PM_to_UserMode(void)
{
    MVFC   PSW,R1
    OR     #00100000h,R1
    PUSH.L R1
    MVFC   PC,R1
    ADD    #10,R1
    PUSH.L R1
    RTE
    NOP
    NOP
}
    #endif
#endif

