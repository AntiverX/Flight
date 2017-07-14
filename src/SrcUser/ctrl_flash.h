/*
    文件名称：   ctrl_flash.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   Flash 读写相关，主要用于保存遥控器微调参数
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __CTRL_FLASH_H__
#define __CTRL_FLASH_H__

#include "ctrl_basic.h"

#define RC_OF_SAVED         0xAAAA      // 已经保存过微调值
#define RC_OF_NOT_SAVED     0x5555      // 没有保存过微调值

typedef struct
{
    uint16 IsSaved;             // 0xAAAA: Saved.   0x5555: Not Saved.
    int16 RC_Rol_Offset;
    int16 RC_Pit_Offset;
    
    uint16 CRCVal;              // ^ Front Data.
}RC_Offset_t;

extern YesNo_t En_RC_Offset_Update;             // 是否可以更新遥控器微调值
extern YesNo_t Saving_RC_Offset;                // 是否正在保存遥控器微调值

extern void Key_PowerOn_Event_Handle(void);     // 当上电时已经有按键按下，保存遥控器值到 Flash
extern YesNo_t Save_RC_Offset_To_Flash(int16 OF_Rol, int16 OF_Pit);     // 保存遥控器微调值到 Flash
extern YesNo_t Read_RC_Offset_From_Flash(RC_Offset_t* pOffset);          // 从 Flash 读取遥控器微调值

#endif  // __CTRL_FLASH_H__

