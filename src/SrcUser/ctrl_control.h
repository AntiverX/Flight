/*
    文件名称：   ctrl_control.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   系统控制具体怎么实现的
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __CONTROL_H__
#define __CONTROL_H__

/*
    关于程控功能实现介绍：
    
    1 控制板如何控制飞机动的？
        控制板的串口1和飞控连接，以38400的波特率通信
        按照约定的协议发送数据包，可以实现：
            解锁飞控、改变Roll、Pitch、Yaw、Throttle的控制量
            也可以控制飞行模式
            和使用遥控器对飞机的控制一个道理。
            
    2 控制板从飞控获取数据吗？
        获取
        获取超声波测量的高度、遥控器前6个通道的输入状态、以及飞控解析的姿态 Rol、Pitch、Yaw 值
        
    3 控制板和采集板怎么通信？得到的数据怎么用？
        控制板和采集板通过串口2，以115200的波特率通信
        采集板总是周期性的向控制板发送视觉处理信息，信息包含的内容：
            横滚、俯仰、偏航 三个方向相对于地面路线的 位置、速度，以及能否分析出来这些数据
        控制板得到采集板发送的信息后，更具设定的期望值，采用PID算法求出控制量，然后向飞控发送控制信息
        
    4 Demo 都是怎么实现的？
        Demo 的实现充分采用了状态机的思路，对于路线上每个结点(交叉点、拐点)都认为是一个特殊状态，
        始终判断当前处于哪个状态(结点)，并分析下一步如何移动。
*/

#include "ctrl_pid.h"

#define LOST_CNT_MAX                (4000)          // 特征点丢失后最多追踪多少毫秒
#define FIND_START_POINT_CM_MIN     (50)            // 到达多高(cm)后 进行自动调整
#define FIND_START_POINT_MS_MAX     (1000 * 1000)   // 找起点允许的最大时间
#define FOUND_SHAPE_CACHE_MAX       10              // 找起点的过程中 对每个有效的形状记录的最大数

// 执行时是否需要开启遥控器
typedef enum
{
    DemoCtrl_RC_On = 0,             // 程控时，解锁后打开遥控器才能继续执行任务
    DemoCtrl_RC_Off,                // 程控时，解锁后直接执行程控任务
}DemoCtrl_t;

// 功能演示程序选择
typedef enum
{
    Demo_St = 0,                    // 在起点处 起飞、悬停、降落
    Demo_StToSp,                    // 起飞、向前飞到终点、降落
    Demo_StLdLu,                    // 起飞、向左飞到左下角、向前飞到左上角、降落
    
    Demo_Num,
}Demo_t;

// 控制板发送给飞控的控制命令类型
typedef enum
{
    Auto_Off = 0,                   // 程控关闭(仅受用户遥控器控制 但此时自动降落功能可能无法使用)
    Unlock_RightDown,               // 解锁，将左摇杆打到右下角的动作
    Unlock_ThrMid,                  // 解锁，将油门回中动作
    Auto_On,                        // 程控开启(若此时遥控器打开，依然可以使用遥控器进行控制)
    Lock,                           // 锁定飞机
    
}Msg_CtrlToFMU_Type_t;

// 演示程序：起点处 起飞、悬停、降落 状态机
typedef enum
{
    Demo_St_Act_Climb = 0,          // 爬升到指定高度
    Demo_St_Act_Stable,             // 等待系统稳定
    Demo_St_Act_Hover,              // 悬停
    Demo_St_Act_Land,               // 降落
    Demo_St_Act_Lock,               // 锁定
    
    Demo_St_Act_Num,    
}Demo_St_Act_t;

// 演示程序：起飞、向前飞到终点、降落 状态机
typedef enum
{
    Demo_StToSp_Act_Climb = 0,      // 爬升到指定高度
    Demo_StToSp_Act_Stable,         // 等待系统稳定
    Demo_StToSp_Act_StHover,        // 悬停
    Demo_StToSp_Act_ToSp,           // 前进飞向终点
    Demo_StToSp_Act_SpHover,        // 在终点处悬停
    Demo_StToSp_Act_Land,           // 降落
    Demo_StToSp_Act_Lock,           // 锁定
    
    Demo_StToSp_Act_Num,
}Demo_StToSp_Act_t;

