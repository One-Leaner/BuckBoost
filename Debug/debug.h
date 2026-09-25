/********************************** (C) COPYRIGHT  *******************************
 * File Name          : debug.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : This file contains all the functions prototypes for UART
 *                      Printf , Delay functions.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "stdlib.h"
#include "ch32v30x.h"

/* UART Printf Definition */
#define DEBUG_UART1 1
#define DEBUG_UART2 2
#define DEBUG_UART3 3

/* DEBUG UATR Definition */
#ifndef DEBUG
#define DEBUG DEBUG_UART1
#endif

    void Delay_Init(void);
    void Delay_Us(uint32_t n);
    void Delay_Ms(uint32_t n);
    void USART_Debug_Init(uint32_t baudrate);
    void USART1_DMA_Idle_Init(void);
    uint8_t debug_scanf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif