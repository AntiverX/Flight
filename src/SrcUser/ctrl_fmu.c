/*
    文件名称：   ctrl_fmu.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   控制板和飞控相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#include "ctrl_fmu.h"
#include "ctrl_led.h"
#include "ctrl_flash.h"
#include "ctrl_pid.h"
#include "ctrl_control.h"
#include "ctrl_beep.h"
#include "ctrl_usart.h"
#include "ctrl_smp.h"
#include "r_cg_macrodriver.h"
#include "r_cg_cmt.h"
#include "r_cg_port.h"
#include "r_cg_sci.h"

extern Msg_FMUToCtrl_t Msg_FMUToCtrl = {0};     // 飞控给控制板发送的数据包
extern Msg_CtrlToFMU_t Msg_CtrlToFMU = {0};     // 控制板给飞控发送的数据包

extern YesNo_t En_Update_Alt_Sample = No;      // 高度可更新标志(高度是飞控发送给控制板的，当控制板接收到了飞控的数据包，才去更新高度采样值)
extern YesNo_t AutoCtrl_RC_Ch6 = No;           // 飞控下发的数据包中，遥控器第六通道用来开启、关闭程控
extern YesNo_t AutoCtrl_HeightAbove_35cm = No; // 高度是否达到了xx厘米，高度太低，镜头捕捉的画面无法识别特征点
extern uint16_t g_sci1_rx_count;
/*
    功能：更新飞控发送给控制板的信息
    返回：是否正确收到了飞控发送的数据包
*/
extern YesNo_t Msg_FMU_To_Ctrl_Update(void)
{
    YesNo_t UpdateFlag;
    
    static uint16 New_Pkg_Offset = 0;                          // 本次接收的数据包在接收 Buffer 中的起始偏移量(当通信正常 该值为 0)
    static uint16 New_Pkg_Length = sizeof(Msg_FMUToCtrl_t);    // 本次期望接收的数据包长度(当通信正常 总是期望接收一个完整的数据包)
    Msg_FMUToCtrl_t* pNewMsg;
    uint16 i;
    
    UpdateFlag = No;
    
    // 串口 1 收到了来自飞控的信息
    if(Yes == U1_Rx_End)
    {
        // 数据包长度正常
    	//----------------------------------Modified Here.-----------------------------------------//
//    	if(New_Pkg_Length == huart1.RxXferSize)
        if(New_Pkg_Length == g_sci1_rx_count)
        {
            pNewMsg = (Msg_FMUToCtrl_t*)&U1RxBuf[New_Pkg_Offset];
            
            // 通过内容固定的帧头、帧尾检测数据包是否正确
            if((0xAAAA == pNewMsg->Pkg.St) && (0x5555 == pNewMsg->Pkg.Sp))
            {
                // 一切正常，将接收到的数据拷贝到信息存储区
                for(i=0; i<sizeof(Msg_FMUToCtrl_t); i++)
                {
                    Msg_FMUToCtrl.Buf[i] = pNewMsg->Buf[i];
                }
                UpdateFlag = Yes;                           // 成功接收到数据包
                New_Pkg_Offset = 0;                         // 下次接收的偏移量为 0
                New_Pkg_Length = sizeof(Msg_FMUToCtrl_t);   // 下次期望接收一个完整的数据包
                LED_B_IO_TOOGLE;                            // 蓝色 LED 状态翻转一次(若通信正常 看到蓝色 LED 在闪烁)
            }
            // 帧长正确 但是帧头或帧尾至少有个出错
            else
            {
            	LED_B_IO_ON;
                // 或许上次接收的数据包还正确，但是目前发生了错误
                if(0 == New_Pkg_Offset)
                {
                    for(i=0; i<New_Pkg_Length-1; i++)
                    {
                        // 尝试从目前接收到的信息中找到帧头
                        if((0xAA == pNewMsg->Buf[i]) && (0xAA == pNewMsg->Buf[i+1]))
                        {
                            New_Pkg_Offset = i;
                            New_Pkg_Length = sizeof(Msg_FMUToCtrl_t) - (New_Pkg_Length - (i+1));
                            break;
                        }
                        // 尝试从目前接收到的信息中找到帧尾
                        else if((0x55 == pNewMsg->Buf[i]) && (0x55 == pNewMsg->Buf[i+1]))
                        {
                            New_Pkg_Offset = i + 2;
                            New_Pkg_Length = sizeof(Msg_FMUToCtrl_t) - (New_Pkg_Length - (i+2));
                            break;
                        }
                    }
                    New_Pkg_Length = sizeof(Msg_FMUToCtrl_t) - New_Pkg_Length;
                }
                // 本次通信异常，上次也是出错了
                else
                {
                	LED_B_IO_OFF;
                    New_Pkg_Offset = 0;
                    New_Pkg_Length = sizeof(Msg_FMUToCtrl_t);
                }
            }
        }
        // 接收的长度和期望的不一致，重新接收
        else
        {
        	LED_B_IO_ON;
            New_Pkg_Offset = 0;
            New_Pkg_Length = sizeof(Msg_FMUToCtrl_t);
        }
        
        En_Update_Alt_Sample = UpdateFlag;      // 高度采样值是否可以更新
        En_RC_Offset_Update = UpdateFlag;       // 遥控器微调值是否可以更新
        
        if(Msg_FMUToCtrl.Pkg.RC_Ch_06 > 500)    // 判断程控使能开关是否打开(当遥控器上的通道6输入大于500则认为 程控开启)
            AutoCtrl_RC_Ch6 = Yes;
        else
            AutoCtrl_RC_Ch6 = No;
        
        // 当控制板成功接收到了飞控的信息，需要将部分信息转发给采集板
        if(Yes == UpdateFlag)
        {
            Msg_Ctrl_To_Smp_Update();
        }
        U1_Rx_End = No;
        R_SCI1_Serial_Receive(&U1RxBuf[New_Pkg_Offset],New_Pkg_Length);
    }
    return UpdateFlag;
}

