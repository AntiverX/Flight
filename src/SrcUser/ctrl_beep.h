/*
    文件名称：   ctrl_beep.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   蜂鸣器相关操作实现
    修改日期：   2017-7-7
    修改内容：   添加注释
*/
//----------------------------------Modified Here.-----------------------------------------//
#ifndef __SMP_BEEP_H__
#define __SMP_BEEP_H__

#define BEEP_IO_ON              PORTB.PODR.BIT.B5 = 0
#define BEEP_IO_OFF             PORTB.PODR.BIT.B5 = 1
#define BEEP_IO_TOOGLE          PORTB.PODR.BIT.B5 = ~PORTB.PODR.BIT.B5
extern void Beep_On(void);      // 开启蜂鸣器
extern void Beep_Off(void);     // 关闭蜂鸣器
extern void Beep(void);    // 蜂鸣器功能测试

#endif  // __SMP_BEEP_H__

