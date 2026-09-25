/********************************** (C) COPYRIGHT  *******************************
 * File Name          : debug.c
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
#include "debug.h"

static uint8_t p_us = 0;
static uint16_t p_ms = 0;

/*********************************************************************
 * @fn      Delay_Init
 *
 * @brief   Initializes Delay Funcation.
 *
 * @return  none
 */
void Delay_Init(void)
{
    p_us = SystemCoreClock / 8000000;
    p_ms = (uint16_t)p_us * 1000;
}

/*********************************************************************
 * @fn      Delay_Us
 *
 * @brief   Microsecond Delay Time.
 *
 * @param   n - Microsecond number.
 *
 * @return  None
 */
void Delay_Us(uint32_t n)
{
    uint32_t i;

    SysTick->SR &= ~(1 << 0);
    i = (uint32_t)n * p_us;

    SysTick->CMP = i;
    SysTick->CTLR |= (1 << 4);
    SysTick->CTLR |= (1 << 5) | (1 << 0);

    while ((SysTick->SR & (1 << 0)) != (1 << 0))
        ;
    SysTick->CTLR &= ~(1 << 0);
}

/*********************************************************************
 * @fn      Delay_Ms
 *
 * @brief   Millisecond Delay Time.
 *
 * @param   n - Millisecond number.
 *
 * @return  None
 */
void Delay_Ms(uint32_t n)
{
    uint32_t i;

    SysTick->SR &= ~(1 << 0);
    i = (uint32_t)n * p_ms;

    SysTick->CMP = i;
    SysTick->CTLR |= (1 << 4);
    SysTick->CTLR |= (1 << 5) | (1 << 0);

    while ((SysTick->SR & (1 << 0)) != (1 << 0))
        ;
    SysTick->CTLR &= ~(1 << 0);
}

/*********************************************************************
 * @fn      USART_Debug_Init
 *
 * @brief   Initializes the debug USART peripheral (TX + RX GPIO, baud rate).
 *
 * @param   baudrate - USART communication baud rate.
 *
 * @return  None
 */
void USART_Debug_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStc = {0};

#if (DEBUG == DEBUG_UART1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* PA9  = USART1_TX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA10 = USART1_RX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#elif (DEBUG == DEBUG_UART2)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#elif (DEBUG == DEBUG_UART3)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

#endif

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

#if (DEBUG == DEBUG_UART1)
    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

    NVIC_InitStc.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStc.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStc.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStc.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStc);

    USART_Cmd(USART1, ENABLE);

#elif (DEBUG == DEBUG_UART2)
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);

#elif (DEBUG == DEBUG_UART3)
    USART_Init(USART3, &USART_InitStructure);
    USART_Cmd(USART3, ENABLE);

#endif
}

/*********************************************************************
 * @fn      _write
 *
 * @brief   Support Printf Function
 *
 * @param   *buf - UART send Data.
 *          size - Data length
 *
 * @return  size: Data length
 */
__attribute__((used)) int _write(int fd, char *buf, int size)
{
    int i = 0;

    for (i = 0; i < size; i++)
    {
#if (DEBUG == DEBUG_UART1)
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
            ;
        USART_SendData(USART1, *buf++);
#elif (DEBUG == DEBUG_UART2)
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
            ;
        USART_SendData(USART2, *buf++);
#elif (DEBUG == DEBUG_UART3)
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
            ;
        USART_SendData(USART3, *buf++);
#endif
    }
    return size;
}

/*********************************************************************
 * @fn      _sbrk
 *
 * @brief   Change the spatial position of data segment.
 *
 * @return  size: Data length
 */
__attribute__((used)) void *_sbrk(ptrdiff_t incr)
{
    extern char _end[];
    extern char _heap_end[];
    static char *curbrk = _end;

    if ((curbrk + incr < _end) || (curbrk + incr > _heap_end))
        return NULL - 1;

    curbrk += incr;
    return curbrk - incr;
}

/* ====================== USART1 DMA RX (Idle Line) ====================== */

#define USART_RX_DMA_BUF_SIZE 64
static uint8_t usart_rx_dma_buf[USART_RX_DMA_BUF_SIZE];
static volatile uint8_t usart_rx_dma_done = 0;
static volatile uint16_t usart_rx_dma_len = 0;

/*********************************************************************
 * @fn      USART1_DMA_Idle_Init
 *
 * @brief   Init USART1 RX DMA (Channel5) + Idle Line interrupt.
 *          USART GPIO and mode must be configured by USART_Debug_Init first.
 */
void USART1_DMA_Idle_Init(void)
{
    DMA_InitTypeDef DMA_InitStc = {0};

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* DMA1 Channel5: USART1 RX -> usart_rx_dma_buf */
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStc.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DATAR);
    DMA_InitStc.DMA_MemoryBaseAddr = (uint32_t)usart_rx_dma_buf;
    DMA_InitStc.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStc.DMA_BufferSize = USART_RX_DMA_BUF_SIZE;
    DMA_InitStc.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStc.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStc.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStc.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStc.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStc.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStc.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStc);

    DMA_Cmd(DMA1_Channel5, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
}

/*********************************************************************
 * @fn      USART1_IRQHandler
 *
 * @brief   USART1 Idle Line ISR: record DMA byte count, restart DMA,
 *          set usart_rx_dma_done flag.
 */
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void USART1_IRQHandler(void)
{
    uint8_t clear;
    uint16_t dma_cnt;

    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        /*
         * 【关键顺序】先读 DMA 计数器，再读 DATAR 清除 IDLE 标志。
         * 若先读 DATAR，可能抢在 DMA 之前把最后一个字节读走，
         * 导致 DMA 丢字节、缓冲区内容不完整。
         */
        dma_cnt = DMA_GetCurrDataCounter(DMA1_Channel5);
        usart_rx_dma_len = USART_RX_DMA_BUF_SIZE - dma_cnt;

        /* 读 DATAR 完成 "读STATR + 读DATAR" 序列，清除 IDLE */
        clear = (uint8_t)USART1->DATAR;
        (void)clear;

        /* 重启 DMA：重置计数器 + 复位内存地址到缓冲区首地址 */
        DMA_Cmd(DMA1_Channel5, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel5, USART_RX_DMA_BUF_SIZE);
        DMA1_Channel5->MADDR = (uint32_t)usart_rx_dma_buf;
        DMA_Cmd(DMA1_Channel5, ENABLE);

        usart_rx_dma_done = 1;
    }

    /* 清除 ORE 过载错误，防止 USART 锁死不再接收 */
    if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
    {
        clear = (uint8_t)USART1->DATAR;
        (void)clear;
        USART_ClearFlag(USART1, USART_FLAG_ORE);
    }
}

/*********************************************************************
 * @fn      debug_scanf
 *
 * @brief   Non-blocking UART command parser, driven by DMA+Idle Line.
 *          Parses a complete line with vsscanf, returns 1 on success.
 *
 * @param   fmt - format string (same as sscanf)
 * @param   ... - variable arguments (pointers to receive variables)
 *
 * @return  0: no data  /  1: parsed
 */
uint8_t debug_scanf(const char *fmt, ...)
{
    if (!usart_rx_dma_done)
        return 0;

    usart_rx_dma_done = 0;

    if (usart_rx_dma_len == 0 || usart_rx_dma_len >= USART_RX_DMA_BUF_SIZE)
        return 0;

    usart_rx_dma_buf[usart_rx_dma_len] = '\0';
    printf("\r\n");

    va_list args;
    va_start(args, fmt);
    vsscanf((char *)usart_rx_dma_buf, fmt, args);
    va_end(args);

    return 1;
}