/******************************************************************************
 * @file     WT588D.cpp
 * @brief    正常工作耗电4.5mA，RESET拉低为690uA。而功放最大电流可以高至270mA。
 * @version  1.0.0
 * @date     2015年1月21日
 *
 * @note
 * Copyright (C) 2014 YUWEI Corp. All rights reserved.
 ******************************************************************************/
#include "WT588D.h"

WT588D::WT588D(PinName dat, PinName busy) :
		sdaIO(dat), busyIO(busy), voice_index_()
{
}

WT588D::~WT588D()
{
}

void WT588D::Init()
{
	sdaIO = 0;
	busyIO.mode(PullUp);
    tmr.start();
}

void WT588D::SetDatHigh()
{
	sdaIO = 1;
}

void WT588D::SetDatLow()
{
    sdaIO = 0;
}

void WT588D::OneWireDrive()
{
    __disable_irq();
    for (int i = 0; i < 8; i++)
    {
        sdaIO = 1;
        if (voice_index_ & 1)
        {
            wait_us(250); /* 600us */
            sdaIO = 0;
            wait_us(100); /* 200us */
        }
        else
        {
            wait_us(100); /*  200us */
            sdaIO = 0;
            wait_us(250); /* 600us */
        }
        voice_index_ >>= 1;
    }
    sdaIO = 1;
    __enable_irq();
	tmr.reset();
}

/**
 * 芯片在发送语音信号后，延时33ms才会出现BUSY为低电平。
 * @param addr
 */
void WT588D::play(uint8_t addr)
{
#if 1
	unsigned char i;
	sdaIO = 0;
	wait_ms(5); /* delay 5ms */
	// 因单线通讯不能被打断，因此要禁止中断
	__disable_irq();
	for (i = 0; i < 8; i++)
	{
		sdaIO = 1;
		if (addr & 1)
		{
			wait_us(250); /* 600us */
			sdaIO = 0;
			wait_us(100); /* 200us */
		}
		else
		{
			wait_us(100); /*  200us */
			sdaIO = 0;
			wait_us(250); /* 600us */
		}
		addr >>= 1;
	}
	sdaIO = 1;
	__enable_irq();
#else
	sdaIO = 0;
	voice_index_ = addr;
	tmr_.attach_us(this, &WT588D::OneWireDrive, 5000);
#endif
	tmr.reset();
}

/**
 *
 * @return
 */
bool WT588D::IsBusy()
{
	return (tmr.read_ms() < 100) || busyIO == 0;
}

int WT588D::GetLastVoiceTime()
{
    return tmr.read_ms();
}
