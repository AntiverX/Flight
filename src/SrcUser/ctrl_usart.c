/*
    文件名称：   ctrl_usart.c
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   串口接收、发送数据相关功能
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#include "ctrl_usart.h"
#include "ctrl_fmu.h"
#include "ctrl_smp.h"

extern uint8 U1TxBuf[U1_TX_BUF_LEN] = {0};  // 串口 1 发送缓存区
extern uint8 U1RxBuf[U1_RX_BUF_LEN] = {0};  // 串口 1 接收缓存区

extern uint8 U2TxBuf[U2_TX_BUF_LEN] = {0};  // 串口 2 发送缓存区
extern uint8 U2RxBuf[U2_RX_BUF_LEN] = {0};  // 串口 2 接收缓存区

extern uint8 U3TxBuf[U3_TX_BUF_LEN] = {0};  // 串口 3 发送缓存区
extern uint8 U3RxBuf[U3_RX_BUF_LEN] = {0};  // 串口 3 接收缓存区

extern YesNo_t U1_Rx_End = No;              // 串口 1 是否接收完毕指定数据
extern YesNo_t U1_Tx_End = Yes;             // 串口 1 是否发送完毕指定数据
extern YesNo_t U2_Rx_End = No;              // 串口 2 是否接收完毕指定数据
extern YesNo_t U2_Tx_End = Yes;             // 串口 2 是否发送完毕指定数据
extern YesNo_t U3_Rx_End = No;              // 串口 3 是否接收完毕指定数据
extern YesNo_t U3_Tx_End = Yes;             // 串口 3 是否发送完毕指定数据

// 串口接收缓存区内容初始化
extern void Usart_Rx_Buf_Init(void)
{
    uint16 i;
    
    for(i=0; i<U1_RX_BUF_LEN; i++)
        U1RxBuf[i] = 0;
    
    for(i=0; i<U2_RX_BUF_LEN; i++)
        U2RxBuf[i] = 0;
    
    for(i=0; i<U3_RX_BUF_LEN; i++)
        U3RxBuf[i] = 0;
}

//----------------------------------Modified Here.-----------------------------------------//
//// 修改串口通信波特率
//void Usart_Change_Baud(UART_HandleTypeDef* pHuart, uint32 NewBaud)
//{
//    pHuart->Init.BaudRate = NewBaud;
//    pHuart->Init.WordLength = UART_WORDLENGTH_8B;
//    pHuart->Init.StopBits = UART_STOPBITS_1;
//    pHuart->Init.Parity = UART_PARITY_NONE;
//    pHuart->Init.Mode = UART_MODE_TX_RX;
//    pHuart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
//    pHuart->Init.OverSampling = UART_OVERSAMPLING_16;
//
//    if (HAL_UART_Init(pHuart) != HAL_OK)
//    {
//        Error_Handler();
//    }
//}

//----------------------------------Modified Here.-----------------------------------------//
// 指定串口发送一个字节数据
//extern void Usart_Send_Byte(UART_HandleTypeDef* huart, uint8 Byte)
//{
//    uint8* pTxBuf;
//
//    if(&huart1 == huart)
//    {
//        pTxBuf = U1TxBuf;
//
//        while(No == U1_Tx_End)
//            ;
//        U1_Tx_End = No;
//    }
//    else if(&huart2 == huart)
//    {
//        pTxBuf = U2TxBuf;
//
//        while(No == U2_Tx_End)
//            ;
//        U2_Tx_End = No;
//    }
//    else if(&huart3 == huart)
//    {
//        pTxBuf = U3TxBuf;
//
//        while(No == U3_Tx_End)
//            ;
//        U3_Tx_End = No;
//    }
//
//    pTxBuf[0] = Byte;
//
//    HAL_UART_Transmit_IT(huart, pTxBuf, 1);
//}

//----------------------------------Modified Here.-----------------------------------------//
// 指定的串口 发送指定内容数据
//extern void Usart_Send_Data(UART_HandleTypeDef* huart, uint8* pTx, uint8 len)
//{
//    uint16 i;
//    uint8* pTxBuf;
//
//    if(&huart1 == huart)
//    {
//        pTxBuf = U1TxBuf;
//
//        while(No == U1_Tx_End)
//            ;
//        U1_Tx_End = No;
//    }
//    else if(&huart2 == huart)
//    {
//        pTxBuf = U2TxBuf;
//
//        while(No == U2_Tx_End)
//            ;
//        U2_Tx_End = No;
//    }
//    else if(&huart3 == huart)
//    {
//        pTxBuf = U3TxBuf;
//
//        while(No == U3_Tx_End)
//            ;
//        U3_Tx_End = No;
//    }
//
//    for(i=0; i<len; i++)
//        pTxBuf[i] = pTx[i];
//
//    HAL_UART_Transmit_IT(huart, pTxBuf, len);
//}

//----------------------------------Modified Here.-----------------------------------------//
// 指定的串口发送字符串数据
//extern void Usart_Send_String(UART_HandleTypeDef* huart, const char* pTx)
//{
//    uint16 i;
//    uint8* pTxBuf;
//
//    if(&huart1 == huart)
//    {
//        pTxBuf = U1TxBuf;
//
//        while(No == U1_Tx_End)
//            ;
//        U1_Tx_End = No;
//    }
//    else if(&huart2 == huart)
//    {
//        pTxBuf = U2TxBuf;
//
//        while(No == U2_Tx_End)
//            ;
//        U2_Tx_End = No;
//    }
//    else if(&huart3 == huart)
//    {
//        pTxBuf = U3TxBuf;
//
//        while(No == U3_Tx_End)
//            ;
//        U3_Tx_End = No;
//    }
//
//    for(i=0; i<strlen(pTx); i++)
//        pTxBuf[i] = pTx[i];
//
//    HAL_UART_Transmit_IT(huart, pTxBuf, strlen(pTx));
//}

// 指定的串口接收指定长度的数据，存放在指定的空间
//extern void Usart_Receive_IT(UART_HandleTypeDef *huart, uint8 *pRx, uint16_t len)
//{
//    uint8* pRxBuf;
//
//    if(&huart1 == huart)
//    {
//        pRxBuf = U1RxBuf;
//        U1_Rx_End = No;
//    }
//    else if(&huart2 == huart)
//    {
//        pRxBuf = U2RxBuf;
//        U2_Rx_End = No;
//    }
//    else if(&huart3 == huart)
//    {
//        pRxBuf = U3RxBuf;
//        U3_Rx_End = No;
//    }
//
//    HAL_UART_Receive_IT(huart, pRxBuf, len);
//}

//----------------------------------Modified Here.-----------------------------------------//
// 指定的串口发送缓存区已经填充过内容 按照指定的长度发送数据
//extern void Usart_Send_TxBuf_Filled_Data(UART_HandleTypeDef *huart, uint16_t len)
//{
//    uint8* pTxBuf;
//
//    if(&huart1 == huart)
//    {
//        pTxBuf = U1TxBuf;
//
//        while(No == U1_Tx_End)
//            ;
//        U1_Tx_End = No;
//    }
//    else if(&huart2 == huart)
//    {
//        pTxBuf = U2TxBuf;
//
//        while(No == U2_Tx_End)
//            ;
//        U2_Tx_End = No;
//    }
//    else if(&huart3 == huart)
//    {
//        pTxBuf = U3TxBuf;
//
//        while(No == U3_Tx_End)
//            ;
//        U3_Tx_End = No;
//    }
//
//    HAL_UART_Transmit_IT(huart, pTxBuf, len);
//}

//// 串口发送结束回调函数
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if(&huart1 == huart)
//    {
//        U1_Tx_End = Yes;
//    }
//    else if(&huart2 == huart)
//    {
//        U2_Tx_End = Yes;
//    }
//    else if(&huart3 == huart)
//    {
//        U3_Tx_End = Yes;
//    }
//}

//// 串口接收结束回调函数
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if(&huart1 == huart)
//    {
//        U1_Rx_End = Yes;
//    }
//    else if(&huart2 == huart)
//    {
//        U2_Rx_End = Yes;
//    }
//    else if(&huart3 == huart)
//    {
//        U3_Rx_End = Yes;
//    }
//}

//----------------------------------Modified Here.-----------------------------------------//
// 串口错误回调函数
//void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
//{
//    if(&huart1 == huart)
//    {
//        Usart_Receive_IT(&huart1, U1RxBuf, sizeof(Msg_FMUToCtrl_t));
//    }
//    else if(&huart2 == huart)
//    {
//        Usart_Receive_IT(&huart2, U2RxBuf, sizeof(Msg_SmpToCtrl_t));
//    }
//    else if(&huart3 == huart)
//    {
//        Usart_Receive_IT(&huart3, U3RxBuf, 1);
//    }
//}
