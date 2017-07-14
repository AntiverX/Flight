/*
    文件名称：   ctrl_drone_status.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   系统控制状态相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __DRONE_STATUS_H__
#define __DRONE_STATUS_H__

#include "ctrl_basic.h"

#define UNLOCK_SF_MS    (5 * 1000)                      // 解锁安全时间延时
#define UNLOCK_RD_MS    ((5 * 1000) + UNLOCK_SF_MS)     // 解锁动作 左摇杆打到右下角保持多长时间
#define UNLOCK_TM_MS    ((1 * 1000) + UNLOCK_RD_MS)     // 解锁动作 左摇杆回中命令发送多长时间

extern uint32 Drone_Unlock_ms;      // 解锁持续时间计时
extern YesNo_t DroneUnlocking;      // 是否正在解锁

#endif  // __DRONE_STATUS_H__

