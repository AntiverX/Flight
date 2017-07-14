/*
    文件名称：   ctrl_encoder.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   旋转编码器相关功能
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __CTRL_ENCODER_H__
#define __CTRL_ENCODER_H__



#include "ctrl_basic.h"

/*
    For control module, Encoder function:
    1 Select function
        0:  reserved    (Nothing to do)
        
        1:  StartPoint                  - Need RC On and Ch6 control
        2:  StartPoint To EndPoint      - Need RC On and Ch6 control
        3:  StartPoint LeftDown LeftUp  - Need RC On and Ch6 control
        
        4:  StartPoint                  - Need RC Always Off
        5:  StartPoint To EndPoint      - Need RC Always Off
        6:  StartPoint LeftDown LeftUp  - Need RC Always Off
        
        7:
        8:
        9:
        A:
        B:
        C:
        D:
        E:
        F:
*/

#define ENCODER_BIT_8_IO_LEVEL      HAL_GPIO_ReadPin(Encoder_8_GPIO_Port, Encoder_8_Pin)
#define ENCODER_BIT_4_IO_LEVEL      HAL_GPIO_ReadPin(Encoder_4_GPIO_Port, Encoder_4_Pin)
#define ENCODER_BIT_2_IO_LEVEL      HAL_GPIO_ReadPin(Encoder_2_GPIO_Port, Encoder_2_Pin)
#define ENCODER_BIT_1_IO_LEVEL      HAL_GPIO_ReadPin(Encoder_1_GPIO_Port, Encoder_1_Pin)

#define ENCODER_VALUE_RAW   (       ((ENCODER_BIT_8_IO_LEVEL) << 3)     \
                                |   ((ENCODER_BIT_4_IO_LEVEL) << 2)     \
                                |   ((ENCODER_BIT_2_IO_LEVEL) << 1)     \
                                |   ( ENCODER_BIT_1_IO_LEVEL)        )
                                
#define ENCODER_VALUE       ((ENCODER_VALUE_RAW) & 0x000F)

extern void Update_Encoder_Value(void);             // 更新编码器当前值
extern void Update_Encoder_PowerOn_Value(void);     // 更新编码器上电值
extern uint16 Get_Encoder_Value(void);              // 获取编码器当前值
extern uint16 Get_Encoder_PowerOn_Value(void);      // 获取编码器上电初始值

#endif  // __CTRL_ENCODER_H__

