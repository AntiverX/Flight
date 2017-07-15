/*
    文件名称：   ctrl_pid.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   系统控制PID相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __PID_H__
#define __PID_H__

#include "ctrl_basic.h"

#define SAMPLE_CACHE_MAX  10

// 控制项(维度)
typedef enum
{
    Rol = 0,
    Pit,
    Alt,
    Yaw,
    
    Dof_Num,
}Dof_t;

// 串级PID的内外环
typedef enum
{
    Pos = 0,    // 位置环(外环)
    Spd,        // 速度环(内环)
    
    DofPID_Num,
}DofPID_t;

// 比例、积分、微分
typedef enum
{
    P = 0,
	I,
	D,
    Para_Num,
}DofPIDPara_t;

// 单级 PID 数据结构
typedef struct
{
    float p;                                // 比例控制系数
    float i;                                // 积分控制系数
    float d;                                // 微分控制系数
                        
    YesNo_t valid;                          // 本次采样是否有效
    YesNo_t valid_last;                     // 上次采样是否右下
    
    float sample;                           // 采样值
    float sample_last;                      // 上次采样值
    float sample_cache[SAMPLE_CACHE_MAX];   // 采样缓存
    uint16 sample_cache_cnt;                // 采样缓存计数
    uint16 sample_cache_max;                // 采样缓存个数上限
    float set;                              // PID 控制设定值(目标值)
    float set_min;                          // PID 控制 设定值的最小允许值
    float set_max;                          // PID 控制 是定制的最大允许值
    float err;                              // PID 控制 误差值
    float err_abs;                          // PID 控制 误差的绝对值
    float err_last;                         // PID 控制 上次的误差值
    float err_p;                            // PID 控制 比例误差
    float err_i;                            // PID 控制 积分误差
    float err_i_abs_max;                    // PID 控制 积分上限
    float err_d;                            // PID 控制 微分误差值
    float i_act_err_abs_max;                // PID 控制 积分分离控制
                
    float out;                              // PID 控制 输出值
    float out_p;                            // PID 控制 比例输出
    float out_i;                            // PID 控制 积分输出
    float out_i_min;                        // PID 控制 积分输出限幅最小值
    float out_i_max;                        // PID 控制 积分输出限幅最大值
    float out_d;                            // PID 控制 微分输出
}PID_t;

// 串级 PID 数据结构
typedef struct
{
    float Out_Float;                        // 控制项输出值(浮点型)
    int16 Out_Int16;                        // 控制项输出值(整型)
                        
    float Out_Min;                          // 控制项输出限幅 最小值
    float Out_Max;                          // 控制项输出限幅 最大值
                
    float K_PosErr_To_SpdSet;               // 控制项串级 PID 控制，内外环传递系数
                
    PID_t PID[DofPID_Num];                  // 内外环单级数据
}Dof_PID_t;         
            
// 单级 PID 采样数据
typedef struct          
{           
    float sample;                           // 本次采样值
    float sample_last;                      // 上次采样值
    YesNo_t valid;                          // 本次采样是否有效
    YesNo_t valid_last;                     // 上次采样是否有效
}PID_Sample_t;

// 串级 PID 采样数据
typedef struct
{
    PID_Sample_t PosSmp;
    PID_Sample_t SpdSmp;
}Dof_PID_Sample_t;

extern Dof_PID_Sample_t Dof_Alt_Smp;        // 高度控制项采样数据
extern Dof_PID_t SysPID[Dof_Num];           // 系统各控制项 PID 数据


extern void PID_Parameter_Init(void);                       // 系统控制参数初始化
extern void PID_Update_Alt_Sample(float NewAltPos_Raw);     // 更新高度采样
extern uint16 Get_Height(void);                             // 获取当前高度
extern int16 Dof_PID_Update_Calculate(Dof_PID_t* pDof_PID); // 系统控制 PID 输出更新
extern int16 Dof_PID_Update(Dof_PID_t* pDof_PID, float PosSet, float PosSmp, YesNo_t PosSmp_Valid, float SpdSet, float SpdSmp, YesNo_t SpdSmp_Valid);   // 更新指定的PID项
extern YesNo_t Is_Alt_Valid(uint16 Alt_cm);                 // 判断高度值是否合理

extern void PID_Update_Sample(PID_t* pPID, float Sample_NewRaw, YesNo_t Sample_Valid);  // 更新 PID 采样值
extern void PID_Update_Set(PID_t* pPID, float Set_New);                                  // 更新 PID 设定值
extern void PID_Update_Error(PID_t* pPID, YesNo_t Sample_Valid);                         // 更新 PID 误差值
extern float PID_Update_Out(PID_t* pPID);                                                // 更新 PID 输出值

extern float PID_Update(PID_t* pPID, float Set, float Smp, YesNo_t SmpValid);           // 系统 PID 控制更新

#endif  // __PID_H__

