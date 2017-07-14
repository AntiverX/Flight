/*
    文件名称：   ctrl_fmu.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   控制板和飞控相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __CTRL_FMU_H__
#define __CTRL_FMU_H__

#include "ctrl_basic.h"

// 飞控向控制板发送数据的类型
typedef struct
{
    uint16 St;              // 0xAAAA
    
    uint32 ms;              // TimeStamp
    uint32 Seq;             // Sequence id
    
    int16 RC_Ch_01;         // Roll
    int16 RC_Ch_02;         // Pitch
    int16 RC_Ch_03;         // Throttle
    int16 RC_Ch_04;         // Yaw
    int16 RC_Ch_05;         // Mode
    int16 RC_Ch_06;         // AutoControl Switch
    int16 Alt_Sonar;        // Height in cm
    
	int16 AHRS_Rol_1000;    // (AHRS_Rol*1000) AHRS_Rol is float type, but described in radian.
	int16 AHRS_Pit_1000;    // (AHRS_Pit*1000) AHRS_Pit is float type, but described in radian.
	int16 AHRS_Yaw_1000;    // (AHRS_Yaw*1000) AHRS_Yaw is float type, but described in radian.
    
    uint16 Sp;              // 0x5555
}Msg_FMUToCtrl_Pkg_t;

// 飞控向控制板发送数据协议
typedef union
{
    Msg_FMUToCtrl_Pkg_t Pkg;
    uint8 Buf[sizeof(Msg_FMUToCtrl_Pkg_t)];
}Msg_FMUToCtrl_t;

// 控制板向飞控发送数据的格式
typedef struct
{
    uint16 St;      // 0xAAAA
    
    uint32 ms;      // TimeStamp
    uint32 Seq;     // Sequence id
    
    int16 Rol;      // -4500        0       4500
    int16 Pit;      // -4500        0       4500
    int16 Alt;      // 0            550     1000
    int16 Yaw;      // -4500        0       4500
    int16 Mod;      // 
    int16 Cmd;      // 
    
    uint16 Sp;      // 0x5555
}Msg_CtrlToFMU_Pkg_t;

// 控制板向飞控发送数据协议
typedef union
{
    Msg_CtrlToFMU_Pkg_t Pkg;
    uint8 Buf[sizeof(Msg_CtrlToFMU_Pkg_t)];
}Msg_CtrlToFMU_t;

// 控制板向飞控发送的控制命令类型
typedef enum
{
	Cmd_Nothing = 0,    // 非特殊命令 普通控制命令
	Cmd_Unlocking,      // 特殊命令 解锁飞控
}MsgToFMU_Cmd_t;

extern Msg_FMUToCtrl_t Msg_FMUToCtrl;           // 飞控给控制板发送的数据包
extern Msg_CtrlToFMU_t Msg_CtrlToFMU;           // 控制板给飞控发送的数据包

extern YesNo_t En_Update_Alt_Sample;            // 高度可更新标志(高度是飞控发送给控制板的，当控制板接收到了飞控的数据包，才去更新高度采样值)
extern YesNo_t AutoCtrl_RC_Ch6;                 // 飞控下发的数据包中，遥控器第六通道用来开启、关闭程控
extern YesNo_t AutoCtrl_HeightAbove_35cm;       // 高度是否达到了xx厘米，高度太低，镜头捕捉的画面无法识别特征点

extern YesNo_t Msg_FMU_To_Ctrl_Update(void);    // 控制板接收、解析飞控发送的信息

#endif  // __CTRL_FMU_H__

