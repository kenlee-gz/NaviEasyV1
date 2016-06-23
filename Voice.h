/******************************************************************************
 * @file     	Voice.h
 * @brief    
 * @version  	1.0.0
 * @date     	2015年11月6日
 *
 * @note
 * Copyright (C) 2015 YUWEI Corp. All rights reserved.
 ******************************************************************************/

#ifndef VOICE_H_
#define VOICE_H_

/**
 * 领航易装的语音
 */
enum VoiceIndex
{
    kVoiceWifiOK = 0,
    kVoiceDing = 1,
    kVoiceW41 = 2,
    kVoiceW42 = 3,
    kVoiceW43 = 4,
    kVoiceW44 = 5,
    kVoiceW45 = 6,
    kVoiceW46 = 7,
    kVoiceW47 = 8,
    kVoice0 = 9,
    kVoice1 = 10,
    kVoice2 = 11,
    kVoice3 = 12,
    kVoice4 = 13,
    kVoice5 = 14,
    kVoice6 = 15,
    kVoice7 = 16,
    kVoice8 = 17,
    kVoice9 = 18,
    kVoice10 = 19,
    kVoice11 = 20,
    kVoice12 = 21,
    kVoice13 = 22,
    kVoice14 = 23,
    kVoice15 = 24,
    kVoice16 = 25,
    kVoice17 = 26,
    kVoice18 = 27,
    kVoice19 = 28,
    kVoice20 = 29,
    kVoice21 = 30,
    kVoiceTiao = 31,
    kVoiceTong = 32,
    kVoiceWan = 33,
    kVoiceDa = 34,
    kVoiceEmpty = 35,
    kVoicePlaying = 36,
    kVoiceHuPai = 37,
    kVoiceZiZhu = 38,
    kVoiceError = 39,
    kVoiceStart = 40,
    kVoiceChaiDui = 41,
    kVoicePeng0 = 42,
    kVoicePeng1 = 43,
    kVoicePeng2 = 44,
    kVoicePeng3 = 45,
    kVoiceZhua = 46,
    kVoiceHouHu = 47,
    kVoiceHouHu2 = 48,
    kVoice5W = 49,
    kVoiceRingDelete = 50,
    kVoiceBadCardIn6 = 51,
    kVoiceBadCard = 52,
    kVoiceZhang = 53,
    kVoiceRingHave = 54,
    kVoiceIdDupli = 55,
    kVoiceZZ0 = 56,
    kVoiceZZ1 = 57,
    kVoiceZZ2 = 58,
    kVoiceZZ3 = 59,
    kVoiceGo = 60,
    kVoiceDaHu3 = 61,
    kVoiceDaHu4 = 62,
    kVoiceDaHu5 = 63,
    kVoiceDaHu6 = 64,
    kVoiceDaHu7 = 65,
    kVoiceDaHu8 = 66,
    kVoiceDaHu9 = 67,
    kVoiceDaHu10 = 68,
    kVoiceDaHu11 = 69,
    kVoiceDaHu12 = 70,
    kVoicePingHu = 71,
    kVoiceDianGangQuPai = 72,
    kVoiceHouGangQuPai = 73,
    kVoiceBuy = 74,
    kVoiceCS888 = 75,
    kVoiceNotHu = 76,
    kVoiceTing = 77,
    kVoiceWZKJ = 78,
    kVoiceHu = 79,
    kVoiceDui = 80,
    kVoiceGang = 81,
    kVoiceZhuangJia = 82,
    kVoiceCai = 83,
    kVoiceZiMo = 84,
    kVoiceFangPeng = 85,
    kVoicePengZhuan = 86,
    kVoiceFangZiMo = 87,
    kVoiceFang = 88,
    kVoiceSite0 = 89,
    kVoiceSite3 = 90,
    kVoiceSite1 = 91,
    kVoiceSite2 = 92,
    kVoiceOnlyPre = 93,
    kVoiceCancel = 94,
    kVoiceOffInstruction = 95,
    kVoiceOnInstruction = 96,
    kVoiceResetData = 97,
    kVoiceGuoPai = 98,
    kVoiceBieJiaGang = 99,
    kVoiceBiDa = 100,
    kVoiceTingPai = 101,
    kVoiceRePlay = 102,
    kVoiceHou = 103,
    kVoiceMaShang = 104,
    kVoicePengPai = 105,
    kVoiceQue = 106,
    kVoiceFuYuan = 107,
    kVoiceGai = 108,
    kVoiceTiaoZi = 109,
    kVoiceTongZi = 110,
    kVoiceWanZi = 111,
    kVoiceDaHu0 = 112,
    kVoiceDaHu1 = 113,
    kVoiceDaHu2 = 114,
    kVoiceC_Hua = 115,
    kVoicePao = 116,
    kVoiceQian_Ja = 117,
    kVoiceCSBuHua = 118,
    kVoiceHua_1 = 119,
    kVoiceHua_2 = 120,
    kVoiceHua_3 = 121,
    kVoiceHua_4 = 122,
    kVoiceHua_5 = 123,
    kVoiceHua_6 = 124,
    kVoiceHua_7 = 125,
    kVoiceHua_8 = 126,
    kVoiceBaiDa = 127,
    kVoiceGuangBan = 128,   //!< 光板

    kVoiceEast = 129,       //!< 东
    kVoiceNorth = 130,      //!< 北
    kVoiceWest = 131,       //!< 西
    kVoiceSouth = 132,      //!< 南
    kVoiceVersion = 133,    //!< 版本
    kVoiceChannel = 134,    //!< 通道
    kVoiceAddress = 135,    //<! 地址
    kVoiceLearn = 136,      //!< 学习
    kVoiceReader = 137,     //!< 读头
    kVoiceSuccess = 138,    //!< 成功
    kVoiceFail = 139,       //!< 失败
    kVoiceOverTime = 140,   //!< 超时
    kVoice100 = 141,        //!< 百
    kVoiceLanPai = 142,     //!< 蓝牌
    kVoiceLvPai = 143,      //!< 绿牌
};

void Speech_Init(PinName dat, PinName clk, PinName en);
void Speech_Play();
void Speech_Add(unsigned char speech);
void Speech_Add(unsigned char* speechs, uint8_t len);
void Speech_AddNum(unsigned char value);
void Speech_Clear();
void Speech_SetVolume(unsigned char vol);

#endif /* VOICE_H_ */
