/*
    文件名称：   ctrl_led.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   LED 操作相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __SMP_LED_H__
#define __SMP_LED_H__



#define LED_R_IO_LEVEL_ON       GPIO_PIN_RESET
#define LED_R_IO_LEVEL_OFF      GPIO_PIN_SET

#define LED_B_IO_LEVEL_ON       GPIO_PIN_RESET
#define LED_B_IO_LEVEL_OFF      GPIO_PIN_SET

#define LED_R_IO_ON             HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, LED_R_IO_LEVEL_ON)
#define LED_R_IO_OFF            HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, LED_R_IO_LEVEL_OFF)
#define LED_R_IO_TOOGLE         HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin)

#define LED_B_IO_ON             HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, LED_B_IO_LEVEL_ON)
#define LED_B_IO_OFF            HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, LED_B_IO_LEVEL_OFF)
#define LED_B_IO_TOOGLE         HAL_GPIO_TogglePin(LED_B_GPIO_Port, LED_B_Pin)

#endif  // __SMP_LED_H__

