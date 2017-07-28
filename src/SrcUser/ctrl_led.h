/*
    文件名称：   ctrl_led.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   LED 操作相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __SMP_LED_H__
#define __SMP_LED_H__

#include "r_cg_macrodriver.h"

#define LED_R_IO_LEVEL_ON       GPIO_PIN_RESET
#define LED_R_IO_LEVEL_OFF      GPIO_PIN_SET

#define LED_B_IO_LEVEL_ON       GPIO_PIN_RESET
#define LED_B_IO_LEVEL_OFF      GPIO_PIN_SET

#define LED_R_IO_ON             PORTA.PODR.BIT.B3 = 0
#define LED_R_IO_OFF            PORTA.PODR.BIT.B3 = 1
#define LED_R_IO_TOOGLE         PORTA.PODR.BIT.B3 = ~PORTA.PODR.BIT.B3

#define LED_B_IO_ON             PORT9.PODR.BIT.B4 = 0
#define LED_B_IO_OFF            PORT9.PODR.BIT.B4 = 1
#define LED_B_IO_TOOGLE         PORT9.PODR.BIT.B4 =  ~PORT9.PODR.BIT.B4

#define LED_SIGNAL_IO_ON PORT7.PODR.BIT.B6 = 0
#define LED_SIGNAL_IO_OFF PORT7.PODR.BIT.B6 = 1
#define LED_SIGNAL_IO_TOOGLE PORT7.PODR.BIT.B6 = ~PORT7.PODR.BIT.B6

#endif  // __SMP_LED_H__

