/*
    文件名称：   ctrl_encoder.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   旋转编码器相关功能
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#include "ctrl_encoder.h"
#include "ctrl_basic.h"
#include "r_cg_port.h"
#include "r_cg_macrodriver.h"

// 编码器数据
typedef struct
{
    uint16 PowerOnVal;      // 上电时的值
    uint16 CurrentVal;      // 当前值
}Encoder_t;

static Encoder_t Encoder = {0};

// 更新编码器当前值
extern void Update_Encoder_Value(void)
{
    Encoder.CurrentVal = ((PORT7.PIDR.BIT.B1) << 3)     \
            |   ((PORT7.PIDR.BIT.B0) << 2)     \
            |   ((PORT3.PIDR.BIT.B3) << 1)     \
            |   ( PORT3.PIDR.BIT.B2);
}

// 更新编码器上电初始值
extern void Update_Encoder_PowerOn_Value(void)
{
    Update_Encoder_Value();
    Encoder.PowerOnVal = Encoder.CurrentVal;
}

// 获取编码器当前值
extern uint16 Get_Encoder_Value(void)
{
    return Encoder.CurrentVal;
}

// 获取编码器上电初始值
extern uint16 Get_Encoder_PowerOn_Value(void)
{
    return Encoder.PowerOnVal;
}

