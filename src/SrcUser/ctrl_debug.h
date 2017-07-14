///*
//    文件名称：   ctrl_debug.h
//    文件作者：   中科浩电 www.bj-zkhd.com
//    文件功能：   通过串口3进行调试相关功能
//    修改日期：   2017-7-7
//    修改内容：   修改注释
//*/
//
//#ifndef __CTRL_DEBUG_H__
//#define __CTRL_DEBUG_H__
//
//#include "ctrl_pid.h"
//
//// 以什么形式查看调试信息
//typedef enum
//{
//    Ascii = 0,      // 字符串
//    Graph,          // 曲线
//}DbgOut_t;
//
//// 调试命令
//typedef enum
//{
//    U3Cmd_Ascii = 0x01,     // 以字符形式输出调试信息
//    U3Cmd_Graph,            // 以曲线形式查看调试信息
//
//    U3Cmd_Rol,              // 调试横滚参数
//    U3Cmd_Pit,              // 调试俯仰参数
//    U3Cmd_Alt,              // 调试高度参数
//    U3Cmd_Yaw,              // 调试偏航参数
//
//    U3Cmd_Pos,              // 调试外环参数
//    U3Cmd_Spd,              // 调试内环参数
//
//    U3Cmd_P,                // 调试比例参数
//    U3Cmd_I,                // 调试积分参数
//    U3Cmd_D,                // 调试微分参数
//
//    U3Cmd_I1,               // 调试项值加 1.00
//    U3Cmd_D1,               // 调试项值减 1.00
//    U3Cmd_I01,              // 调试项值加 0.10
//    U3Cmd_D01,              // 调试项值减 0.10
//    U3Cmd_I001,             // 调试项值加 0.01
//    U3Cmd_D001,             // 调试项值减 0.01
//
//    U3Cmd_Set0,             // 调试项值置零
//    U3Cmd_ShowPID,
//}U3Cmd_t;
//
//typedef struct
//{
//    int16 Height;       // 高度 cm
//    float Rol;          // 横滚角弧度值
//    float Pit;          // 俯仰角弧度值
//    int16 Smp_X;        // 采集板采集到的X位置
//    int16 Smp_Y;        // 采集板采集到的Y位置
//    int16 Cal_X;        // 根据角度补偿计算出来的X位置
//    int16 Cal_Y;        // 根据角度补偿计算出来的Y位置
//}SmpPosWithAngle_t;
//
//extern volatile Dof_t Dbg_Dof;                          // 调试项 目前在调试 横滚、俯仰、偏航、高度 哪一项
//extern volatile DofPID_t Dbg_PID;                       // 调试项 目前在调试 内环、外环 哪一项
//extern volatile DofPIDPara_t Dbg_Para;                  // 调试项 目前在调试 比例、积分、微分 哪一项
//extern DbgOut_t DbgOutType;                              // 调试信息输出方式 可以按照字符或曲线两种方式进行
//extern float PidPara[Dof_Num][DofPID_Num][Para_Num];    // 调试参数缓存
//
//extern void Dbg_PID_Cache_Init(void);       // 调试参数缓存初始化
//extern void Debug_Message_Handle(void);     // 调试信息处理
//extern void Dbg_U3_Send_Msg(void);          // 串口3发送调试信息
//
//#endif  // __CTRL_DEBUG_H__
//
