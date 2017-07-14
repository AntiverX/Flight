/*
    文件名称：   ctrl_basic.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   基本数据类型和基本操作实现
    修改日期：   2017-7-7
    修改内容：   添加注释
*/

#include "ctrl_basic.h"

extern uint32 SysDelay_ms_Cnt = 0;      // 定时器毫秒级延时计数
extern uint32 Sys_ms = 0;               // 系统时间 毫秒(从系统开始运行了多长时间了)

// 求指定个数浮点数的平均值
extern float Average_Float(float* pDataBuf, uint16 num)
{
    double Sum;
    uint16 i;
    float Average;
    
    Sum = 0.0;
    for(i=0; i<num; i++)
        Sum += pDataBuf[i];
    
    Average = Sum / num;
    
    return Average;
}

// 用特定数据填充指定区域
extern void Fill_Mem(uint8* pAdd, uint16 Len, uint8 FillWith)
{
    uint16 i;
    
    for(i=0; i< Len; i++)
    {
        pAdd[i] = FillWith;
    }
}

// 毫秒级延时函数 非阻塞式
extern void SysDelay_ms(uint32 ms)
{
    SysDelay_ms_Cnt = 0;
    
    while(SysDelay_ms_Cnt < ms)
        ;
}

// 绝对值
extern float Abs(float num)
{
    if(num >= 0)
        return num;
    else
        return -num;
}

// 数值范围约束
extern void Constraint(float* pFloatNum, float Min, float Max)
{
    if(Min <= Max)
    {
        if(*pFloatNum < Min)
            *pFloatNum = Min;
        else if(*pFloatNum > Max)
            *pFloatNum = Max;
    }
}

// 按照绝对值进行数据范围约束
extern void Constraint_By_Abs(float* pFloatNum, float AbsMax)
{
    Constraint(pFloatNum, -AbsMax, AbsMax);
}

// 数据滤波算法(滑动窗口滤波算法)
extern float DataFilter(float* pDataBuf, uint16 Num)
{
    static float DataCache[20];
    static double Sum;
    uint16 i;
    
    if(1 == Num)
    {
        return pDataBuf[0];
    }
    else if(2 == Num)
    {
        Sum = (pDataBuf[0] + pDataBuf[1]) / 2;
        return (float)Sum;
    }
    else if(Num >= 3)
    {
        for(i=0; i<Num; i++)
            DataCache[i] = pDataBuf[i];
        
        Data_Quick_Sort(DataCache, Num);
        Sum = 0.00;
        for(i=1; i<=Num-2; i++)
            Sum += DataCache[i];
        
        return Sum/(Num-2);
    }
    
    return 0;
}

// 数据快速排序算法
extern void Data_Quick_Sort(float* pDatas, uint16 Num)
{
    uint16 CntL, CntR;
    float Compare;
    
    if(Num < 2)
        return;
	
    CntL = 0;
    CntR = Num - 1;
	
	Compare = pDatas[CntL];  		// first item saved, and be used for comparer
   
    while(CntL < CntR)
    {
        while(CntL < CntR)
        {
            if(pDatas[CntR] < Compare)
            {
                pDatas[CntL] = pDatas[CntR];
                break;
            }
            CntR--;
        }
        
        while(CntL < CntR)
        {
            if(pDatas[CntL] > Compare)
            {
                pDatas[CntR] = pDatas[CntL];
                break;
            }
            CntL++;
        }
    }
    
    pDatas[CntL++] = Compare;                       // rember to cover the last saved element with Comparer
    Data_Quick_Sort(&pDatas[0], CntL);              // pDatas[0]        ~   pDatas[CntL-1]
    Data_Quick_Sort(&pDatas[CntL], Num-(CntL));     // pDatas[CntL] ~   pDatas[Num-1]
}

