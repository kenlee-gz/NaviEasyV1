/******************************************************************************
 * @file     	mahjong.h
 * @brief    
 * @version  	1.0.0
 * @date     	2016��4��28��
 *
 * @note
 * Copyright (C) 2016 YUWEI Corp. All rights reserved.
 ******************************************************************************/

#ifndef MAHJONG_H_
#define MAHJONG_H_

#include <stdint.h>

struct MahjongInfo
{
    uint8_t dsfID;
    uint8_t uids[4];
};

struct MahjongInfoEx
{
    uint8_t dsfID;
    uint8_t uids[8];
};

enum MahjongIDs
{
    kMahjongNone = 0,

    // ��
    kMahjongTiao1 = 1,
    kMahjongTiao2 = 2,
    kMahjongTiao3 = 3,
    kMahjongTiao4 = 4,
    kMahjongTiao5 = 5,
    kMahjongTiao6 = 6,
    kMahjongTiao7 = 7,
    kMahjongTiao8 = 8,
    kMahjongTiao9 = 9,

    // Ͳ
    kMahjongTong1 = 1 + 9,
    kMahjongTong2 = 2 + 9,
    kMahjongTong3 = 3 + 9,
    kMahjongTong4 = 4 + 9,
    kMahjongTong5 = 5 + 9,
    kMahjongTong6 = 6 + 9,
    kMahjongTong7 = 7 + 9,
    kMahjongTong8 = 8 + 9,
    kMahjongTong9 = 9 + 9,

    // ��
    kMahjongWan1 = 1 + 18,
    kMahjongWan2 = 2 + 18,
    kMahjongWan3 = 3 + 18,
    kMahjongWan4 = 4 + 18,
    kMahjongWan5 = 5 + 18,
    kMahjongWan6 = 6 + 18,
    kMahjongWan7 = 7 + 18,
    kMahjongWan8 = 8 + 18,
    kMahjongWan9 = 9 + 18,

    kMahjongEast = 0x1c,    // ��
    kMahjongNorth = 0x1d,   // ��
    kMahjongWest = 0x1e,    // ��
    kMahjongSouth = 0x1f,   // ��
    kMahjongMiddle = 0x20,  // ��
    kMahjongFa = 0x21,      // ��
    kMahjongBai = 0x22,     // ��
    kMahjongSpring = 0x23,  // ��
    kMahjongSummer = 0x24,  // ��
    kMahjongAutumn = 0x25,  // ��
    kMahjongWinter = 0x26,  // ��
    kMahjongMei = 0x27,     // ÷
    kMahjongLan = 0x28,     // ��
    kMahjongZu = 0x29,      // ��
    kMahjongJu = 0x2a,      // ��
    kMahjongBaiDa = 0x2b,   // �ٴ�
    kMahjongGuangBan = 0x2c,   // ���
};

#endif /* MAHJONG_H_ */
