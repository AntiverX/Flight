/*
    文件名称：   ctrl_control.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   系统控制具体怎么实现的
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#include "ctrl_control.h"
#include "ctrl_smp.h"
#include "ctrl_fmu.h"
#include "ctrl_smp.h"
#include "ctrl_key.h"
#include "ctrl_debug.h"
#include "ctrl_drone_status.h"
#include "ctrl_encoder.h"
#include "ctrl_flash.h"
#include "ctrl_led.h"
#include "ctrl_beep.h"
#include "ctrl_usart.h"
#include "r_cg_macrodriver.h"
#include "r_cg_port.h"
#include "r_cg_sci.h"

// 定义、初始化 Demo 状态
extern Demo_St_Act_t Demo_St_Act = Demo_St_Act_Climb;
extern Demo_StToSp_Act_t Demo_StToSp_Act = Demo_StToSp_Act_Climb;
extern Demo_StLdLu_Act_t Demo_StLdLu_Act = Demo_StLdLu_Act_Climb;

extern int16 RC_Offset_Rol = 0;                         // 遥控器横滚方向微调值
extern int16 RC_Offset_Pit = 0;                         // 遥控器俯仰方向微调值
extern uint16 HomeHeight_cm = 18;                       // Home 高度(飞机放置在地面上通过自身传感器测量到的高度值)

extern YesNo_t DroneLanding = No;                       // 是否在降落
extern YesNo_t MsgToFMU_Lock = No;                      // 是否要发送锁定信息

extern Demo_t SysDemo = Demo_St;                        // 初始化默认 Demo
extern DemoCtrl_t DemoCtrlType = DemoCtrl_RC_On;        // 一键启动解锁后 是否要等待遥控器打开后才继续执行 Demo
extern Demo_St_LostInfo_t Demo_LostInfo = {No};         // 特征点丢失后，追踪、找回信息
extern uint16 Demo_Act_Num = 0;                         // Demo 一共有多少个状态(动作)
extern uint16 Demo_Act_Cnt = 0;                         // Demo 现在在执行那个状态(动作)

extern NodeShape_t NodeShape_Curt = NodeShape_Nothing;   // 初始化 什么也看不到
extern NodeShape_t NodeShape_Last = NodeShape_Nothing;   // 初始化 什么也看不到
extern int32 FindStartPoint_ms_Left = -1;               // 还有多长时间找起点，如果找不到就放弃(初始化成小于0的值)
extern uint16 NodeShapeProb[NodeShape_Num] = {0};       // 特征点概率法匹配

static RC_Offset_t RC_Offset = {No};                    // 遥控器微调参数值(操作 Flash 时用到)

// 更新 Demo 的 PID 设定值
static void SysCtrl_Update_PID_Set_Find_St(void);
static void SysCtrl_Update_PID_Set_Demo_St(void);
static void SysCtrl_Update_PID_Set_Demo_StToSp(void);
static void SysCtrl_Update_PID_Set_Demo_StLdLu(void);

// 更新 Demo 的状态(动作)
static void SysCtrl_Update_Act_Find_St(void);
static void SysCtrl_Update_Act_Demo_St(void);
static void SysCtrl_Update_Act_Demo_StToSp(void);
static void SysCtrl_Update_Act_Demo_StLdLu(void);

// 更新 Demo 丢失追踪信息
static void SysCtrl_Update_Lost_Info_Find_St(void);
static void SysCtrl_Update_Lost_Info_Demo_St(void);
static void SysCtrl_Update_Lost_Info_Demo_StToSp(void);
static void SysCtrl_Update_Lost_Info_Demo_StLdLU(void);

// 修改 PID 运算结果
static void SysCtrl_Update_PID_Change_Find_St(void);
static void SysCtrl_Update_PID_Change_Demo_St(void);
static void SysCtrl_Update_PID_Change_Demo_StToSp(void);
static void SysCtrl_Update_PID_Change_Demo_StLdLU(void);

static void SysCtrl_Update_PID_Set_Find_St(void)
{
    ;
}

static void SysCtrl_Update_PID_Set_Demo_St(void)
{
    switch(Demo_St_Act)
    {
        case Demo_St_Act_Climb:     {}  break;
        case Demo_St_Act_Stable:    {}  break;
        case Demo_St_Act_Hover:     {}  break;
        case Demo_St_Act_Land:
        {
            SysPID[Alt].PID[Pos].set = SysPID[Alt].PID[Pos].sample - 80;
            SysPID[Alt].PID[Spd].set = -5;
        }
        break;
        case Demo_St_Act_Lock:      {}  break;
    }
}

static void SysCtrl_Update_PID_Set_Demo_StToSp(void)
{
    switch(Demo_StToSp_Act)
    {
        case Demo_StToSp_Act_Climb:     {}  break;
        case Demo_StToSp_Act_Stable:    {}  break;
        case Demo_StToSp_Act_StHover:     {}  break;
        case Demo_StToSp_Act_ToSp:      {}  break;
        case Demo_StToSp_Act_Land:
        {
            SysPID[Alt].PID[Pos].set = SysPID[Alt].PID[Pos].sample - 80;
            SysPID[Alt].PID[Spd].set = -5;
        }
        break;
        case Demo_StToSp_Act_Lock:      {}  break;
    }
}

static void SysCtrl_Update_PID_Set_Demo_StLdLu(void)
{
    switch(Demo_StLdLu_Act)
    {
        case Demo_StLdLu_Act_Climb:     {}  break;
        case Demo_StLdLu_Act_Stable:    {}  break;
        case Demo_StLdLu_Act_Hover:     {}  break;
        case Demo_StLdLu_Act_ToLd:      {}  break;
        case Demo_StLdLu_Act_LdHover:   {}  break;
        case Demo_StLdLu_Act_ToLu:      {}  break;
        case Demo_StLdLu_Act_Land:
        {
            SysPID[Alt].PID[Pos].set = SysPID[Alt].PID[Pos].sample - 80;
            SysPID[Alt].PID[Spd].set = -5;
        }
        break;
        case Demo_StLdLu_Act_Lock:      {}  break;
    }
}

static void SysCtrl_Update_Act_Find_St(void)
{
    NodeShape_t shape_i;
    NodeShape_t shape_new = NodeShape_Nothing;
    
    // 只有当高度大于一定范围才进行控制(否则高度过低情况下 镜头看到的因为白平衡不稳定)
    if((FindStartPoint_ms_Left<0) && (Msg_FMUToCtrl.Pkg.Alt_Sonar>FIND_START_POINT_CM_MIN))
    {
        FindStartPoint_ms_Left = FIND_START_POINT_MS_MAX;
        //Beep_On();  // 开始寻找起点后蜂鸣器会响起
    }
    
    // 在寻找起点的过程中 如果连续一段时间没有找到就放弃
    if((FindStartPoint_ms_Left>0) && (NodeShape_Curt!=NodeShape_StartPoint))
    {
        // 每当采集到新的图像 先将原来的记录信息队列调整一个节奏(类似做先出先进处理 此处为先出操作)
        for(shape_i=NodeShape_Nothing; shape_i<NodeShape_Num; shape_i++)
        {
            if(NodeShapeProb[shape_i] > 0)
            {
                NodeShapeProb[shape_i]--;
            }
        }
        
        // 判断当前图像看到的形状
        if(Yes == Is_On_Node(No, No, Yes, Yes))         // 横线
        {
            NodeShapeProb[NodeShape_Horizontal]++;
        }    
        else if(Yes == Is_On_Node(Yes, Yes, No, No))    // 竖线
        {
            NodeShapeProb[NodeShape_Vertical]++;
        }    
        else if(Yes == Is_On_Node(Yes, No, No, Yes))    // 左下角
        {
            NodeShapeProb[NodeShape_LeftDown]++;
        }    
        else if(Yes == Is_On_Node(Yes, No, Yes, No))    // 右下角
        {
            NodeShapeProb[NodeShape_RightDown]++;
        }
        else if(Yes == Is_On_Node(Yes, No, Yes, Yes))   // 起点
        {
            NodeShapeProb[NodeShape_StartPoint]++;
        }
        else	
        {
            NodeShapeProb[NodeShape_Nothing]++;
        }
        
        // 一次采集、分析后 检查有没有缓存溢出的(实际上不会溢出 只是上了上限)
        for(shape_i=NodeShape_Nothing; shape_i<NodeShape_Num; shape_i++)
        {
            if(NodeShapeProb[shape_i] > FOUND_SHAPE_CACHE_MAX)
            {
                NodeShapeProb[shape_i] = FOUND_SHAPE_CACHE_MAX;
            }
        }
        
        // 判断最有可能的位置
        for(shape_i=NodeShape_Nothing; shape_i<NodeShape_Num; shape_i++)
        {
            if(NodeShapeProb[shape_i] > NodeShapeProb[shape_new])
            {
                shape_new = shape_i;
            }
        }
        
        // 每当发现新的节点 更新节点状态 (横线和竖线是路线  不是节点形状)
        if(shape_new!=NodeShape_Curt)
        {
            if(     (shape_new != NodeShape_Horizontal)
                &&  (shape_new != NodeShape_Vertical)
                &&  (NodeShape_Curt != NodeShape_Horizontal)
                &&  (NodeShape_Curt != NodeShape_Vertical)  )
            {
                NodeShape_Last = NodeShape_Curt;
                NodeShape_Curt = shape_new;
            }
            else
            {
                NodeShape_Curt = shape_new;
            }
        }
        
//        if(NodeShape_StartPoint == NodeShape_Curt)
//            //Beep_Off();
//        	;
    }
    // 超时
    else
    {
        //Beep_Off();
    }
}

static void SysCtrl_Update_Act_Demo_St(void)
{
    static uint32 Hover_Start_Stamp;
    uint32 Sys_ms_Now;
    switch(Demo_St_Act)
    {
        case Demo_St_Act_Climb:
        {
            if(     (SysPID[Alt].PID[Pos].err_abs)
                &&  (SysPID[Alt].PID[Pos].err_abs<10)    )
            {

                Demo_St_Act = Demo_St_Act_Stable;
                Demo_LostInfo.FeatureValid = No;
            }
        }
        break;
        case Demo_St_Act_Stable:
        {
            if(     (SysPID[Rol].PID[Pos].err_abs)
                &&  (SysPID[Rol].PID[Pos].err_abs < 50)
                &&  (SysPID[Pit].PID[Pos].err_abs)
                &&  (SysPID[Pit].PID[Pos].err_abs < 50)
                &&  (SysPID[Alt].PID[Pos].err_abs)
                &&  (SysPID[Alt].PID[Pos].err_abs < 10)
                &&  (SysPID[Rol].PID[Spd].err_abs)
                &&  (SysPID[Rol].PID[Spd].err_abs < 2)
                &&  (SysPID[Pit].PID[Spd].err_abs)
                &&  (SysPID[Pit].PID[Spd].err_abs < 2)  )
            {
            	BEEP_IO_ON;
                Demo_St_Act = Demo_St_Act_Hover;
                Demo_LostInfo.FeatureValid = No;
                Hover_Start_Stamp = Sys_ms;
            }
        }
        break;
        case Demo_St_Act_Hover:
        {
            Sys_ms_Now = Sys_ms;
            if((Sys_ms_Now-Hover_Start_Stamp) > (5*1000))
            {
                #if(DBG)
                #else
                    Demo_St_Act = Demo_St_Act_Land;
                    Demo_LostInfo.FeatureValid = No;
                    DroneLanding = Yes;
                #endif //DBG
            }
        }
        break;
        case Demo_St_Act_Land:
        {
            if(Get_Height() < HomeHeight_cm+5)
            {
                Demo_St_Act = Demo_St_Act_Lock;
                MsgToFMU_Lock = Yes;
                Demo_LostInfo.FeatureValid = No;
            }
        }
        break;
        case Demo_St_Act_Lock:
        {
        }
        break;
    }
    
    Demo_Act_Cnt = Demo_St_Act;
}

static void SysCtrl_Update_Act_Demo_StToSp(void)
{
    static uint32 Hover_Start_Stamp;
    uint32 Sys_ms_Now;
    static uint16 SpPoint_Cnt = 0;
    
    switch(Demo_StToSp_Act)
    {
        case Demo_StToSp_Act_Climb:
        {
            if(     (SysPID[Alt].PID[Pos].err_abs)
                &&  (SysPID[Alt].PID[Pos].err_abs<10)
                &&  (SysPID[Alt].PID[Spd].err_abs)
                &&  (SysPID[Alt].PID[Spd].err_abs<1)    )
            {
                Demo_StToSp_Act = Demo_StToSp_Act_Stable;
                Demo_LostInfo.FeatureValid = No;
            }
        }
        break;
        case Demo_StToSp_Act_Stable:
        {
            if(     (SysPID[Rol].PID[Pos].err_abs)
                &&  (SysPID[Rol].PID[Pos].err_abs < 50)  //之前是40
                &&  (SysPID[Pit].PID[Pos].err_abs)
                &&  (SysPID[Pit].PID[Pos].err_abs < 50)  //之前是40
                &&  (SysPID[Alt].PID[Pos].err_abs)
                &&  (SysPID[Alt].PID[Pos].err_abs < 10)  )
            {
                Demo_StToSp_Act = Demo_StToSp_Act_StHover;
                Demo_LostInfo.FeatureValid = No;
                Hover_Start_Stamp = Sys_ms;
            }
        }
        break;
        case Demo_StToSp_Act_StHover:
        {
            Sys_ms_Now = Sys_ms;
            if(     ((Sys_ms_Now-Hover_Start_Stamp) > (3*1000))
                &&  (Is_On_Node(Yes, No, Yes, Yes))
                &&  (SysPID[Rol].PID[Pos].err_abs)
                &&  (SysPID[Rol].PID[Pos].err_abs < 30) )
            {
                Demo_StToSp_Act = Demo_StToSp_Act_ToSp;
                Demo_LostInfo.FeatureValid = No;
            }
        }
        break;
        case Demo_StToSp_Act_ToSp:
        {
            if(Is_On_Node(No, Yes, Yes, Yes))
            {
                SpPoint_Cnt++;
                if(SpPoint_Cnt >= 2)
                {
                    Demo_StToSp_Act = Demo_StToSp_Act_SpHover;
                    Demo_LostInfo.FeatureValid = No;
                }
            }
            else
            {
                SpPoint_Cnt = 0;
            }
        }
        break;
        case Demo_StToSp_Act_SpHover:
        {
            if(     (SysPID[Rol].PID[Pos].err_abs)
                &&  (SysPID[Rol].PID[Pos].err_abs < 40)
                &&  (SysPID[Pit].PID[Pos].err_abs)
                &&  (SysPID[Pit].PID[Pos].err_abs < 40)
                &&  (SysPID[Alt].PID[Pos].err_abs)
                &&  (SysPID[Alt].PID[Pos].err_abs < 10)  )
            {
                if(Is_On_Node(No, Yes, Yes, Yes))
                {
                    Demo_StToSp_Act = Demo_StToSp_Act_Land;
                    Demo_LostInfo.FeatureValid = No;
                    DroneLanding = Yes;
                }
            }
        }
        break;
        case Demo_StToSp_Act_Land:
        {
            if(Get_Height() < HomeHeight_cm+5)
            {
                Demo_StToSp_Act = Demo_StToSp_Act_Lock;
                MsgToFMU_Lock = Yes;
                Demo_LostInfo.FeatureValid = No;
            }
        }
        break;
        case Demo_StToSp_Act_Lock:
        {
        }
        break;
    }
    
    Demo_Act_Cnt = Demo_StToSp_Act;
}

static void SysCtrl_Update_Act_Demo_StLdLu(void)
{
    static uint32 Hover_Start_Stamp;
    static uint32 Hover_Ld_Stamp;
    uint32 Sys_ms_Now;
    static uint16 SpPoint_Cnt = 0;
    static uint16 Lu_Cnt = 0;
    
    switch(Demo_StLdLu_Act)
    {
        case Demo_StLdLu_Act_Climb:
        {
            if(     (SysPID[Alt].PID[Pos].err_abs)
                &&  (SysPID[Alt].PID[Pos].err_abs<10)
                &&  (SysPID[Alt].PID[Spd].err_abs)
                &&  (SysPID[Alt].PID[Spd].err_abs<1)    )
            {
                Demo_StLdLu_Act = Demo_StLdLu_Act_Stable;
                Demo_LostInfo.FeatureValid = No;
            }
        }
        break;
        case Demo_StLdLu_Act_Stable:
        {
            if(     (SysPID[Rol].PID[Pos].err_abs)
                &&  (SysPID[Rol].PID[Pos].err_abs < 40)
                &&  (SysPID[Pit].PID[Pos].err_abs)
                &&  (SysPID[Pit].PID[Pos].err_abs < 40)
                &&  (SysPID[Alt].PID[Pos].err_abs)
                &&  (SysPID[Alt].PID[Pos].err_abs < 10)  )
            {
                Demo_StLdLu_Act = Demo_StLdLu_Act_Hover;
                Demo_LostInfo.FeatureValid = No;
                Hover_Start_Stamp = Sys_ms;
            }
        }
        break;
        case Demo_StLdLu_Act_Hover:
        {
            Sys_ms_Now = Sys_ms;
            if(     ((Sys_ms_Now-Hover_Start_Stamp) > (3*1000))
                &&  (Is_On_Node(Yes, No, Yes, Yes))
                &&  (SysPID[Pit].PID[Pos].err_abs)
                &&  (SysPID[Pit].PID[Pos].err_abs < 30) )
            {
                Demo_StLdLu_Act = Demo_StLdLu_Act_ToLd;
                Demo_LostInfo.FeatureValid = No;
            }
        }
        break;
        case Demo_StLdLu_Act_ToLd:
        {
            if(Is_On_Node(Yes, No, No, Yes))
            {
                SpPoint_Cnt++;
                if(SpPoint_Cnt >= 2)
                {
                    Demo_StLdLu_Act = Demo_StLdLu_Act_LdHover;
                    Demo_LostInfo.FeatureValid = No;
                    Hover_Ld_Stamp = Sys_ms;
                }
            }
            else
            {
                SpPoint_Cnt = 0;
            }
        }
        break;
        case Demo_StLdLu_Act_LdHover:
        {
            Sys_ms_Now = Sys_ms;
            if(     ((Sys_ms_Now-Hover_Ld_Stamp) > (3*1000))
                &&  (Is_On_Node(Yes, No, No, Yes))
                &&  (SysPID[Rol].PID[Pos].err_abs)
                &&  (SysPID[Rol].PID[Pos].err_abs < 30) )
            {
                Demo_StLdLu_Act = Demo_StLdLu_Act_ToLu;
                Demo_LostInfo.FeatureValid = No;
            }
        }
        break;
        case Demo_StLdLu_Act_ToLu:
        {
            if(Is_On_Node(No, Yes, No, Yes))
            {
                Lu_Cnt++;
                if(Lu_Cnt >= 2)
                {
                    Demo_StLdLu_Act = Demo_StLdLu_Act_Land;
                    Demo_LostInfo.FeatureValid = No;
                }
            }
            else
            {
                Lu_Cnt = 0;
            }
        }
        break;
        case Demo_StLdLu_Act_Land:
        {
            if(Get_Height() < HomeHeight_cm+5)
            {
                Demo_StLdLu_Act = Demo_StLdLu_Act_Lock;
                MsgToFMU_Lock = Yes;
                Demo_LostInfo.FeatureValid = No;
            }
        }
        break;
        case Demo_StLdLu_Act_Lock:
        {
        }
        break;
    }
    
    Demo_Act_Cnt = Demo_StLdLu_Act;
}

static void SysCtrl_Update_Lost_Info_Find_St(void)
{
    Demo_LostInfo.FeatureValid = No;
}

static void SysCtrl_Update_Lost_Info_Demo_St(void)
{
    Demo_LostInfo.FeatureValid = No;
    
    if(Is_On_Node(Yes, No, Yes, Yes))
    {
        Demo_LostInfo.FeatureValid = Yes;
        Demo_LostInfo.RecordValid = Yes;
        
        Demo_LostInfo.Record[Rol][Pos] = SysPID[Rol].PID[Pos].err;
        Demo_LostInfo.Record[Rol][Spd] = SysPID[Rol].PID[Spd].err;
        
        Demo_LostInfo.Record[Pit][Pos] = SysPID[Pit].PID[Pos].err;
        Demo_LostInfo.Record[Pit][Spd] = SysPID[Pit].PID[Spd].err;
        
        Demo_LostInfo.Record[Alt][Pos] = SysPID[Alt].PID[Pos].err;
        Demo_LostInfo.Record[Alt][Spd] = SysPID[Alt].PID[Spd].err;
        
        Demo_LostInfo.Record[Yaw][Pos] = SysPID[Yaw].PID[Pos].err;
        Demo_LostInfo.Record[Yaw][Spd] = SysPID[Yaw].PID[Spd].err;
        
        Demo_LostInfo.Lost_ms = 0;
    }
}

static void SysCtrl_Update_Lost_Info_Demo_StToSp(void)
{
    Demo_LostInfo.FeatureValid = No;
    
    switch(Demo_StToSp_Act)
    {
        case Demo_StToSp_Act_Climb:
        case Demo_StToSp_Act_Stable:
        case Demo_StToSp_Act_StHover:
        {
            if(Yes == Is_On_Node(Yes, No, Yes, Yes))
                Demo_LostInfo.FeatureValid = Yes;
        }
        break;
        case Demo_StToSp_Act_ToSp:
        {
            if(Yes == Msg_SmpToCtrl.Pkg.Valid_Rol_Pos)
                Demo_LostInfo.FeatureValid = Yes;
        }
        break;
        case Demo_StToSp_Act_SpHover:
        {
            if(Yes == Is_On_Node(No, Yes, Yes, Yes))
                Demo_LostInfo.FeatureValid = Yes;
        }
        break;
        case Demo_StToSp_Act_Land:
        {
            if(Yes == Is_On_Node(No, Yes, Yes, Yes))
                Demo_LostInfo.FeatureValid = Yes;
        }
        break;
        case Demo_StToSp_Act_Lock:
        {
        }
        break;
    }
    
    if(Yes == Demo_LostInfo.FeatureValid)
    {
        Demo_LostInfo.RecordValid = Yes;
        
        Demo_LostInfo.Record[Rol][Pos] = SysPID[Rol].PID[Pos].err;
        Demo_LostInfo.Record[Rol][Spd] = SysPID[Rol].PID[Spd].err;
        
        Demo_LostInfo.Record[Pit][Pos] = SysPID[Pit].PID[Pos].err;
        Demo_LostInfo.Record[Pit][Spd] = SysPID[Pit].PID[Spd].err;
        
        Demo_LostInfo.Record[Alt][Pos] = SysPID[Alt].PID[Pos].err;
        Demo_LostInfo.Record[Alt][Spd] = SysPID[Alt].PID[Spd].err;
        
        Demo_LostInfo.Record[Yaw][Pos] = SysPID[Yaw].PID[Pos].err;
        Demo_LostInfo.Record[Yaw][Spd] = SysPID[Yaw].PID[Spd].err;
        
        Demo_LostInfo.Lost_ms = 0;
    }
}

static void SysCtrl_Update_Lost_Info_Demo_StLdLU(void)
{
    Demo_LostInfo.FeatureValid = No;
    
    switch(Demo_StLdLu_Act)
    {
        case Demo_StLdLu_Act_Climb:
        case Demo_StLdLu_Act_Stable:
        case Demo_StLdLu_Act_Hover:
        {
            if(Yes == Is_On_Node(Yes, No, Yes, Yes))
                Demo_LostInfo.FeatureValid = Yes;
        }
        break;
        case Demo_StLdLu_Act_ToLd:
        {
            if(Yes == Msg_SmpToCtrl.Pkg.Valid_Pit_Pos)
                Demo_LostInfo.FeatureValid = Yes;
        }
        break;
        case Demo_StLdLu_Act_LdHover:
        {
            if(Yes == Is_On_Node(Yes, No, No, Yes))
                Demo_LostInfo.FeatureValid = Yes;
        }
        break;
        case Demo_StLdLu_Act_ToLu:
        {
            if(Yes == Msg_SmpToCtrl.Pkg.Valid_Rol_Pos)
                Demo_LostInfo.FeatureValid = Yes;
        }
        break;
        case Demo_StLdLu_Act_Land:
        {
            if(Yes == Is_On_Node(No, Yes, No, Yes))
                Demo_LostInfo.FeatureValid = Yes;
        }
        break;
        case Demo_StLdLu_Act_Lock:
        {
        }
        break;
    }
    
    if(Yes == Demo_LostInfo.FeatureValid)
    {
        Demo_LostInfo.RecordValid = Yes;
        
        Demo_LostInfo.Record[Rol][Pos] = SysPID[Rol].PID[Pos].err;
        Demo_LostInfo.Record[Rol][Spd] = SysPID[Rol].PID[Spd].err;
        
        Demo_LostInfo.Record[Pit][Pos] = SysPID[Pit].PID[Pos].err;
        Demo_LostInfo.Record[Pit][Spd] = SysPID[Pit].PID[Spd].err;
        
        Demo_LostInfo.Record[Alt][Pos] = SysPID[Alt].PID[Pos].err;
        Demo_LostInfo.Record[Alt][Spd] = SysPID[Alt].PID[Spd].err;
        
        Demo_LostInfo.Record[Yaw][Pos] = SysPID[Yaw].PID[Pos].err;
        Demo_LostInfo.Record[Yaw][Spd] = SysPID[Yaw].PID[Spd].err;
        
        Demo_LostInfo.Lost_ms = 0;
    }
}

static void SysCtrl_Update_PID_Change_Find_St(void)
{
    if(FindStartPoint_ms_Left > 0)
    {
        switch(NodeShape_Curt)
        {
            case NodeShape_Nothing:                                 // 现在什么也看不见
            {
                if(FindStartPoint_ms_Left > 0)
                    SysPID[Pit].Out_Int16 = -100;
            }
            break;
            case NodeShape_Horizontal:                              // 现在能看见横线
            {
                if(     (NodeShape_Nothing == NodeShape_Last)       // 上一刻看不见节点
                    ||  (NodeShape_RightDown == NodeShape_Last) )   // 上一刻看见右下角
                {
                    SysPID[Rol].Out_Int16 = -100;                   // 向左走
                }
                else if(NodeShape_LeftDown == NodeShape_Last)       // 上一刻看见左下角
                {
                    SysPID[Rol].Out_Int16 = 100;                    // 向右走
                }
                
                if((Yes==Msg_SmpToCtrl.Pkg.Valid_L) && (Yes==Msg_SmpToCtrl.Pkg.Valid_R))
                    SysPID[Rol].Out_Int16 = -100;
            }
            break;
            case NodeShape_Vertical:                                // 现在能看见竖线
            {
                SysPID[Pit].Out_Int16 = 100;                        // 向后走
            }
            break;
            case NodeShape_LeftDown:                                // 现在能看见左下角
            {
                SysPID[Rol].Out_Int16 = 100;                         // 向右走
            }
            break;
            case NodeShape_RightDown:                               // 现在能看见右下角
            {
                SysPID[Rol].Out_Int16 = -100;                       // 向左走
            }
            break;
        }
    }
}

static void SysCtrl_Update_PID_Change_Demo_St(void)
{
	switch(Demo_St_Act)
	{
		case Demo_St_Act_Climb:
		{

		}
		break;
	}
}

static void SysCtrl_Update_PID_Change_Demo_StToSp(void)
{
    switch(Demo_StToSp_Act)
    {
		case Demo_StToSp_Act_Climb:
		{

		}
		break;
        case Demo_StToSp_Act_ToSp:
        {
            SysPID[Pit].Out_Int16 = -55;
        }
        break;
    }
}

static void SysCtrl_Update_PID_Change_Demo_StLdLU(void)
{
    switch(Demo_StLdLu_Act)
    {
        case Demo_StLdLu_Act_ToLd:
        {
            SysPID[Rol].Out_Int16 = -60;
        }
        break;
        case Demo_StLdLu_Act_ToLu:
        {
            SysPID[Pit].Out_Int16 = -60;
        }
        break;
    }
}

// 初始化控制板发送给飞控的消息
extern void SysCtrl_Msg_Ctrl_To_FMU_Init(void)
{
    // 消息序列置零
    Msg_CtrlToFMU.Pkg.Seq = 0;
}

// 初始化遥控器微调参数 当选择的是 Demo4、Demo5、Deomo6 默认遥控器是全程关闭
// 所以需要从 Flash 中读取出事先保存好的微调参数
extern void SysCtrl_Modify_RC_Offset(void)
{
    if(Yes == Read_RC_Offset_From_Flash(&RC_Offset))
    {
        RC_Offset_Rol = RC_Offset.RC_Rol_Offset;
        RC_Offset_Pit = RC_Offset.RC_Pit_Offset;
        //----------------------------------Modified Here.-----------------------------------------//
        //PORTA.PODR.BIT.B3 = 0;
        //PORT9.PODR.BIT.B4 = 1;
//        LED_R_IO_OFF;
//        LED_B_IO_ON;
    }
    else
    {
    	//----------------------------------Modified Here.-----------------------------------------//
    	//PORTA.PODR.BIT.B3 = 1;
    	//PORT9.PODR.BIT.B4 = 0;
    	//        LED_R_IO_ON;
//        LED_B_IO_OFF;
//        while(1)
//            ;
    }
}

// 根据上电时 编码器的初始值 决定执行那个程控任务
extern void SysCtrl_Set_Demo_By_Encoder(void)
{
    switch(Get_Encoder_Value())
    {
        case 1:
        {
            SysDemo = Demo_St;
            Demo_Act_Num = Demo_St_Act_Num;
        }
        break;
        case 2:
        {
            SysDemo = Demo_StToSp;
            Demo_Act_Num = Demo_StToSp_Act_Num;
        }
        break;
        case 3:
        {
            SysDemo = Demo_StLdLu;
            Demo_Act_Num = Demo_StLdLu_Act_Num;
        }
        break;
    }
}

// 为了提高稳定性，起飞后发现起点才执行程控任务，此函数用于初始化找到起点的相关参数
extern void SysCtrl_Init_Find_Start_Point(void)
{
    uint16 i;
    
    for(i=0; i<NodeShape_Num; i++)
        NodeShapeProb[i] = 0;
    
    NodeShapeProb[NodeShape_Nothing] = FOUND_SHAPE_CACHE_MAX;    // 上电状态是什么特征点也没找到
}

// 更新系统 PID 控制
extern void SysCtrl_Update_PID(void)
{
    if(Yes == En_Update_PID)                // 当到达新的控制周期
    {
        En_Update_PID = No;                 // 复位控制周期控制标志
        
        SysCtrl_Update_PID_Set();           // 更新 PID 设定值(目标值)
        SysCtrl_Update_PID_Sample();        // 更新 PID 采样值
        SysCtrl_Update_PID_Error();         // 更新 PID 误差
        SysCtrl_Update_PID_Calculate();     // 更新 PID 计算
        SysCtrl_Update_PID_Change();        // 修改 PID 输出
    }
}

extern void SysCtrl_Update_PID_Set(void)
{
    SysPID[Rol].PID[Pos].set = 320 / 2;
    SysPID[Rol].PID[Spd].set = SysPID[Rol].PID[Pos].err / SysPID[Rol].K_PosErr_To_SpdSet;

    SysPID[Pit].PID[Pos].set = 240 / 2;
    SysPID[Pit].PID[Spd].set = SysPID[Pit].PID[Pos].err / SysPID[Pit].K_PosErr_To_SpdSet;

    SysPID[Alt].PID[Pos].set = 80;
    SysPID[Alt].PID[Spd].set = SysPID[Alt].PID[Pos].err / SysPID[Alt].K_PosErr_To_SpdSet;

    SysPID[Yaw].PID[Pos].set = 0;
    SysPID[Yaw].PID[Spd].set = SysPID[Yaw].PID[Pos].err / SysPID[Yaw].K_PosErr_To_SpdSet;
    

    if(NodeShape_Curt == NodeShape_StartPoint)
    {
        switch(SysDemo)
        {
            case Demo_St:       SysCtrl_Update_PID_Set_Demo_St();       break;
            case Demo_StToSp:   SysCtrl_Update_PID_Set_Demo_StToSp();   break;
            case Demo_StLdLu:   SysCtrl_Update_PID_Set_Demo_StLdLu();   break;
        }
    }
    else
    {
        SysCtrl_Update_PID_Set_Find_St();
    }
}

extern void SysCtrl_Update_PID_Sample(void)
{
    PID_Update_Alt_Sample(Msg_FMUToCtrl.Pkg.Alt_Sonar);
    
    if(Get_Height() > 35)
        AutoCtrl_HeightAbove_35cm = Yes;
    else
        AutoCtrl_HeightAbove_35cm = No;
    
    PID_Update_Sample(&SysPID[Rol].PID[Pos], Msg_SmpToCtrl.Pkg.Smp_Rol_Pos, (YesNo_t)Msg_SmpToCtrl.Pkg.Valid_Rol_Pos);
    PID_Update_Sample(&SysPID[Rol].PID[Spd], Msg_SmpToCtrl.Pkg.Smp_Rol_Spd, (YesNo_t)Msg_SmpToCtrl.Pkg.Valid_Rol_Spd);
    
    PID_Update_Sample(&SysPID[Pit].PID[Pos], Msg_SmpToCtrl.Pkg.Smp_Pit_Pos, (YesNo_t)Msg_SmpToCtrl.Pkg.Valid_Pit_Pos);
    PID_Update_Sample(&SysPID[Pit].PID[Spd], Msg_SmpToCtrl.Pkg.Smp_Pit_Spd, (YesNo_t)Msg_SmpToCtrl.Pkg.Valid_Pit_Spd);
    
    PID_Update_Sample(&SysPID[Yaw].PID[Pos], Msg_SmpToCtrl.Pkg.Smp_Yaw_Pos, (YesNo_t)Msg_SmpToCtrl.Pkg.Valid_Yaw_Pos);
    PID_Update_Sample(&SysPID[Yaw].PID[Spd], Msg_SmpToCtrl.Pkg.Smp_Yaw_Spd, (YesNo_t)Msg_SmpToCtrl.Pkg.Valid_Yaw_Spd);
    
    PID_Update_Sample(&SysPID[Alt].PID[Pos], Dof_Alt_Smp.PosSmp.sample, Dof_Alt_Smp.PosSmp.valid);
    PID_Update_Sample(&SysPID[Alt].PID[Spd], Dof_Alt_Smp.SpdSmp.sample, Dof_Alt_Smp.SpdSmp.valid);
}

extern void SysCtrl_Update_PID_Error(void)
{
    #if(DBG)
        Demo_LostInfo.FeatureValid = Yes;
    #endif //DBG

    if(Yes == Demo_LostInfo.FeatureValid)
    {
        PID_Update_Error(&SysPID[Rol].PID[Pos], SysPID[Rol].PID[Pos].valid);
        PID_Update_Error(&SysPID[Rol].PID[Spd], SysPID[Rol].PID[Spd].valid);
        
        PID_Update_Error(&SysPID[Pit].PID[Pos], SysPID[Pit].PID[Pos].valid);
        PID_Update_Error(&SysPID[Pit].PID[Spd], SysPID[Pit].PID[Spd].valid);
        
        PID_Update_Error(&SysPID[Yaw].PID[Pos], SysPID[Yaw].PID[Pos].valid);
        PID_Update_Error(&SysPID[Yaw].PID[Spd], SysPID[Yaw].PID[Spd].valid);
    }
    else
    {
        if(     (Yes == Demo_LostInfo.RecordValid)
            &&  (Demo_LostInfo.Lost_ms < LOST_CNT_MAX)  )
        {
            SysPID[Rol].PID[Pos].err_p = Demo_LostInfo.Record[Rol][Pos];
            SysPID[Rol].PID[Pos].err_i = 0;
            SysPID[Rol].PID[Pos].err_d = 0;
            
            SysPID[Pit].PID[Pos].err_p = Demo_LostInfo.Record[Pit][Pos];
            SysPID[Pit].PID[Pos].err_i = 0;
            SysPID[Pit].PID[Pos].err_d = 0;
            
            SysPID[Yaw].PID[Pos].err_p = 0;
            SysPID[Yaw].PID[Pos].err_i = 0;
            SysPID[Yaw].PID[Pos].err_d = 0;
        }
        else
        {
            SysPID[Rol].PID[Pos].err_p = 0;
            SysPID[Rol].PID[Pos].err_i = 0;
            SysPID[Rol].PID[Pos].err_d = 0;
            
            SysPID[Pit].PID[Pos].err_p = 0;
            SysPID[Pit].PID[Pos].err_i = 0;
            SysPID[Pit].PID[Pos].err_d = 0;
            
            SysPID[Yaw].PID[Pos].err_p = 0;
            SysPID[Yaw].PID[Pos].err_i = 0;
            SysPID[Yaw].PID[Pos].err_d = 0;
        }
    }
    
    PID_Update_Error(&SysPID[Alt].PID[Pos], SysPID[Alt].PID[Pos].valid);
    PID_Update_Error(&SysPID[Alt].PID[Spd], SysPID[Alt].PID[Spd].valid);
}

extern void SysCtrl_Update_PID_Calculate(void)
{
    #define LOST_OUT_SCALE      (0.4)
    
    Dof_PID_Update_Calculate(&SysPID[Rol]);
    Dof_PID_Update_Calculate(&SysPID[Pit]);
    Dof_PID_Update_Calculate(&SysPID[Alt]);
    Dof_PID_Update_Calculate(&SysPID[Yaw]);
    
    if(Yes == Demo_LostInfo.FeatureValid)
    {
    }
    else
    {
        if(SysPID[Rol].Out_Float > (LOST_OUT_SCALE*SysPID[Rol].Out_Max))
            SysPID[Rol].Out_Float = (LOST_OUT_SCALE*SysPID[Rol].Out_Max);
        if(SysPID[Rol].Out_Float < (LOST_OUT_SCALE*SysPID[Rol].Out_Min))
            SysPID[Rol].Out_Float = (LOST_OUT_SCALE*SysPID[Rol].Out_Min);
        
        if(SysPID[Pit].Out_Float > (LOST_OUT_SCALE*SysPID[Pit].Out_Max))
            SysPID[Pit].Out_Float = (LOST_OUT_SCALE*SysPID[Pit].Out_Max);
        if(SysPID[Pit].Out_Float < (LOST_OUT_SCALE*SysPID[Pit].Out_Min))
            SysPID[Pit].Out_Float = (LOST_OUT_SCALE*SysPID[Pit].Out_Min);
        
        SysPID[Rol].Out_Int16 = SysPID[Rol].Out_Float;
        SysPID[Pit].Out_Int16 = SysPID[Pit].Out_Float;
        SysPID[Alt].Out_Int16 = SysPID[Alt].Out_Float;
        SysPID[Yaw].Out_Int16 = SysPID[Yaw].Out_Float;
    }
}

extern void SysCtrl_Update_PID_Change(void)
{
    #if(DBG)
    NodeShape_Curt = NodeShape_StartPoint;
    #endif

    if(NodeShape_Curt == NodeShape_StartPoint)
    {
        switch(SysDemo)
        {
            case Demo_St:       SysCtrl_Update_PID_Change_Demo_St();        break;
            case Demo_StToSp:   SysCtrl_Update_PID_Change_Demo_StToSp();    break;
            case Demo_StLdLu:   SysCtrl_Update_PID_Change_Demo_StLdLU();    break;
        }
    }
    else
    {
        SysCtrl_Update_PID_Change_Find_St();
    }
}

// 更新系统控制状态(动作)
extern void SysCtrl_Update_Act(void)
{
    // 当控制状态更新使能有效
    if(Yes == En_Update_Act)
    {

        // 复位标志位
        En_Update_Act = No;
        
        // 如果需要等待遥控器开启程控使能 但目前未使能程控
        if((DemoCtrl_RC_On==DemoCtrlType) && (No==AutoCtrl_RC_Ch6))
        {
            // 不更新状态(不做任何事情 等待通过遥控器开启程控)
            return;
        }

        // 如果已经到达起始点位置 则根据旋转编码器设定的 Demo 执行程控任务
        #if(DBG)
            NodeShape_Curt = NodeShape_StartPoint;
		#endif //DBG
        
        if(NodeShape_Curt == NodeShape_StartPoint)
        {
            switch(SysDemo)
            {
                case Demo_St:       SysCtrl_Update_Act_Demo_St();        break;
                case Demo_StToSp:   SysCtrl_Update_Act_Demo_StToSp();    break;
                case Demo_StLdLu:   SysCtrl_Update_Act_Demo_StLdLu();    break;
           }
        }
       // 否则先找起点
        else
        {
            SysCtrl_Update_Act_Find_St();
        }
    }
}

// 更新特征点丢失后追踪、找回信息
extern void SysCtrl_Update_Lost_Info(void)
{
    #if(DBG)
        NodeShape_Curt = NodeShape_StartPoint;
    #endif
    
    if(NodeShape_Curt == NodeShape_StartPoint)
    {
        switch(SysDemo)
        {
            case Demo_St:       SysCtrl_Update_Lost_Info_Demo_St();        break;
            case Demo_StToSp:   SysCtrl_Update_Lost_Info_Demo_StToSp();    break;
            case Demo_StLdLu:   SysCtrl_Update_Lost_Info_Demo_StLdLU();    break;
        }
    }
    else
    {
        SysCtrl_Update_Lost_Info_Find_St();
    }
}

// 控制板向飞控发送指定类型的消息
extern void SysCtrl_Update_Msg_Ctrl_To_FMU_Type(Msg_CtrlToFMU_Type_t MsgType)
{
    Msg_CtrlToFMU.Pkg.St = 0xAAAA;
    Msg_CtrlToFMU.Pkg.Sp = 0x5555;
    
    Msg_CtrlToFMU.Pkg.ms = Sys_ms;
    Msg_CtrlToFMU.Pkg.Seq++;
    
    switch(MsgType)
    {
        case Auto_Off:
        {
            Msg_CtrlToFMU.Pkg.Rol = 0;
            Msg_CtrlToFMU.Pkg.Pit = 0;
            Msg_CtrlToFMU.Pkg.Alt = 0;
            Msg_CtrlToFMU.Pkg.Yaw = 0;
            Msg_CtrlToFMU.Pkg.Mod = 0;
            Msg_CtrlToFMU.Pkg.Cmd = Cmd_Nothing;
        }
        break;
        case Unlock_RightDown:
        {
            Msg_CtrlToFMU.Pkg.Rol = 0;
            Msg_CtrlToFMU.Pkg.Pit = 0;
            Msg_CtrlToFMU.Pkg.Alt = 0;
            Msg_CtrlToFMU.Pkg.Yaw = 4500;
            Msg_CtrlToFMU.Pkg.Mod = 0;
            Msg_CtrlToFMU.Pkg.Cmd = Cmd_Unlocking;
        }
        break;
        case Unlock_ThrMid:
        {
            Msg_CtrlToFMU.Pkg.Rol = 0;
            Msg_CtrlToFMU.Pkg.Pit = 0;
            Msg_CtrlToFMU.Pkg.Alt = 556;
            Msg_CtrlToFMU.Pkg.Yaw = 0;
            Msg_CtrlToFMU.Pkg.Mod = 0;
            Msg_CtrlToFMU.Pkg.Cmd = Cmd_Unlocking;
        }
        break;
        case Auto_On:
        {
            #if(DBG)
                AutoCtrl_HeightAbove_35cm = Yes;
            #endif
            
            if(Yes == AutoCtrl_HeightAbove_35cm)
            {
                Msg_CtrlToFMU.Pkg.Rol = SysPID[Rol].Out_Int16 + -40;
                Msg_CtrlToFMU.Pkg.Pit = SysPID[Pit].Out_Int16 + -30;
                //Msg_CtrlToFMU.Pkg.Rol = SysPID[Rol].Out_Int16 + RC_Offset_Rol;
                //Msg_CtrlToFMU.Pkg.Pit = SysPID[Pit].Out_Int16 + RC_Offset_Pit;
                Msg_CtrlToFMU.Pkg.Yaw = SysPID[Yaw].Out_Int16;
            }
            else if(No == AutoCtrl_HeightAbove_35cm && No == DroneLanding)
            {
            	//微调
            	//Msg_CtrlToFMU.Pkg.Rol = SysPID[Rol].Out_Int16 + -40;
                //Msg_CtrlToFMU.Pkg.Pit = -30;
            }
            else
            {
                Msg_CtrlToFMU.Pkg.Rol = 0;
                Msg_CtrlToFMU.Pkg.Pit = 0;
                Msg_CtrlToFMU.Pkg.Yaw = 0;
            }
            
            if(Yes == DroneLanding)
                Msg_CtrlToFMU.Pkg.Alt = SysPID[Alt].Out_Int16 - (80-Get_Height());
            else
                Msg_CtrlToFMU.Pkg.Alt = SysPID[Alt].Out_Int16;
            
            Msg_CtrlToFMU.Pkg.Mod = 0;
            Msg_CtrlToFMU.Pkg.Cmd = Cmd_Nothing;
        }
        break;
        case Lock:
        {
            Msg_CtrlToFMU.Pkg.Rol = 0;
            Msg_CtrlToFMU.Pkg.Pit = 0;
            Msg_CtrlToFMU.Pkg.Alt = -556;
            Msg_CtrlToFMU.Pkg.Yaw = 0;
            Msg_CtrlToFMU.Pkg.Mod = 450 - 250;    // 1:0 2:250 3:450 4:599 5:750 6:1000
            Msg_CtrlToFMU.Pkg.Cmd = Cmd_Nothing;
        }
        break;
        default:
        {
            Msg_CtrlToFMU.Pkg.Rol = 0;
            Msg_CtrlToFMU.Pkg.Pit = 0;
            Msg_CtrlToFMU.Pkg.Alt = 0;
            Msg_CtrlToFMU.Pkg.Yaw = 0;
            Msg_CtrlToFMU.Pkg.Mod = 0;
            Msg_CtrlToFMU.Pkg.Cmd = Cmd_Nothing;
        }
        break;
    }
}

// 更新控制板发送给飞控的信息
extern void SysCtrl_Update_Msg_Ctrl_To_FMU(void)
{
    if(Yes == En_SendMsgToFMU_WhenSmpUpdate)
    {
        En_SendMsgToFMU_WhenSmpUpdate = No;
        
        if(Yes == StartUnlock_By_Key)
        {
            StartUnlock_By_Key = No;
            Drone_Unlock_ms = 0;
            DroneUnlocking = Yes;
        }
        
        if(Yes == DroneUnlocking)
        {
            SysCtrl_Unlock_FMU(Drone_Unlock_ms);
        }
        else
        {
            if(DemoCtrl_RC_On == DemoCtrlType)
            {
                if(Yes == AutoCtrl_RC_Ch6)
                {
                    if(Yes == MsgToFMU_Lock)
                    {
                        SysCtrl_Update_Msg_Ctrl_To_FMU_Type(Lock);
                    }
                    else
                    {
                        SysCtrl_Update_Msg_Ctrl_To_FMU_Type(Auto_On);
                    }
                }
                else
                {
                    SysCtrl_Update_Msg_Ctrl_To_FMU_Type(Auto_Off);
                }
            }
            else if(DemoCtrl_RC_Off == DemoCtrlType)
            {
                if(Yes == MsgToFMU_Lock)
                {
                    SysCtrl_Update_Msg_Ctrl_To_FMU_Type(Lock);
                }
                else
                {
                    SysCtrl_Update_Msg_Ctrl_To_FMU_Type(Auto_On);
                }
            }
            else
            {
                SysCtrl_Update_Msg_Ctrl_To_FMU_Type(Auto_Off);
            }
        }
        
        R_SCI1_Serial_Send(Msg_CtrlToFMU.Buf,sizeof(Msg_CtrlToFMU_t));
    }
}

// 程控解锁飞控
extern void SysCtrl_Unlock_FMU(uint32 UnlockLasted_ms)
{
    if(UnlockLasted_ms < UNLOCK_SF_MS)
    {
        ;   // do nothing, just delay serval second for safety.
    }
    else if(UnlockLasted_ms < UNLOCK_RD_MS)
    {
        SysCtrl_Update_Msg_Ctrl_To_FMU_Type(Unlock_RightDown);
    }
    else if(UnlockLasted_ms < UNLOCK_TM_MS)
    {
        SysCtrl_Update_Msg_Ctrl_To_FMU_Type(Unlock_ThrMid);
    }
    else
    {
        DroneUnlocking = No;
    }
}

// 判断能否某个指定的特征点
extern YesNo_t Is_On_Node(YesNo_t F, YesNo_t B, YesNo_t L, YesNo_t R)
{
    if(F != Msg_SmpToCtrl.Pkg.Valid_F)
        return No;
    if(B != Msg_SmpToCtrl.Pkg.Valid_B)
        return No;
    if(L != Msg_SmpToCtrl.Pkg.Valid_L)
        return No;
    if(R != Msg_SmpToCtrl.Pkg.Valid_R)
        return No;
    
    return Yes;
}

