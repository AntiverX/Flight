///*
//    文件名称：   ctrl_serial_graph.c
//    文件作者：   中科浩电 www.bj-zkhd.com
//    文件功能：   输出调试曲线相关内容
//    修改日期：   2017-7-7
//    修改内容：   修改注释
//*/
//
//#include "ctrl_serial_graph.h"
//#include "ctrl_usart.h"
//
//// 山外多功能调试助手 数据结构
//typedef struct
//{
//    int16 St;       // 帧头 固定内容  0x03 0xFC
//    int16 Ch[8];    // 曲线数据 8个通道
//    int16 Sp;       // 帧尾 固定内容  0x03 0xFC
//}ShanWaiGraph_Pkg_t;
//
//// 山外多功能调试助手 通信协议
//typedef union
//{
//    ShanWaiGraph_Pkg_t Pkg;
//    uint8 Buf[sizeof(ShanWaiGraph_Pkg_t)];
//}ShanWaiGraph_t;
//
//// 定义通信数据
//static ShanWaiGraph_t ShanWaiGraph;
//
//// 向 山外多功能调试助手 发送曲线信息
//extern void GraphGen(int16 ch1,int16 ch2,int16 ch3,int16 ch4,int16 ch5,int16 ch6,int16 ch7,int16 ch8)
//{
//
//    ShanWaiGraph.Pkg.St = 0xFC03;
//    ShanWaiGraph.Pkg.Sp = 0x03FC;
//
//    ShanWaiGraph.Pkg.Ch[0] = ch1;
//    ShanWaiGraph.Pkg.Ch[1] = ch2;
//    ShanWaiGraph.Pkg.Ch[2] = ch3;
//    ShanWaiGraph.Pkg.Ch[3] = ch4;
//    ShanWaiGraph.Pkg.Ch[4] = ch5;
//    ShanWaiGraph.Pkg.Ch[5] = ch6;
//    ShanWaiGraph.Pkg.Ch[6] = ch7;
//    ShanWaiGraph.Pkg.Ch[7] = ch8;
//
//    Usart_Send_Data(&huart3, ShanWaiGraph.Buf, sizeof(ShanWaiGraph_t));
//}
//
