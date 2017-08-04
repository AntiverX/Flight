/*
    文件名称：   ctrl_basic.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   基本数据类型和基本操作实现
    修改日期：   2017-7-7
    修改内容：   添加注释
*/

#ifndef __CTRL_BASIC_H__
#define __CTRL_BASIC_H__

#include "math.h"

// 数据类型定义
typedef signed              char    int8;
typedef signed      short   int     int16;
typedef signed              int     int32;
typedef signed      long    long    int64;

typedef unsigned            char    uint8;
typedef unsigned    short   int     uint16;
typedef unsigned            int     uint32;
typedef unsigned    long    long    uint64;

//typedef signed              char    int8_t;
//typedef signed      short   int     int16_t;
//typedef signed              int     int32_t;
//typedef signed      long    long    int64_t;

//typedef unsigned            char    uint8_t;
//typedef unsigned    short   int     uint16_t;
//typedef unsigned            int     uint32_t;
//typedef unsigned    long	long    uint64_t;

// 二态数据
typedef enum
{
    No = 0,
    Yes,
}YesNo_t;

// 操作成功或失败
typedef enum
{
    Failure = 0,
    Success,
}OptSta_t;

#define FLOAT_0                     ((float)(0.0001))                                       // 浮点数 0
#define FLOAT_EQUAL(x, cmp)         ((((x)-(cmp)) > -FLOAT_0) && (((x)-(cmp)) < FLOAT_0))   // 判断浮点数等于0

// include"math.h"  中的三角函数均是按照弧度进行计算的
#define Pi                          ((double)3.1415926)         // π
#define DEGREE_TO_RADIAN(d)         ((d) * Pi / 180)             // 角度转弧度
#define RADIAN_TO_DEGREE(r)         ((r) * 180 / Pi)             // 弧度转角度

// 毫秒级延时相关变量
extern uint32 SysDelay_ms_Cnt;
extern uint32 Sys_ms;

extern float Average_Float(float* pDataBuf, uint16 num);            // 求指定个数浮点数的平均值
extern void Fill_Mem(uint8* pAdd, uint16 Len, uint8 FillWith);      // 用特定数据填充指定区域
extern void SysDelay_ms(uint32 ms);                                 // 毫秒级延时函数 非阻塞式
extern float Abs(float num);                                        // 绝对值
extern void Constraint(float* pFloatNum, float Min, float Max);     // 数值范围约束
extern void Constraint_By_Abs(float* pFloatNum, float AbsMax);      // 绝对值约束
extern float DataFilter(float* pDataBuf, uint16 Num);               // 数据滤波算法
extern void Data_Quick_Sort(float* pDataBuf, uint16 Num);           // 数据快速排序

#define DBG 1

#endif	// __CTRL_BASIC_H__

