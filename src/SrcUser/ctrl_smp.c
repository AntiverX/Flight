/*
    文件名称：   ctrl_smp.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   控制板和采集板通信相关内容
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#include "ctrl_smp.h"
#include "ctrl_fmu.h"
#include "ctrl_led.h"
#include "ctrl_usart.h"
#include "r_cg_sci.h"

extern Msg_SmpToCtrl_t Msg_SmpToCtrl = {0};

extern YesNo_t En_Update_PID = No;                      // 系统PID运算更新使能，每当接收到采集板的信息，才更新PID运算
extern YesNo_t En_SendMsgToFMU_WhenSmpUpdate = No;      // 控制板向飞控发送控制指令使能，每当处理完采集板的信息，才向飞控发送新的控制命令
extern YesNo_t En_SendDbgMsg = No;                      // 发送调试信息使能，每当处理完采集板信息，才发送调试信息
extern YesNo_t En_Update_Act = No;                      // 允许更新状态
extern uint16_t g_sci5_rx_count;
extern Msg_CtrlToSmp_t Msg_CtrlToSmp = {0};

// 初始化控制板发送给采集板的信息
extern void Msg_Ctrl_To_Smp_Init(void)
{
    Msg_CtrlToSmp.Pkg.Seq = 0;
}
/*
    功能：接收、处理采集板发送给控制板的信息
    返回：是否正确接收到了采集板的数据包
*/
extern YesNo_t Msg_Smp_To_Ctrl_Update(void)
{
    YesNo_t UpdateFlag;
    
    static uint16 New_Pkg_Offset = 0;                          // offset of this pacakge start(in receive buffer)
    static uint16 New_Pkg_Length = sizeof(Msg_SmpToCtrl_t);    // length of this package
    Msg_SmpToCtrl_t* pNewMsg;
    uint16 i;
    
    UpdateFlag = No;
    
    // received package from vision sample board
    if(Yes == U2_Rx_End)
    {
        // package length is ok
    	//----------------------------------Modified Here.-----------------------------------------//
//        if(New_Pkg_Length == huart2.RxXferSize)
        if(New_Pkg_Length == g_sci5_rx_count)
        {
            pNewMsg = (Msg_SmpToCtrl_t*)&U2RxBuf[New_Pkg_Offset];
            
            // package length and start and end all ok
            if((0xAAAA == pNewMsg->Pkg.St) && (0x5555 == pNewMsg->Pkg.Sp))
            {
                for(i=0; i<sizeof(Msg_SmpToCtrl_t); i++)
                {
                    Msg_SmpToCtrl.Buf[i] = pNewMsg->Buf[i];
                }
                UpdateFlag = Yes;
                New_Pkg_Offset = 0;
                New_Pkg_Length = sizeof(Msg_SmpToCtrl_t);
                LED_R_IO_TOOGLE;
            }
            // package length is ok, but start or end not
            else
            {
            	LED_R_IO_ON;
                
                // maybe everything is ok last time, but error occurred this time
                if(0 == New_Pkg_Offset)
                {
                    for(i=0; i<New_Pkg_Length-1; i++)
                    {
                        if((0xAA == pNewMsg->Buf[i]) && (0xAA == pNewMsg->Buf[i+1]))
                        {
                            New_Pkg_Offset = i;
                            New_Pkg_Length = sizeof(Msg_SmpToCtrl_t) - (New_Pkg_Length - (i+1));
                            break;
                        }
                        else if((0x55 == pNewMsg->Buf[i]) && (0x55 == pNewMsg->Buf[i+1]))
                        {
                            New_Pkg_Offset = i + 2;
                            New_Pkg_Length = sizeof(Msg_SmpToCtrl_t) - (New_Pkg_Length - (i+2));
                            break;
                        }
                    }
                    New_Pkg_Length = sizeof(Msg_SmpToCtrl_t) - New_Pkg_Length;
                }
                // error occurred last time, and also this time
                else
                {
                    New_Pkg_Offset = 0;
                    New_Pkg_Length = sizeof(Msg_SmpToCtrl_t);
                }
            }
        }
        // package length error, reset communication
        else
        {
        	LED_R_IO_ON;
            New_Pkg_Offset = 0;
            New_Pkg_Length = sizeof(Msg_SmpToCtrl_t);
        }
        En_Update_PID = UpdateFlag;
        En_SendMsgToFMU_WhenSmpUpdate = UpdateFlag;
        En_SendDbgMsg = UpdateFlag;
        En_Update_Act = UpdateFlag;
        //----------------------------------Modified Here.-----------------------------------------//
//        Usart_Receive_IT(&huart2, &U2RxBuf[New_Pkg_Offset], New_Pkg_Length);
        R_SCI5_Serial_Receive(&U2RxBuf[New_Pkg_Offset], New_Pkg_Length);
    }
    
    return UpdateFlag;
}

// 控制板向采集板发送信息
extern void Msg_Ctrl_To_Smp_Update(void)
{
	Msg_CtrlToSmp.Pkg.St = 0xAAAA;
	Msg_CtrlToSmp.Pkg.Sp = 0x5555;
    
	Msg_CtrlToSmp.Pkg.Alt_Sonar     = Msg_FMUToCtrl.Pkg.Alt_Sonar;          // 飞控采集到的高度
	Msg_CtrlToSmp.Pkg.AHRS_Rol_1000 = Msg_FMUToCtrl.Pkg.AHRS_Rol_1000;      // 飞控姿态 Roll
	Msg_CtrlToSmp.Pkg.AHRS_Pit_1000 = Msg_FMUToCtrl.Pkg.AHRS_Pit_1000;      // 飞控姿态 Pitch
	Msg_CtrlToSmp.Pkg.AHRS_Yaw_1000 = Msg_FMUToCtrl.Pkg.AHRS_Yaw_1000;      // 飞控姿态 Yaw
    
	R_SCI5_Serial_Send(Msg_CtrlToSmp.Buf, sizeof(Msg_CtrlToSmp_t));
}

