/*
    文件名称：   ctrl_key.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   按键操作相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __CTRL_KEY_H__
#define __CTRL_KEY_H__

/*
    控制板上按键的作用：
    
        若上电时有按键按下，执行遥控器微调参数保存到 Flash 中
        若上电后按下按键，将执行一键启动程控飞行任务
*/


#include "ctrl_basic.h"

#define SW1_IO_LEVEL                HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)
#define SW2_IO_LEVEL                HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)

#define SW1_IO_LEVEL_PRESSED        GPIO_PIN_RESET
#define SW1_IO_LEVEL_RELEASE        GPIO_PIN_SET

#define SW2_IO_LEVEL_PRESSED        GPIO_PIN_RESET
#define SW2_IO_LEVEL_RELEASE        GPIO_PIN_SET

#define IS_SW1_PRESSED              (SW1_IO_LEVEL_PRESSED == SW1_IO_LEVEL)
#define IS_SW1_RELEASED             (SW1_IO_LEVEL_RELEASE == SW1_IO_LEVEL)

#define IS_SW2_PRESSED              (SW2_IO_LEVEL_PRESSED == SW2_IO_LEVEL)
#define IS_SW2_RELEASED             (SW2_IO_LEVEL_RELEASE == SW2_IO_LEVEL)

// 控制板按键识别
typedef enum
{
    SW1,         // 0
    SW2,         // 1
    
    Key_Num,    // 2
}KeyID_t;

// 按键状态
typedef enum
{
    Key_Release = 0,        // 按键状态：被释放
    Key_Pressed,            // 按键状态：被按下
}Key_Status_t;

// 按键数据类型
typedef struct
{
    Key_Status_t PowerOn;    // 上电时的状态
    Key_Status_t Current;    // 当前时刻状态
}Key_t;

// 按键数据(状态)
extern Key_t Key[Key_Num];  // 按键状态数据

extern YesNo_t StartUnlock_By_Key;              // 是否按下一键启动按键

extern void Update_Key_Value(void);             // 更新按键当前状态
extern void Update_Key_PowerOn_Value(void);     // 更新按键上电初始状态
extern void Unlock_FMU_By_Key_Handle(void);     // 一键启动解锁飞控处理

#endif  // __CTRL_KEY_H__

