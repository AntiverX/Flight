/*
    文件名称：   ctrl_flash.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   Flash 读写相关，主要用于保存遥控器微调参数
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#include "ctrl_flash.h"
#include "ctrl_key.h"
#include "ctrl_led.h"
#include "ctrl_fmu.h"

#include "platform.h"
#include "r_flash_rx_if.h"
#include "r_flash_rx_config.h"
#include "r_flash_rx23t.h"

// 1024 Byte for User, at the last page(1kB(1024 Byte) per page).
#define USER_FLASH_ADDR     (FLASH_CF_BLOCK_0)       // 用户 Flash 数据 起始地址
#define USER_FLASH_SAVE_MAX (1024 - 4)                              // 用户 Flash 数据 最大长度

extern YesNo_t En_RC_Offset_Update = No;        // 是否可以更新遥控器微调值
extern YesNo_t Saving_RC_Offset = No;           // 是否正在保存遥控器微调值

static YesNo_t Flash_Erase_User_Zone(void);     // 擦除用户 Flash 区内容

// 擦除用户 Flash 区内容
static YesNo_t Flash_Erase_User_Zone(void)
{
	flash_err_t err;
	flash_res_t result;
	err = R_FLASH_Erase((flash_block_address_t)USER_FLASH_ADDR, 1);
	if (FLASH_SUCCESS != err)
		return No;
	err = R_FLASH_BlankCheck((uint32_t)USER_FLASH_ADDR, FLASH_CF_BLOCK_SIZE, &result);
	if ((err != FLASH_SUCCESS) || (result != FLASH_RES_BLANK))
		return No;
	return Yes;

	//----------------Original Code----------------/
    /*FLASH_EraseInitTypeDef FLASH_Erase_UserZone;
    uint32 PageError;
    YesNo_t OptResult;
    
    OptResult = No;
    
    FLASH_Erase_UserZone.TypeErase = FLASH_TYPEERASE_PAGES;    
    FLASH_Erase_UserZone.PageAddress = USER_FLASH_ADDR;
    FLASH_Erase_UserZone.NbPages = 1;
    
    if(HAL_OK == HAL_FLASHEx_Erase(&FLASH_Erase_UserZone, &PageError))
        OptResult = Yes;
    return OptResult;*/
}

// 保存遥控器的微调参数到 Flash ，以便执行全程控任务时，加以利用，提高稳定性
extern void Key_PowerOn_Event_Handle(void)
{
    /*
        本函数的作用是将遥控的微调参数保存到 Flash 中
        每架飞机并非装好后就平稳，需要通过遥控器上的微调开关来调整
        遥控器微调过的飞机的微调参数会保存在遥控器当中
        控制板可以通过串口从飞控获取遥控器的输入量
        当遥控器摇杆置中时，遥控器的舵量中仅包含微调量，控制板就可以获取到微调量
        每架飞机的微调参数并非固定、统一，一般都不一定和其他的飞机相同
        所以通过本程序 将遥控器的微调参数保存到 Flash 中
        这样，在遥控器关闭时，执行程控任务时，仍可在输出量上叠加微调量，提高稳定
        
        
        将遥控器微调参数保存到 Flash 中的方法：
            1 遥控器开启、摇杆置中(打开电源开关 放一边就行了)
            2 等待飞控自检完毕
            3 看到控制板上的蓝色 LED 在闪烁(控制板和飞控通信状态指示灯 闪烁表示通信正常)
            4 按住控制板上的 复位键
            5 按住控制板上的 SW1 或 SW2 键
            6 松开控制板复位键
            7 松开刚才按下的按键
            8 看到蓝色 LED 闪烁若干次(5次)
            9 若成功的将遥控器微调参数保存到了控制板的 Flash ，蓝色 LED 常亮，否则红色 LED 常亮。
    */
    
    #define AVG_NUM     10      // 对于每项微调值通过多少次来求平均值
    
    int16 i;
    int16 Sum_Rol_Offset;
    int16 Sum_Pit_Offset;
    
    if((Key_Pressed==Key[SW1].PowerOn) || (Key_Pressed==Key[SW2].PowerOn))
    {
    	PORTA.PODR.BIT.B3 = 1;//LED_R_IO_ON;
        PORT9.PODR.BIT.B4 = 0;//LED_B_IO_OFF;
        
        Sum_Rol_Offset = 0;
        Sum_Pit_Offset = 0;
        
        for(i=0; i<AVG_NUM; i++)
        {
            while(No == Msg_FMU_To_Ctrl_Update())
                ;
            
            Sum_Rol_Offset += Msg_FMUToCtrl.Pkg.RC_Ch_01;
            Sum_Pit_Offset += Msg_FMUToCtrl.Pkg.RC_Ch_02;
        }
        
        if(Yes == Save_RC_Offset_To_Flash(Sum_Rol_Offset/AVG_NUM, Sum_Pit_Offset/AVG_NUM))
        {
            PORTA.PODR.BIT.B3 = 0;
            PORT9.PODR.BIT.B4 = 1;
//            LED_R_IO_OFF;
//            LED_B_IO_ON;
        }
        else
        {
        	PORTA.PODR.BIT.B3 = 1;
        	PORT9.PODR.BIT.B4 = 0;
//            LED_R_IO_ON;
//            LED_B_IO_OFF;
        }
        
        /*
            保存微调参数时 遥控器必须开启
            遥控器开启 程控无法解锁
            所以不必往下执行
            以便观察最后的指示灯状态
        */
        while(1)
            ;
    }
}

