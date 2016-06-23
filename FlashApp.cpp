/******************************************************************************
 * @file     	FlashApp.cpp
 * @brief    
 * @version  	1.0.0
 * @date     	2015Äê12ÔÂ4ÈÕ
 *
 * @note
 * Copyright (C) 2015 YUWEI Corp. All rights reserved.
 ******************************************************************************/

#include "mbed.h"
#include "FlashApp.h"

bool EraseFlash(int Address)
{
    uint32_t PageError;
    FLASH_EraseInitTypeDef EraseInitStruct;

    HAL_FLASH_Unlock();

    EraseInitStruct.TypeErase = TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = Address;
    EraseInitStruct.NbPages = 1;

    bool result = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) == HAL_OK;

    HAL_FLASH_Lock();
    return result;
}

bool WriteToFlash(int Address, const void* buf, int len)
{
    Timer tmr;
    const uint8_t* pbuf = (const uint8_t*) buf;
    uint32_t PageError;
    uint32_t DATA_16;
    int pos = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;

    HAL_FLASH_Unlock();

#if 0
    EraseInitStruct.TypeErase = TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = Address;
    EraseInitStruct.NbPages = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
    }
#endif
    /* Program the user Flash area word by word
     (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    tmr.start();
    while (pos < len && tmr.read_ms() < 1000)
    {
        DATA_16 = *(pbuf + pos);
        DATA_16 |= (*(pbuf + pos + 1)) << 8;
        if (HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, Address, DATA_16) == HAL_OK)
        {
            Address += 2;
            pos += 2;
        }
    }

    HAL_FLASH_Lock();
    return pos >= len;
}

int ReadFromFlash(int addr, uint8_t* buf, int len)
{
    uint8_t* pbuf = (uint8_t*) buf;
    for (int i = 0; i < len; i++)
    {
        *pbuf++ = *(uint8_t*) (addr + i);
    }
    return 0;
}
