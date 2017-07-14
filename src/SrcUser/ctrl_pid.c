/*
    文件名称：   ctrl_pid.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   系统控制PID相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#include "ctrl_pid.h"
#include "ctrl_smp.h"
#include "ctrl_fmu.h"

extern Dof_PID_Sample_t Dof_Alt_Smp = {0};      // 高度采样值
extern Dof_PID_t SysPID[Dof_Num] = {0};         // 系统控制参数

// 系统控制参数相似参数初始化
static void Dof_PID_Init_Similar_Parameter(Dof_PID_t* pDof_PID, float Out_Min, float Out_Max, float K_PosErr_To_SpdSet);

// 系统控制参数相似参数初始化
static void Dof_PID_Init_Similar_Parameter(Dof_PID_t* pDof_PID, float Out_Min, float Out_Max, float K_PosErr_To_SpdSet)
{
    pDof_PID->Out_Float = 0;
    pDof_PID->Out_Int16 = 0;
    pDof_PID->Out_Min = Out_Min;
    pDof_PID->Out_Max = Out_Max;
    pDof_PID->K_PosErr_To_SpdSet = K_PosErr_To_SpdSet;
    
    pDof_PID->PID[Pos].valid_last = No;
    Fill_Mem((uint8*)pDof_PID->PID[Pos].sample_cache, sizeof(float)*SAMPLE_CACHE_MAX, 0);
    pDof_PID->PID[Pos].sample_cache_cnt = 0;
    pDof_PID->PID[Pos].err = 0;
    pDof_PID->PID[Pos].err_last = 0;
    pDof_PID->PID[Pos].err_p = 0;
    pDof_PID->PID[Pos].err_i = 0;
    pDof_PID->PID[Pos].err_d = 0;    
    pDof_PID->PID[Pos].out = 0;
    pDof_PID->PID[Pos].out_p = 0;
    pDof_PID->PID[Pos].out_i = 0;
    pDof_PID->PID[Pos].out_i_min = pDof_PID->Out_Min * 0.5;
    pDof_PID->PID[Pos].out_i_max = pDof_PID->Out_Max * 0.5;
    pDof_PID->PID[Pos].out_d = 0;
    
    pDof_PID->PID[Spd].valid_last = No;
    Fill_Mem((uint8*)pDof_PID->PID[Spd].sample_cache, sizeof(float)*SAMPLE_CACHE_MAX, 0);
    pDof_PID->PID[Spd].sample_cache_cnt = 0;
    pDof_PID->PID[Spd].err = 0;
    pDof_PID->PID[Spd].err_last = 0;
    pDof_PID->PID[Spd].err_p = 0;
    pDof_PID->PID[Spd].err_i = 0;
    pDof_PID->PID[Spd].err_d = 0;    
    pDof_PID->PID[Spd].out = 0;
    pDof_PID->PID[Spd].out_p = 0;
    pDof_PID->PID[Spd].out_i = 0;
    pDof_PID->PID[Spd].out_i_min = pDof_PID->Out_Min * 0.5;
    pDof_PID->PID[Spd].out_i_max = pDof_PID->Out_Max * 0.5;
    pDof_PID->PID[Spd].out_d = 0;
}

// 系统控制参数初始化
extern void PID_Parameter_Init(void)
{
    // PID参数设定：控制项、输出最小值、输出最大值、外环对内环控制系数
    Dof_PID_Init_Similar_Parameter(&SysPID[Rol], -1200, 1200, 40.0);
    Dof_PID_Init_Similar_Parameter(&SysPID[Pit], -1200, 1200, 40.0);
    Dof_PID_Init_Similar_Parameter(&SysPID[Alt], -150, 150, 12.0);
    Dof_PID_Init_Similar_Parameter(&SysPID[Yaw], -500, 500, 10.0);
    
    // 横滚 位置环 控制参数 P、I、D、采样缓存个数、设定值、设定值最小值、设定值最大值、积分项绝对值最大值、积分分离最大误差值
    SysPID[Rol].PID[Pos].p = -1.00;
    SysPID[Rol].PID[Pos].i = -0.12;
    SysPID[Rol].PID[Pos].d = -3.00;//-2.00;
    SysPID[Rol].PID[Pos].sample_cache_max = 2;
    SysPID[Rol].PID[Pos].set = 320 / 2;
    SysPID[Rol].PID[Pos].set_min = (320 / 2) - (240 / 2);
    SysPID[Rol].PID[Pos].set_max = (320 / 2) + (240 / 2);
    SysPID[Rol].PID[Pos].err_i_abs_max = 200;
    SysPID[Rol].PID[Pos].i_act_err_abs_max = 80;
    
    // 横滚 速度环 控制参数 P、I、D、采样缓存个数、设定值、设定值最小值、设定值最大值、积分项绝对值最大值、积分分离最大误差值
    SysPID[Rol].PID[Spd].p = -32.0;
    SysPID[Rol].PID[Spd].i = -1.00;
    SysPID[Rol].PID[Spd].d = -20.0;
    SysPID[Rol].PID[Spd].sample_cache_max = 5;
    SysPID[Rol].PID[Spd].set = 0;
    SysPID[Rol].PID[Spd].set_min = -4;
    SysPID[Rol].PID[Spd].set_max = 4;
    SysPID[Rol].PID[Spd].err_i_abs_max = 30;
    SysPID[Rol].PID[Spd].i_act_err_abs_max = 10;
    
    // 俯仰 位置环 控制参数 P、I、D、采样缓存个数、设定值、设定值最小值、设定值最大值、积分项绝对值最大值、积分分离最大误差值
    SysPID[Pit].PID[Pos].p =                    SysPID[Rol].PID[Pos].p;
    SysPID[Pit].PID[Pos].i =                    SysPID[Rol].PID[Pos].i;
    SysPID[Pit].PID[Pos].d =                    SysPID[Rol].PID[Pos].d;
    SysPID[Pit].PID[Pos].sample_cache_max =     SysPID[Rol].PID[Pos].sample_cache_max;
    SysPID[Pit].PID[Pos].set =                  240 / 2;
    SysPID[Pit].PID[Pos].set_min =              0;
    SysPID[Pit].PID[Pos].set_max =              240;
    SysPID[Pit].PID[Pos].err_i_abs_max =        SysPID[Rol].PID[Pos].err_i_abs_max;
    SysPID[Pit].PID[Pos].i_act_err_abs_max =    SysPID[Rol].PID[Pos].i_act_err_abs_max;
    
    // 俯仰 速度环 控制参数 P、I、D、采样缓存个数、设定值、设定值最小值、设定值最大值、积分项绝对值最大值、积分分离最大误差值
    SysPID[Pit].PID[Spd].p =                    SysPID[Rol].PID[Spd].p;
    SysPID[Pit].PID[Spd].i =                    SysPID[Rol].PID[Spd].i;
    SysPID[Pit].PID[Spd].d =                    SysPID[Rol].PID[Spd].d;
    SysPID[Pit].PID[Spd].sample_cache_max =     SysPID[Rol].PID[Spd].sample_cache_max;    
    SysPID[Pit].PID[Spd].set =                  SysPID[Rol].PID[Spd].set;
    SysPID[Pit].PID[Spd].set_min =              SysPID[Rol].PID[Spd].set_min;
    SysPID[Pit].PID[Spd].set_max =              SysPID[Rol].PID[Spd].set_max;
    SysPID[Pit].PID[Spd].err_i_abs_max =        SysPID[Rol].PID[Spd].err_i_abs_max;
    SysPID[Pit].PID[Spd].i_act_err_abs_max =    SysPID[Rol].PID[Spd].i_act_err_abs_max;
    
    // 高度 位置环 控制参数 P、I、D、采样缓存个数、设定值、设定值最小值、设定值最大值、积分项绝对值最大值、积分分离最大误差值
    SysPID[Alt].PID[Pos].p = 1.20;
    SysPID[Alt].PID[Pos].i = 0.50;
    SysPID[Alt].PID[Pos].d = 0.00;
    SysPID[Alt].PID[Pos].sample_cache_max =  SAMPLE_CACHE_MAX;
    SysPID[Alt].PID[Pos].set = 80;
    SysPID[Alt].PID[Pos].set_min = 10;
    SysPID[Alt].PID[Pos].set_max = 120;
    SysPID[Alt].PID[Pos].err_i_abs_max = 8;
    SysPID[Alt].PID[Pos].i_act_err_abs_max = 3;
    
    // 高度 速度环 控制参数 P、I、D、采样缓存个数、设定值、设定值最小值、设定值最大值、积分项绝对值最大值、积分分离最大误差值
    SysPID[Alt].PID[Spd].p = 45.0;
    SysPID[Alt].PID[Spd].i = 0.00;
    SysPID[Alt].PID[Spd].d = 0.00;
    SysPID[Alt].PID[Spd].sample_cache_max =  SAMPLE_CACHE_MAX;
    SysPID[Alt].PID[Spd].set = 0;
    SysPID[Alt].PID[Spd].set_min = -2.0;
    SysPID[Alt].PID[Spd].set_max = 2.0;
    SysPID[Alt].PID[Spd].err_i_abs_max = 200;
    SysPID[Alt].PID[Spd].i_act_err_abs_max = 200;
    
    // 偏航 位置环 控制参数 P、I、D、采样缓存个数、设定值、设定值最小值、设定值最大值、积分项绝对值最大值、积分分离最大误差值
    SysPID[Yaw].PID[Pos].p = -1.00;
    SysPID[Yaw].PID[Pos].i = -0.20;
    SysPID[Yaw].PID[Pos].d = -0.00;
    SysPID[Yaw].PID[Pos].sample_cache_max = 3;
    SysPID[Yaw].PID[Pos].set = 0;
    SysPID[Yaw].PID[Pos].set_min = -800;
    SysPID[Yaw].PID[Pos].set_max = 800;
    SysPID[Yaw].PID[Pos].err_i_abs_max = 100;
    SysPID[Yaw].PID[Pos].i_act_err_abs_max = 100;
    
    // 偏航 速度环 控制参数 P、I、D、采样缓存个数、设定值、设定值最小值、设定值最大值、积分项绝对值最大值、积分分离最大误差值
    SysPID[Yaw].PID[Spd].p = 0.00;
    SysPID[Yaw].PID[Spd].i = 0.00;
    SysPID[Yaw].PID[Spd].d = 0.00;
    SysPID[Yaw].PID[Spd].sample_cache_max = 3;
    SysPID[Yaw].PID[Spd].set = 0;
    SysPID[Yaw].PID[Spd].set_min = -1;
    SysPID[Yaw].PID[Spd].set_max = 1;
    SysPID[Yaw].PID[Spd].err_i_abs_max = 5;
    SysPID[Yaw].PID[Spd].i_act_err_abs_max = 1;
}

// 更新高度采样
extern void PID_Update_Alt_Sample(float NewAltPos_Raw)
{
    // 如果高度采样可更新
    if(Yes == En_Update_Alt_Sample)
    {
        En_Update_Alt_Sample = No;
        
        Dof_Alt_Smp.PosSmp.valid_last = Dof_Alt_Smp.PosSmp.valid;    
        Dof_Alt_Smp.PosSmp.sample_last = Dof_Alt_Smp.PosSmp.sample;
        if(Yes == Is_Alt_Valid(NewAltPos_Raw))
        {
            Dof_Alt_Smp.PosSmp.valid = Yes;
            Dof_Alt_Smp.PosSmp.sample = NewAltPos_Raw;
        }
        else
            Dof_Alt_Smp.PosSmp.valid = No;
        
        Dof_Alt_Smp.SpdSmp.valid_last = Dof_Alt_Smp.SpdSmp.valid;
        Dof_Alt_Smp.SpdSmp.sample_last = Dof_Alt_Smp.SpdSmp.sample;
        if((Yes == Dof_Alt_Smp.PosSmp.valid) && (Yes == Dof_Alt_Smp.PosSmp.valid_last))
        {
            Dof_Alt_Smp.SpdSmp.valid = Yes;
            Dof_Alt_Smp.SpdSmp.sample = Dof_Alt_Smp.PosSmp.sample - Dof_Alt_Smp.PosSmp.sample_last;
        }
        else
            Dof_Alt_Smp.SpdSmp.valid = No;
    }
}

// 获取当前高度
extern uint16 Get_Height(void)
{
    return SysPID[Alt].PID[Pos].sample;
}

// 系统控制 PID 输出更新
extern int16 Dof_PID_Update_Calculate(Dof_PID_t* pDof_PID)
{
    pDof_PID->Out_Float = 0.00;
    pDof_PID->Out_Float += PID_Update_Out(&pDof_PID->PID[Pos]);
    pDof_PID->Out_Float += PID_Update_Out(&pDof_PID->PID[Spd]);
    
    if(pDof_PID->Out_Float < pDof_PID->Out_Min)
        pDof_PID->Out_Float = pDof_PID->Out_Min;
    else if(pDof_PID->Out_Float > pDof_PID->Out_Max)
        pDof_PID->Out_Float = pDof_PID->Out_Max;
    
    pDof_PID->Out_Int16 = (int16)pDof_PID->Out_Float;
    
    return pDof_PID->Out_Int16;
}

// 更新指定的PID项
extern int16 Dof_PID_Update(Dof_PID_t* pDof_PID, float PosSet, float PosSmp, YesNo_t PosSmp_Valid, float SpdSet, float SpdSmp, YesNo_t SpdSmp_Valid)
{
    pDof_PID->Out_Float = 0.00;
    pDof_PID->Out_Float += PID_Update(&pDof_PID->PID[Pos], PosSet, PosSmp, PosSmp_Valid);
    pDof_PID->Out_Float += PID_Update(&pDof_PID->PID[Spd], SpdSet, SpdSmp, SpdSmp_Valid);
    
    if(pDof_PID->Out_Float < pDof_PID->Out_Min)
        pDof_PID->Out_Float = pDof_PID->Out_Min;
    else if(pDof_PID->Out_Float > pDof_PID->Out_Max)
        pDof_PID->Out_Float = pDof_PID->Out_Max;
    
    pDof_PID->Out_Int16 = (int16)pDof_PID->Out_Float;
    
    return pDof_PID->Out_Int16;
}

// 判断高度值是否合理
extern YesNo_t Is_Alt_Valid(uint16 Alt_cm)
{
    if((Alt_cm > 10) && (Alt_cm < 280))
        return Yes;
    else
        return No;
}

// 更新 PID 采样值
extern void PID_Update_Sample(PID_t* pPID, float Sample_NewRaw, YesNo_t Sample_Valid)
{
    uint16 i;
    
    pPID->valid = Sample_Valid;
    
    for(i=SAMPLE_CACHE_MAX-1; i>0; i--)
        pPID->sample_cache[i] = pPID->sample_cache[i-1];
    
    pPID->sample_cache[0] = Sample_NewRaw;
    
    if(Yes == Sample_Valid)
    {
        pPID->sample_cache_cnt++;
        if(pPID->sample_cache_cnt > pPID->sample_cache_max)
            pPID->sample_cache_cnt = pPID->sample_cache_max;
        
//        pPID->sample = Average_Float(pPID->sample_cache, pPID->sample_cache_cnt);
        pPID->sample = DataFilter(pPID->sample_cache, pPID->sample_cache_cnt);
    }    
    else
    {
        pPID->sample_cache_cnt = 0;
    }
}

// 更新 PID 设定值
extern void PID_Update_Set(PID_t* pPID, float Set_New)
{
    pPID->set = Set_New;
    
    if(pPID->set < pPID->set_min)
        pPID->set = pPID->set_min;
    else if(pPID->set > pPID->set_max)
        pPID->set = pPID->set_max;
}

// 更新 PID 误差值
extern void PID_Update_Error(PID_t* pPID, YesNo_t Sample_Valid)
{
    pPID->err_last = pPID->err;
    if(Yes == Sample_Valid)
    {
        pPID->err = pPID->set - pPID->sample;
        
        if(pPID->err > 0)
            pPID->err_abs = pPID->err;
        else
            pPID->err_abs = -pPID->err;
        
        pPID->err_p = pPID->err;
        
        if(Abs(pPID->err) < pPID->i_act_err_abs_max)
        {
            pPID->err_i += pPID->err;
            Constraint_By_Abs(&pPID->err_i, pPID->err_i_abs_max);
        }
        else
            pPID->err_i = 0;
        
        if(Yes == pPID->valid_last)
            pPID->err_d = pPID->err - pPID->err_last;
        else
            pPID->err_d = 0;
    }
    else
    {
        pPID->err = 0;
        pPID->err_p = 0;
        pPID->err_i = 0;
        pPID->err_d = 0;
    }
}

// 更新 PID 输出值
extern float PID_Update_Out(PID_t* pPID)
{
    pPID->out_p = pPID->p * pPID->err_p;
    pPID->out_i = pPID->i * pPID->err_i;
    pPID->out_d = pPID->d * pPID->err_d;
    
    if(pPID->out_i < pPID->out_i_min)
        pPID->out_i = pPID->out_i_min;
    else if(pPID->out_i > pPID->out_i_max)
        pPID->out_i = pPID->out_i_max;
    
    pPID->out = pPID->out_p + pPID->out_i + pPID->out_d;
    
    return pPID->out;
}

// 系统 PID 控制更新
extern float PID_Update(PID_t* pPID, float Set, float Smp, YesNo_t SmpValid)
{
    pPID->valid_last = SmpValid;
    
    PID_Update_Set(pPID, Set);
    PID_Update_Sample(pPID, Smp, SmpValid);
    PID_Update_Error(pPID, SmpValid);
    PID_Update_Out(pPID);
    
    return pPID->out;
}