// 演示程序：起飞、向左飞到左下角、向前飞到左上角、降落 状态机
typedef enum
{
    Demo_StLdLu_Act_Climb = 0,      // 爬升到指定高度
    Demo_StLdLu_Act_Stable,         // 等待系统稳定
    Demo_StLdLu_Act_Hover,          // 悬停
    Demo_StLdLu_Act_ToLd,           // 向左飞向左下角
    Demo_StLdLu_Act_LdHover,        // 在左下角悬停
    Demo_StLdLu_Act_ToLu,           // 向前飞向左上角
    Demo_StLdLu_Act_Land,           // 降落
    Demo_StLdLu_Act_Lock,           // 锁定
    
    Demo_StLdLu_Act_Num,
}Demo_StLdLu_Act_t;

// 丢失追踪信息格式
typedef struct
{
    YesNo_t FeatureValid;               // 能否识别到特征点
    YesNo_t RecordValid;                // 是否记录过有效信息
    
    float Record[Dof_Num][DofPID_Num];  // 特征点丢失前的信息记录
    uint32 Lost_ms;                     // 丢失后持续的时间
}Demo_St_LostInfo_t;

// 找不到起点，但是能看到什么
typedef enum
{
    NodeShape_Nothing = 0,      // 设么也看不到
    NodeShape_Horizontal,       // 横线
    NodeShape_Vertical,         // 竖线
    NodeShape_LeftDown,         // 左下角
    NodeShape_RightDown,        // 右下角
    NodeShape_LeftUp,           // 左上角
    NodeShape_RightUp,          // 右上角
    NodeShape_StartPoint,       // 看到起点
    NodeShape_EndPoint,         // 看到终点
    
    NodeShape_Num,
//    Found_Timeout,                      // 不能一直找下去，设定寻找时间上限 FIND_START_POINT_MS_MAX
}NodeShape_t;

extern Demo_St_Act_t Demo_St_Act;
extern Demo_StToSp_Act_t Demo_StToSp_Act;
extern Demo_StLdLu_Act_t Demo_StLdLu_Act;

extern int16 RC_Offset_Rol;
extern int16 RC_Offset_Pit;
extern uint16 HomeHeight_cm;

extern YesNo_t DroneLanding;
extern YesNo_t MsgToFMU_Lock;

extern Demo_t SysDemo;                              // Demo 选择 决定起飞后沿什么样的路线飞行
extern DemoCtrl_t DemoCtrlType;                     // Demo 控制类型(是否要开启遥控器才执行程控任务)
extern Demo_St_LostInfo_t Demo_LostInfo;            // 节点丢失后找回去的信息
extern uint16 Demo_Act_Num;                         // Demo 中状态个数
extern uint16 Demo_Act_Cnt;                         // Demo 现在执行到了那个状态
extern NodeShape_t NodeShape_Curt;                  // 找起点过程中看到的形状(当前形状)
extern NodeShape_t NodeShape_Last;                  // 找起点过程中看到的形状(上刻形状)
extern int32 FindStartPoint_ms_Left;
extern uint16 NodeShapeProb[NodeShape_Num];         // 节点形状概率值 保存各种可能出现的正确的形状的概率值 最大值项就是当前所处的节点形状

extern void SysCtrl_Msg_Ctrl_To_FMU_Init(void);     // 初始化控制板发送给飞控的消息
extern void SysCtrl_Modify_RC_Offset(void);         // 修改遥控器的微调参数值
extern void SysCtrl_Set_Demo_By_Encoder(void);      // 根据编码器的值选定 Demo
extern void SysCtrl_Init_Find_Start_Point(void);    // 找起点功能初始化
extern void SysCtrl_Update_PID(void);               // 更新系统 PID
extern void SysCtrl_Update_PID_Set(void);           // 更新系统 PID 设定值
extern void SysCtrl_Update_PID_Sample(void);        // 更新系统 PID 采样值
extern void SysCtrl_Update_PID_Error(void);         // 更新系统 PID 误差
extern void SysCtrl_Update_PID_Calculate(void);     // 更新系统 PID 运算
extern void SysCtrl_Update_PID_Change(void);        // 修改系统 PID 结果
extern void SysCtrl_Update_Act(void);               // 更新系统 Demo 状态(执行的动作)
extern void SysCtrl_Update_Lost_Info(void);         // 更新特征点丢失追踪信息
extern void SysCtrl_Update_Msg_Ctrl_To_FMU_Type(Msg_CtrlToFMU_Type_t MsgType);  // 控制板向飞控发送特定类型的消息
extern void SysCtrl_Update_Msg_Ctrl_To_FMU(void);   // 控制板向飞控发送消息
extern void SysCtrl_Unlock_FMU(uint32 UnlockLasted_ms);

extern YesNo_t Is_On_Node(YesNo_t F, YesNo_t B, YesNo_t L, YesNo_t R);

#endif  // __CONTROL_H__

