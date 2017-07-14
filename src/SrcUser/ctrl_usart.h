/*
    文件名称：   ctrl_usart.h
    文件作者：   中科浩电 www.bj-zkhd.com
    文件功能：   串口接收、发送数据相关功能
    修改日期：   2017-7-7
    修改内容：   修改注释
*/

#ifndef __CTRL_USART_H__
#define __CTRL_USART_H__

#include "ctrl_basic.h"
#include "string.h"
//#include "usart.h"

#define USART_TX_BUF_LEN    300             // 串口发送缓存区空间大小
#define USART_RX_BUF_LEN    300             // 串口接收缓存区空间大小

#define U1_TX_BUF_LEN   USART_TX_BUF_LEN    // 串口 1 发送缓存区大小
#define U1_RX_BUF_LEN   USART_RX_BUF_LEN    // 串口 1 接收缓存区大小

#define U2_TX_BUF_LEN   USART_TX_BUF_LEN    // 串口 2 发送缓存区大小
#define U2_RX_BUF_LEN   USART_RX_BUF_LEN    // 串口 2 接收缓存区大小

#define U3_TX_BUF_LEN   USART_TX_BUF_LEN    // 串口 3 发送缓存区大小
#define U3_RX_BUF_LEN   USART_RX_BUF_LEN    // 串口 3 接收缓存区大小

extern uint8 U1TxBuf[U1_TX_BUF_LEN];        // 串口 1 发送缓存区
extern uint8 U1RxBuf[U1_RX_BUF_LEN];        // 串口 1 接收缓存区
                                            
extern uint8 U2TxBuf[U2_TX_BUF_LEN];        // 串口 2 发送缓存区
extern uint8 U2RxBuf[U2_RX_BUF_LEN];        // 串口 2 接收缓存区
                                            
extern uint8 U3TxBuf[U3_TX_BUF_LEN];        // 串口 3 发送缓存区
extern uint8 U3RxBuf[U3_RX_BUF_LEN];        // 串口 3 接收缓存区

extern YesNo_t U1_Rx_End;                   // 串口 1 是否接收完毕指定数据
extern YesNo_t U1_Tx_End;                   // 串口 1 是否发送完毕指定数据
extern YesNo_t U2_Rx_End;                   // 串口 2 是否接收完毕指定数据
extern YesNo_t U2_Tx_End;                   // 串口 2 是否发送完毕指定数据
extern YesNo_t U3_Rx_End;                   // 串口 3 是否接收完毕指定数据
extern YesNo_t U3_Tx_End;                   // 串口 3 是否发送完毕指定数据

extern void Usart_Rx_Buf_Init(void);                                                // 串口接收缓存区内容初始化
//extern void Usart_Change_Baud(UART_HandleTypeDef* pHuart, uint32 NewBaud);          // 修改串口通信波特率
//extern void Usart_Send_Byte(UART_HandleTypeDef* huart, uint8 Byte);                 // 指定串口发送一个字节数据
//extern void Usart_Send_Data(UART_HandleTypeDef* huart, uint8* pTx, uint8 len);      // 指定的串口 发送指定内容数据
//extern void Usart_Send_String(UART_HandleTypeDef* huart, const char* pTx);          // 指定的串口发送字符串数据
//extern void Usart_Receive_IT(UART_HandleTypeDef *huart, uint8 *pRx, uint16_t len);  // 指定的串口接收指定长度的数据，存放在指定的空间
//extern void Usart_Send_TxBuf_Filled_Data(UART_HandleTypeDef *huart, uint16_t len);  // 指定的串口发送缓存区已经填充过内容 按照指定的长度发送数据

#endif  // __CTRL_USART_H__
