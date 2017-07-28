/*
    文件名称：   ctrl_smp.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   控制板和采集板通信相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __CTRL_SMP_H__
#define __CTRL_SMP_H__

#include "ctrl_basic.h"


// 采集板向控制板发送数据格式
typedef struct
{
    uint16 St;              // 帧头 固定内容 0xAAAA
    uint32 ms;              // 时间戳 系统启动后运行了多长时间了 time stamp
    uint32 Seq;             // 帧序列号 +1 every time
    uint16 Valid_Rol_Pos:1; // 1    横滚 位置环 是否有效
    uint16 Valid_Rol_Spd:1; // 2    横滚 速度环 是否有效
    uint16 Valid_Pit_Pos:1; // 3    俯仰 位置环 是否有效
    uint16 Valid_Pit_Spd:1; // 4    俯仰 速度环 是否有效
    uint16 Valid_Yaw_Pos:1; // 5    偏航 位置环 是否有效
    uint16 Valid_Yaw_Spd:1; // 6    偏航 速度环 是否有效
    uint16 Valid_F  :1;     // 7    前方 是有有路线
    uint16 Valid_B  :1;     // 8    后方 是否有路线
    uint16 Valid_L  :1;     // 9    左侧 是否有路线
    uint16 Valid_R  :1;     // 10   右侧 是否有路线
    int16 Smp_Rol_Pos;      // 横滚 位置环 采样值
    int16 Smp_Rol_Spd;      // 横滚 速度环 采样值
    int16 Smp_Pit_Pos;      // 俯仰 位置环 采样值
    int16 Smp_Pit_Spd;      // 俯仰 速度环 采样值
    int16 Smp_Yaw_Pos;      // 偏航 位置环 采样值
    int16 Smp_Yaw_Spd;      // 偏航 速度环 采样值
    uint16 Sp;              // 帧尾 固定内容 0x5555
}Msg_SmpToCtrl_Pkg_t;

// 采集板向控制板发送数据协议
typedef union
{
    Msg_SmpToCtrl_Pkg_t Pkg;
    uint8 Buf[sizeof(Msg_SmpToCtrl_Pkg_t)];
}Msg_SmpToCtrl_t;

// 控制板向采集板发送数据格式
typedef struct
{
	uint16 St;              // 帧头 固定内容 0xAAAA
                                                                                                                                                                    
    uint32 ms;              // 时间戳 系统启动后运行多长时间了 time stamp
    uint32 Seq;             // 帧序列号 +1 every time
    
	int16  Alt_Sonar;       // 高度信息
	int16  AHRS_Rol_1000;   // (AHRS_Rol*1000) AHRS_Rol is float type, but described in radian. 姿态 横滚角
	int16  AHRS_Pit_1000;   // (AHRS_Pit*1000) AHRS_Pit is float type, but described in radian. 姿态 俯仰角
	int16  AHRS_Yaw_1000;   // (AHRS_Yaw*1000) AHRS_Yaw is float type, but described in radian. 姿态 偏航角
    
	uint16 Sp;              // 帧尾 固定内容 0x5555
	
}Msg_CtrlToSmp_Pkg_t;

// 控制板向采集板发送数据协议
typedef union
{
    Msg_CtrlToSmp_Pkg_t Pkg;
    uint8 Buf[sizeof(Msg_CtrlToSmp_Pkg_t)];
}Msg_CtrlToSmp_t;

extern Msg_SmpToCtrl_t Msg_SmpToCtrl;           // 采集板发送给控制板的数据
extern Msg_CtrlToSmp_t Msg_CtrlToSmp;           // 控制板发送给采集板的数据

extern YesNo_t En_Update_PID;                   // 系统PID运算更新使能，每当接收到采集板的信息，才更新PID运算
extern YesNo_t En_SendMsgToFMU_WhenSmpUpdate;   // 控制板向飞控发送控制指令使能，每当处理完采集板的信息，才向飞控发送新的控制命令
extern YesNo_t En_SendDbgMsg;                   // 发送调试信息使能，每当处理完采集板信息，才发送调试信息
extern YesNo_t En_Update_Act;                   // 允许更新状态

extern void Msg_Ctrl_To_Smp_Init(void);         // 初始化控制板发送给采集板的信息
extern YesNo_t Msg_Smp_To_Ctrl_Update(void);    // 控制板接收、解析采集板发送的数据包
extern void Msg_Ctrl_To_Smp_Update(void);       // 控制板向采集板发送从飞控获取的高度、姿态

#endif  // __CTRL_SMP_H__

