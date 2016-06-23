/******************************************************************************
 * @file     	mahjong.h
 * @brief    
 * @version  	1.0.0
 * @date     	2016年4月28日
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

    // 条
    kMahjongTiao1 = 1,
    kMahjongTiao2 = 2,
    kMahjongTiao3 = 3,
    kMahjongTiao4 = 4,
    kMahjongTiao5 = 5,
    kMahjongTiao6 = 6,
    kMahjongTiao7 = 7,
    kMahjongTiao8 = 8,
    kMahjongTiao9 = 9,

    // 筒
    kMahjongTong1 = 1 + 9,
    kMahjongTong2 = 2 + 9,
    kMahjongTong3 = 3 + 9,
    kMahjongTong4 = 4 + 9,
    kMahjongTong5 = 5 + 9,
    kMahjongTong6 = 6 + 9,
    kMahjongTong7 = 7 + 9,
    kMahjongTong8 = 8 + 9,
    kMahjongTong9 = 9 + 9,

    // 万
    kMahjongWan1 = 1 + 18,
    kMahjongWan2 = 2 + 18,
    kMahjongWan3 = 3 + 18,
    kMahjongWan4 = 4 + 18,
    kMahjongWan5 = 5 + 18,
    kMahjongWan6 = 6 + 18,
    kMahjongWan7 = 7 + 18,
    kMahjongWan8 = 8 + 18,
    kMahjongWan9 = 9 + 18,

    kMahjongEast = 0x1c,    // 东
    kMahjongNorth = 0x1d,   // 北
    kMahjongWest = 0x1e,    // 西
    kMahjongSouth = 0x1f,   // 南
    kMahjongMiddle = 0x20,  // 中
    kMahjongFa = 0x21,      // 发
    kMahjongBai = 0x22,     // 白
    kMahjongSpring = 0x23,  // 春
    kMahjongSummer = 0x24,  // 夏
    kMahjongAutumn = 0x25,  // 秋
    kMahjongWinter = 0x26,  // 冬
    kMahjongMei = 0x27,     // 梅
    kMahjongLan = 0x28,     // 兰
    kMahjongZu = 0x29,      // 竹
    kMahjongJu = 0x2a,      // 菊
    kMahjongBaiDa = 0x2b,   // 百搭
    kMahjongGuangBan = 0x2c,   // 光板
};

#endif /* MAHJONG_H_ */
