/******************************************************************************
 * @file     WT588D.h
 * @brief    
 * @version  1.0.0
 * @date     2015年1月21日
 *
 * @note
 * Copyright (C) 2014 YUWEI Corp. All rights reserved.
 ******************************************************************************/
#pragma once

#include "mbed.h"

class WT588D
{
private:
	DigitalOut sdaIO;
	DigitalIn busyIO;
	uint8_t voice_index_;
	Timer tmr;
	Timeout tmr_;
	void OneWireDrive();

public:
	WT588D(PinName dat, PinName busy);
	virtual ~WT588D();

	void play(uint8_t addr);

	bool IsBusy();

	void Init();
	void SetDatLow();
	void SetDatHigh();
	int GetLastVoiceTime(); //!<获取上一次语音播放时间
};


