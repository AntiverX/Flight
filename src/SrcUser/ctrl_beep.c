/*
    文件名称：   ctrl_beep.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   蜂鸣器相关操作实现
    修改日期：   2017-7-7
    修改内容：   添加注释
*/

#include "r_cg_macrodriver.h"
#include "ctrl_beep.h"



extern void Beep_On(void)
{
    BEEP_IO_ON;
}

extern void Beep_Off(void)
{
    BEEP_IO_OFF;
}

// 通过单步调试测试蜂鸣器是否正常受控
extern void Beep(void)
{
    Beep_On();
    Beep_Off();
    Beep_On();
    Beep_Off();
}
