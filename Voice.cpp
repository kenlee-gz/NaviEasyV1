/******************************************************************************
 * @file     	Voice.cpp
 * @brief    
 * @version  	1.0.0
 * @date     	2016��3��18��
 *
 * @note
 *              ��ʼ����
 *              SpeechInit(io_dat, io_clk, io_en);
 *
 *              ���������
 *              Speech_Add(speech1);
 *              Speech_AddNum(58);
 *
 *              ����ѭ��������������ţ�
 *              Speech_Play();
 *
 *              ����������ţ�
 *              Speech_Clear();
 *
 * Copyright (C) 2016 YUWEI Corp. All rights reserved.
 ******************************************************************************/
#include "mbed.h"
#include "WT588D.h"
#include "Voice.h"
#include "CircularBuffer.h"

const uint32_t kVoiceBufferSize = 64;
const int32_t kVoiceAmpPowerTime = 30;
CircularBuffer<uint8_t, kVoiceBufferSize> voiceBuffer;

WT588D* pvoice;
DigitalOut* amp_en;
Timer speechTmr;

void EnableAmp()
{
    *amp_en = 1;
}

void DisableAmp()
{
    *amp_en = 0;
}

bool IsAmpEnable()
{
    return *amp_en == 1;
}

void Speech_Init(PinName dat, PinName busy, PinName en)
{
    pvoice = new WT588D(dat, busy);
    amp_en = new DigitalOut(en);
    DisableAmp();
    pvoice->Init();
    speechTmr.start();
}

void Speech_Play()
{
    uint8_t vol;

    if (!pvoice->IsBusy())
    {
        if (IsAmpEnable())
        {
            if (voiceBuffer.empty())
            {
                if(speechTmr.read_ms() > 30)
                {
                    DisableAmp();
                }
            }
            else if (speechTmr.read_ms() > kVoiceAmpPowerTime)
            {
                voiceBuffer.pop(vol);
                pvoice->play(vol);
                speechTmr.reset();
            }
        }
        else
        {
            if (!voiceBuffer.empty())
            {
                EnableAmp();
                speechTmr.reset();
            }
        }
    }
}

void Speech_Add(unsigned char speech)
{
    voiceBuffer.push(speech);
}

void Speech_Add(unsigned char* speechs, uint8_t len)
{
    for(int i = 0; i < len; i++)
    {
        voiceBuffer.push(speechs[i]);
    }
}

void Speech_SetVolume(unsigned char vol)
{
    int t = pvoice->GetLastVoiceTime();

    if(t < 90 && t >= 0)
    {
        wait_ms(90 - t);
    }

    pvoice->play(0xE0 + vol);
}

void Speech_AddNum(unsigned char value)
{
    unsigned char temp;
    if (value > 99)
    {
        // ����100���ȱ�100��Ȼ��ʮλ���ܱ�0
//        Speech_Add(kVoice100);
        Speech_Add(value/100+kVoice0);

        temp = value % 100;
        if (temp >= 10)
        {
            //����ʮλ
            Speech_Add(temp / 10 + kVoice0);
            Speech_Add(kVoice10);
            //���ظ�λ
            temp = temp % 10;
            if (temp)
                Speech_Add(temp + kVoice0);
        }
        else if(temp > 0)
        {
            Speech_Add(kVoice0);
            Speech_Add(temp + kVoice0);
        }
    }
    else
    {
        // С��100
        if (value >= 10)
        {   //����ʮλ
            Speech_Add((value / 10) + kVoice0);
            Speech_Add(kVoice10);
            //���ظ�λ
            temp = value % 10;
            if (temp)
                Speech_Add(temp + kVoice0);
        }
        else
        {
            //���ظ�λ
            Speech_Add(value + kVoice0);
        }
    }
}

void Speech_Clear()
{
    voiceBuffer.reset();
}

