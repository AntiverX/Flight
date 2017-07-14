/*
    文件名称：   ctrl_key.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   按键操作相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#include "ctrl_key.h"
#include "ctrl_fmu.h"
#include "ctrl_control.h"
#include "r_cg_macrodriver.h"
#include "r_cg_port.h"

extern Key_t Key[Key_Num] = {Key_Release};

extern YesNo_t StartUnlock_By_Key = No;

// 更新按键当前状态
extern void Update_Key_Value(void)
{
    if(PORTB.PIDR.BIT.B6==0)
    {
        Key[SW1].Current = Key_Pressed;
    }
    else if(PORTB.PIDR.BIT.B6==1)
    {
        Key[SW1].Current = Key_Release;
    }

    if(PORTB.PIDR.BIT.B7==0)
    {
        Key[SW2].Current = Key_Pressed;
    }
    else if(PORTB.PIDR.BIT.B7==1)
    {
        Key[SW2].Current = Key_Release;
    }
}

// 更新按键上电初始状态
extern void Update_Key_PowerOn_Value(void)
{
    Update_Key_Value();

    Key[SW1].PowerOn = Key[SW1].Current;
    Key[SW2].PowerOn = Key[SW2].Current;
}

// 检测是否有按键按下(一键启动)
extern void Unlock_FMU_By_Key_Handle(void)
{
    uint16 HomeHeightNew_cm;

    /*
        控制板上有两个按键，都可以用来一键启动，区别是：

            SW1(控制板上 左边的按键) 按下，解锁飞控后，等待遥控器开启才执行程控任务
            SW2(控制板上 右边的按键) 按下，解锁飞控后，直接执行程控任务
    */

    // 当有按键按下
    if((Key_Pressed==Key[SW1].Current) || (Key_Pressed==Key[SW2].Current))
    {
        StartUnlock_By_Key = Yes;                       // 进入解锁状态

        HomeHeightNew_cm = Get_Height();                // 获取当前的高度(适应不同机型 通过按键解锁时 测量的高度就是Home高度值)
        if(     (HomeHeightNew_cm > 1)                  // Home高度防止出错(默认高度小于40cm)
            &&  (HomeHeightNew_cm < 40)
            &&  (0 != Msg_FMUToCtrl.Pkg.RC_Ch_05)   )   // 如果此时遥控器的第五通道(模式)值为0，则说明遥控器是关闭的，那么程控才可以解锁飞控
        {
            HomeHeight_cm = HomeHeightNew_cm;           // 更新 Home 高度
        }

        if(Key_Pressed == Key[SW1].Current)             // 通过 SW1 一键启动
        {
            DemoCtrlType = DemoCtrl_RC_On;              // 需要开启遥控器才执行程控任务
        }
        else if(Key_Pressed == Key[SW2].Current)        // 通过 SW2 一键启动
        {
            DemoCtrlType = DemoCtrl_RC_Off;             // 默认遥控器关闭情况下直接执行程控任务
            SysCtrl_Modify_RC_Offset();
        }

        SysCtrl_Set_Demo_By_Encoder();                  // 根据编码器的值，设定执行哪个Demo
    }
    else
        StartUnlock_By_Key = No;
}

