///*
//    文件名称：   ctrl_debug.c
//    文件作者：   中科浩电 www.bj-zkhd.com
//    文件功能：   通过串口3进行调试相关功能
//    修改日期：   2017-7-7
//    修改内容：   修改注释
//*/
//
//#include "ctrl_debug.h"
//#include "ctrl_smp.h"
//#include "ctrl_serial_graph.h"
//#include "ctrl_control.h"
//#include "ctrl_encoder.h"
//#include "ctrl_usart.h"
//
//static void Dbg_Update_PID_Paramer(U3Cmd_t U3Cmd);                          // 根据串口命令调试
//static uint16 U3_TxBuf_Load(uint16 StartID, uint8* pLoad, uint16 len);    // 向串口3发送缓存区装载数据
//static void Show_PID_Parameter(void);                                       // 串口输出当前所有控制参数
//
//extern volatile Dof_t Dbg_Dof = Rol;                                        // 调试项 目前在调试 横滚、俯仰、偏航、高度 哪一项
//extern volatile DofPID_t Dbg_PID = Pos;                                     // 调试项 目前在调试 内环、外环 哪一项
//extern volatile DofPIDPara_t Dbg_Para = P;                                  // 调试项 目前在调试 比例、积分、微分 哪一项
//extern DbgOut_t DbgOutType = Ascii;                                         // 调试信息输出方式 可以按照字符或曲线两种方式进行
//extern float PidPara[Dof_Num][DofPID_Num][Para_Num] = {0};                  // 调试参数缓存
//
//static uint16 U3Tx_NextID = 0;                                              // 串口3发送缓存区装载下一个空闲地址偏移量
//
//// 调试提示信息
//static char* MsgDof[Dof_Num] =
//{
//    "Rol ",
//    "Pit ",
//    "Alt ",
//    "Yaw ",
//};
//
//// 调试提示信息
//static char* MsgPID[DofPID_Num] =
//{
//    "Pos ",
//    "Spd ",
//};
//
//// 调试提示信息
//static char* MsgPara[Para_Num] =
//{
//    "P ",
//    "I ",
//    "D ",
//};
//
//// 将系统设定的 PID 参数保存在 PidPara 中，相当于对这些数据做一次缓存
//// 当通过调试终端修改参数时，先改缓存值，然后赋给实际控制参数
//extern void Dbg_PID_Cache_Init(void)
//{
//    Dof_t dof;
//    DofPID_t dofpid;
//
//    for(dof=(Dof_t)0; dof<Dof_Num; dof++)
//    {
//        for(dofpid=(DofPID_t)0; dofpid<DofPID_Num; dofpid++)
//        {
//            PidPara[dof][dofpid][P] = SysPID[dof].PID[dofpid].p;
//            PidPara[dof][dofpid][I] = SysPID[dof].PID[dofpid].i;
//            PidPara[dof][dofpid][D] = SysPID[dof].PID[dofpid].d;
//        }
//    }
//}
//
//// 串口 3 用于无线调试，可输出字符、曲线两种格式，每次只接收一个特定字节的数据(命令)
//extern void Debug_Message_Handle(void)
//{
//    U3Cmd_t U3Cmd;
//
//    // 当串口 3 接收到数据
//    if(Yes == U3_Rx_End)
//    {
//        U3_Rx_End = No;
//        U3Cmd = (U3Cmd_t)U3RxBuf[0];
//
//        switch(U3Cmd)
//        {
//            case U3Cmd_Ascii:   DbgOutType = Ascii;     break;      // 以字符形式输出调试信息
//            case U3Cmd_Graph:   DbgOutType = Graph;     break;      // 以曲线形式输出调试信息
//
//            case U3Cmd_Rol:     Dbg_Dof = Rol;          break;      // 查看 横滚 方向信息
//            case U3Cmd_Pit:     Dbg_Dof = Pit;          break;      // 查看 俯仰 方向信息
//            case U3Cmd_Alt:     Dbg_Dof = Alt;          break;      // 查看 定高 方向信息
//            case U3Cmd_Yaw:     Dbg_Dof = Yaw;          break;      // 查看 偏航 方向信息
//
//            case U3Cmd_Pos:     Dbg_PID = Pos;          break;      // 查看 位置环 信息
//            case U3Cmd_Spd:     Dbg_PID = Spd;          break;      // 查看 速度环 信息
//
//            case U3Cmd_P:       Dbg_Para = P;           break;      // 调试项选定 P
//            case U3Cmd_I:       Dbg_Para = I;           break;      // 调试项选定 I
//            case U3Cmd_D:       Dbg_Para = D;           break;      // 调试项选定 D
//
//            case U3Cmd_I1:                                          // Increase 1       加 1
//            case U3Cmd_D1:                                          // Decrease 1       减 1
//            case U3Cmd_I01:                                         // Increase 0.1     加 0.1
//            case U3Cmd_D01:                                         // Decrease 0.1     减 0.1
//            case U3Cmd_I001:                                        // Increase 0.01    加 0.01
//            case U3Cmd_D001:                                        // Decrease 0.01    减 0.01
//            case U3Cmd_Set0:                                        // Set to 0         设置为 0
//                Dbg_Update_PID_Paramer(U3Cmd);                      // 根据指令修改参数值
//            break;
//
//            case U3Cmd_ShowPID: Show_PID_Parameter();   break;      // 显示当前所有项 PID 全部参数
//
//        }
//
//        Usart_Receive_IT(&huart3, U3RxBuf, 1);
//    }
//}
//
//// 串口3发送调试信息
//extern void Dbg_U3_Send_Msg(void)
//{
//    // 当需要发送调试信息
//    if(Yes == En_SendDbgMsg)
//    {
//        En_SendDbgMsg = No;
//
//        // 发送字符串调试信息
//        if(Ascii == DbgOutType)
//        {
//            U3Tx_NextID = U3_TxBuf_Load(0, (uint8*)MsgDof[Dbg_Dof], strlen(MsgDof[Dbg_Dof]));
//            U3Tx_NextID = U3_TxBuf_Load(U3Tx_NextID, (uint8*)MsgPID[Dbg_PID], strlen(MsgPID[Dbg_PID]));
//            U3Tx_NextID = U3_TxBuf_Load(U3Tx_NextID, (uint8*)MsgPara[Dbg_Para], strlen(MsgPara[Dbg_Para]));
//
//            sprintf(    (char*)&U3TxBuf[U3Tx_NextID], "[%+7.2f %+7.2f %+7.2f] [%+7.2f E:%+7.2f O:%+5d] R:%+5d P:%+5d A:%+5d Y:%+5d %2d:%2d/%2d\r\n",
//                        SysPID[Dbg_Dof].PID[Dbg_PID].p,
//                        SysPID[Dbg_Dof].PID[Dbg_PID].i,
//                        SysPID[Dbg_Dof].PID[Dbg_PID].d,
//
//                        SysPID[Dbg_Dof].PID[Dbg_PID].sample,
//                        SysPID[Dbg_Dof].PID[Dbg_PID].err,
//                        (uint16)SysPID[Dbg_Dof].PID[Dbg_PID].out,
//
//                        SysPID[Rol].Out_Int16,
//                        SysPID[Pit].Out_Int16,
//                        SysPID[Alt].Out_Int16,
//                        SysPID[Yaw].Out_Int16,
//                        Get_Encoder_Value(),
//                        Demo_St_Act + 1,
//                        Demo_Act_Num
//                   );
//
//            Usart_Send_TxBuf_Filled_Data(&huart3, strlen((char*)U3TxBuf));
//        }
//        // 发送观察曲线调试信息
//        else if(Graph == DbgOutType)
//        {
//            GraphGen(   SysPID[Dbg_Dof].PID[Dbg_PID].err_p,
//                        SysPID[Dbg_Dof].PID[Dbg_PID].err_i,
//                        SysPID[Dbg_Dof].PID[Dbg_PID].err_d,
//                        SysPID[Dbg_Dof].PID[Dbg_PID].out_p,
//                        SysPID[Dbg_Dof].PID[Dbg_PID].out_i,
//                        SysPID[Dbg_Dof].PID[Dbg_PID].out_d,
//                        SysPID[Dbg_Dof].PID[Dbg_PID].out,
//                        SysPID[Dbg_Dof].Out_Int16);
//        }
//    }
//}
//
//// 根据指令调整当前调试项的控制参数
//static void Dbg_Update_PID_Paramer(U3Cmd_t U3Cmd)
//{
//    Dof_t dof;
//    DofPID_t dofpid;
//
//    // 根据指令修改参数
//    switch(U3Cmd)
//    {
//        case U3Cmd_I1:      PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] += 1.00;    break;
//        case U3Cmd_D1:      PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] -= 1.00;    break;
//        case U3Cmd_I01:     PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] += 0.10;    break;
//        case U3Cmd_D01:     PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] -= 0.10;    break;
//        case U3Cmd_I001:    PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] += 0.01;    break;
//        case U3Cmd_D001:    PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] -= 0.01;    break;
//        case U3Cmd_Set0:    PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] = 0.00;     break;
//    }
//
//    // 对修改后的参数进行限幅检查
//    if(PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] > 100)
//        PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] = 100;
//    else if(PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] < -100)
//        PidPara[Dbg_Dof][Dbg_PID][Dbg_Para] = -100;
//
//    // 如果修改的是横滚方向的控制参数 将修改后的结果同步到俯仰方向
//    if(Rol == Dbg_Dof)
//    {
//        PidPara[Pit][Pos][P] = PidPara[Rol][Pos][P];
//        PidPara[Pit][Pos][I] = PidPara[Rol][Pos][I];
//        PidPara[Pit][Pos][D] = PidPara[Rol][Pos][D];
//
//        PidPara[Pit][Spd][P] = PidPara[Rol][Spd][P];
//        PidPara[Pit][Spd][I] = PidPara[Rol][Spd][I];
//        PidPara[Pit][Spd][D] = PidPara[Rol][Spd][D];
//    }
//    // 如果修改的是俯仰方向的控制参数 将修改后的结果同步到横滚方向
//    else if(Pit == Dbg_Dof)
//    {
//        PidPara[Rol][Pos][P] = PidPara[Pit][Pos][P];
//        PidPara[Rol][Pos][I] = PidPara[Pit][Pos][I];
//        PidPara[Rol][Pos][D] = PidPara[Pit][Pos][D];
//
//        PidPara[Rol][Spd][P] = PidPara[Pit][Spd][P];
//        PidPara[Rol][Spd][I] = PidPara[Pit][Spd][I];
//        PidPara[Rol][Spd][D] = PidPara[Pit][Spd][D];
//    }
//
//    for(dof=(Dof_t)0; dof<Dof_Num; dof++)
//    {
//        for(dofpid=(DofPID_t)0; dofpid<DofPID_Num; dofpid++)
//        {
//            SysPID[dof].PID[dofpid].p = PidPara[dof][dofpid][P];
//            SysPID[dof].PID[dofpid].i = PidPara[dof][dofpid][I];
//            SysPID[dof].PID[dofpid].d = PidPara[dof][dofpid][D];
//        }
//    }
//}
//
//// 向串口3发送缓存指定区域装载指定数据
//static uint16 U3_TxBuf_Load(uint16 StartID, uint8* pLoad, uint16 len)
//{
//    uint16 U3TxBuf_NextID;
//    uint16 i;
//
//    U3TxBuf_NextID = StartID + len;
//
//    if(U3TxBuf_NextID < sizeof(U3TxBuf))
//    {
//        for(i=0; i<len; i++)
//            U3TxBuf[StartID+i] = pLoad[i];
//    }
//    else
//        U3TxBuf_NextID = 0;
//
//    return U3TxBuf_NextID;
//}
//
//// 显示当前所有控制项的控制参数
//static void Show_PID_Parameter(void)
//{
//    sprintf(    (char*)&U3TxBuf,
//                "\r\n\r\nSystem PID:\r\nRol Pos P:%+7.2f I:%+7.2f D:%+7.2f Spd P:%+7.2f I:%+7.2f D:%+7.2f\r\nPit Pos P:%+7.2f I:%+7.2f D:%+7.2f Spd P:%+7.2f I:%+7.2f D:%+7.2f\r\nAlt Pos P:%+7.2f I:%+7.2f D:%+7.2f Spd P:%+7.2f I:%+7.2f D:%+7.2f\r\nYaw Pos P:%+7.2f I:%+7.2f D:%+7.2f Spd P:%+7.2f I:%+7.2f D:%+7.2f\r\n\r\n",
//
//                SysPID[Rol].PID[Pos].p,
//                SysPID[Rol].PID[Pos].i,
//                SysPID[Rol].PID[Pos].d,
//                SysPID[Rol].PID[Spd].p,
//                SysPID[Rol].PID[Spd].i,
//                SysPID[Rol].PID[Spd].d,
//
//                SysPID[Pit].PID[Pos].p,
//                SysPID[Pit].PID[Pos].i,
//                SysPID[Pit].PID[Pos].d,
//                SysPID[Pit].PID[Spd].p,
//                SysPID[Pit].PID[Spd].i,
//                SysPID[Pit].PID[Spd].d,
//
//                SysPID[Alt].PID[Pos].p,
//                SysPID[Alt].PID[Pos].i,
//                SysPID[Alt].PID[Pos].d,
//                SysPID[Alt].PID[Spd].p,
//                SysPID[Alt].PID[Spd].i,
//                SysPID[Alt].PID[Spd].d,
//
//                SysPID[Yaw].PID[Pos].p,
//                SysPID[Yaw].PID[Pos].i,
//                SysPID[Yaw].PID[Pos].d,
//                SysPID[Yaw].PID[Spd].p,
//                SysPID[Yaw].PID[Spd].i,
//                SysPID[Yaw].PID[Spd].d
//               );
//
//        Usart_Send_TxBuf_Filled_Data(&huart3, strlen((char*)U3TxBuf));
//}
//
