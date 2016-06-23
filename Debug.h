/******************************************************************************
 * @file     	Debug.h
 * @brief    
 * @version  	1.0.0
 * @date     	2015��8��22��
 *
 * @note
 * Copyright (C) 2015 YUWEI Corp. All rights reserved.
 ******************************************************************************/

#ifndef DEBUG_H_
#define DEBUG_H_

#define UART_DEBUG 0

#if UART_DEBUG == 1

// Serial��printf���ܵ�������!��ȷ���Ƿ���retarget����(fflush����)
#include "mbed.h"
extern Serial* debug_uart;
//#define        DBG(format, ...)   printf(format, __VA_ARGS__)
#define        DBG(...)     debug_uart->printf(__VA_ARGS__)
#define        DBGC(c)      debug_uart->putc(c)
#define        DBGS(c)      debug_uart->puts(c)
#define        DBGARR(array, cnt)      do   {   \
    for (int i = 0; i < (cnt); i++) \
    {   debug_uart->printf(" %02hhX", array[i]); } \
    debug_uart->puts("\r\n"); \
    }while(0)

#else

#define        DBG(...)
#define        DBGC(c)
#define        DBGS(c)
#define        DBGARR(array, cnt)

#endif

#endif /* DEBUG_H_ */