// 将遥控器微调值保存到 Flash 中具体操作
extern YesNo_t Save_RC_Offset_To_Flash(int16 OF_Rol, int16 OF_Pit)
{
    static RC_Offset_t RC_Offset_Cache;
    flash_err_t err;
    
    RC_Offset_Cache.IsSaved = RC_OF_SAVED;
    RC_Offset_Cache.RC_Rol_Offset = OF_Rol;
    RC_Offset_Cache.RC_Pit_Offset = OF_Pit;
    RC_Offset_Cache.CRCVal = OF_Rol ^ OF_Pit;

    // 1 Open Code Flash  打开代码Flash
    err = R_FLASH_Open();
    if (FLASH_SUCCESS != err)
    	return No;

    // 2 Erase Code Flash  擦除用户区(擦除时总是按块 整块擦除)
    if (No == Flash_Erase_User_Zone())
    	return No;

    // 3 Write to Flash 写数据到 Flash 中
    err = R_FLASH_Write((uint32_t)&RC_Offset_Cache, (uint32_t)USER_FLASH_ADDR, sizeof(RC_Offset_t));
    if(FLASH_SUCCESS != err)
    	return No;
    return Yes;
    /*******************Original Code*******************
    // 1 Unlock Flash   解锁飞控
    if(HAL_OK != HAL_FLASH_Unlock())
        return No;
    
    // 2 Erase Flash    擦除用户区(擦除时总是按块 整块擦除)
    if(No == Flash_Erase_User_Zone())
        return No;
    
    // 3 Write to Flash 写数据到 Flash 中
    NeedWordNum = sizeof(RC_Offset_t) / 4;
    if(sizeof(RC_Offset_t) % 4)
        NeedWordNum += 1;
    
    pWord = (uint32*)&RC_Offset_Cache;
    
    
    for(i=0; i<NeedWordNum; i++)
    {
        if(HAL_OK != HAL_FLASH_Program(TYPEPROGRAM_WORD, USER_FLASH_ADDR+(i*4), pWord[i]))
            return No;
    }
    
    // 4 Lock Flash     锁定 Flash 曹组
    if(HAL_OK != HAL_FLASH_Lock())
        return No;
    
    return Yes;
     *******************Original Code*******************/
}

// 从 Flash 中读取遥控器微调值
extern YesNo_t Read_RC_Offset_From_Flash(RC_Offset_t* pOffset)
{
    RC_Offset_t* pOffsetCache;
    
    pOffsetCache = (RC_Offset_t*)USER_FLASH_ADDR;
    
    // 通过自定义的校验方式检查 用户自定义 Flash 区域是否正确保存了遥控器微调参数
    if(     (RC_OF_SAVED == pOffsetCache->IsSaved)
        &&  ((uint16)pOffsetCache->CRCVal == (uint16)(pOffsetCache->RC_Rol_Offset ^ pOffsetCache->RC_Pit_Offset))   )
    {
        pOffset->IsSaved =          pOffsetCache->IsSaved;
        pOffset->RC_Rol_Offset =    pOffsetCache->RC_Rol_Offset;
        pOffset->RC_Pit_Offset =    pOffsetCache->RC_Pit_Offset;
        pOffset->CRCVal =           pOffsetCache->CRCVal;
        
        return Yes;
    }
    
    return No;
}

