/*
    文件名称：   ctrl_drone_status.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   系统控制状态相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#include "ctrl_drone_status.h"

extern uint32 Drone_Unlock_ms = 0;      // 解锁动作持续了多长时间了
extern YesNo_t DroneUnlocking = No;     // 是否正在解锁飞控

