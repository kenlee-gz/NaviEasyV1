/******************************************************************************
 * @file     main.h
 * @brief    
 * @version  1.0.0
 * @date     2015年7月20日
 *
 * @note
 * Copyright (C) 2015 YUWEI Corp. All rights reserved.
 ******************************************************************************/
#ifndef MAIN_H_
#define MAIN_H_

#define USE_WDT 0

//#define 	BOARD_VERSION		1
#define 	BOARD_VERSION		2

//#define     CC1101VER   1
#define     CC1101VER   2

#define		KEY_FUNCTION	4

enum SystemStatus
{
    kSysPowerOn, kSysWakeuped, kSysWaitConnectPhone, kSysIsLink, kSysDisLink, kSysNormal, kSysClosing, kSysClosed, kSysWifiReceived, kSysWifiLedOff, kSysLearnMode,
};

#define kGroup 4
const int kVersion = 10;

typedef unsigned char CardId;
typedef unsigned char Card;

struct Pair
{
    uint8_t key;
    uint8_t value;
};

/**
 * 按键例子: 08 09 74 09 2B 05 02 EF BF
 */
struct RomterKeyMessage
{
    uint8_t rx_length;
    uint8_t reader;
    uint8_t remoter_id[3];
    uint8_t total_key;
    uint8_t key;
    uint8_t fix_ef;
    uint8_t crc;
};

/**
 * 例子: 04 02 00 F1 F7
 */
struct RfAck0xF1
{
    uint8_t rx_length;
    uint8_t reader;
    uint8_t dat0;
    uint8_t dat1;
    uint8_t crc;
};

#if CC1101VER == 1
struct RF_ReadPaiMessage
{
    uint8_t len;
    uint8_t reader;
    uint8_t pos;
    uint8_t npos;
    uint8_t crc;
};

struct RF_ResetPaiMessage
{
    uint8_t len;
    uint8_t reader;
    uint8_t cmd;
    uint8_t ncmd;
    uint8_t crc;
};

/**
 * 例子: 06 02 05 05 1C 40 58
 */
struct RfInsideCardMessage
{
    uint8_t length;
    uint8_t reader;
    uint8_t pai_length;
    uint8_t position;
    uint8_t dsfid;
    uint8_t uids[4];
    uint8_t rssi;
    uint8_t crc;
};

struct RfMessage
{
    uint8_t length;
    uint8_t reader;
    uint8_t command;
    uint8_t extras[];
};
#else
enum RFCommands
{
    kCmdReadStatus =0,
    kCmdLearnReader1 = 1,
    kCmdLearnReader2 = 2,
    kCmdLearnReader3 = 3,
    kCmdLearnReader4 = 4,
    kCmdControllerPowerOn = 0x0f,
    kCmdReadPai = 0x10,
    kCmdReaderReset = 0x11,
    kCmdReaderOn = 0x12,
    kCmdReaderOff = 0x13,

    kCmdReadStatusAck =0x80 | kCmdReadStatus,
    kCmdLearnReader1Ack = 0x80 | kCmdLearnReader1,
    kCmdLearnReader2Ack = 0x80 | kCmdLearnReader2,
    kCmdLearnReader3Ack = 0x80 | kCmdLearnReader3,
    kCmdLearnReader4Ack = 0x80 | kCmdLearnReader4,
    kCmdControllerPowerOnAck = 0x80 | kCmdControllerPowerOn,
    kCmdReadPaiAck = 0x80 |kCmdReadPai,
    kCmdReaderResetAck = 0x80 | kCmdReaderReset,
    kCmdReaderOnAck = 0x80 | kCmdReaderOn,
    kCmdReaderOffAck = 0x80 | kCmdReaderOff,
};

struct RF_TxMessage
{
    uint8_t length;
    uint8_t reader;
    uint8_t command;
    uint8_t transID;
    union
    {
        uint8_t position;
        uint8_t channel;
        uint8_t mode;
    };
    uint8_t rssi;
    uint8_t crc;
};

struct RF_TxNewRoundMessage
{
    uint8_t length;
    uint8_t reader;
    uint8_t command;
    uint8_t transID;
    uint8_t pai_num;    //!<总牌数
    uint8_t round;      //!<局数,上电后第一局为0
    uint8_t dummy;      //!<预留
    uint8_t crc;
};

struct RF_TxLearnMessage
{
    uint8_t length;
    uint8_t reader;
    uint8_t command;
    uint8_t transID;
    uint8_t channel;
    uint8_t mode;
    uint8_t crc;
};


/**
 * 例子:
 */
struct RF_RxInsideCardMessage
{
    uint8_t length;
    uint8_t reader;
    uint8_t command;
    uint8_t transID;
    uint8_t pai_length;
    uint8_t position;
    uint8_t dsfid;
    uint8_t uids[4];
    uint8_t rssi;
    uint8_t crc;
};

struct RF_Message
{
    uint8_t length;
    uint8_t reader;
    uint8_t command;
    uint8_t transID;
    uint8_t extras[];
};
#endif

/**
 * 例如:
 *       04 05 81 00 80
 *       05 05 81 01 13 93
 *       06 05 81 02 4F 31 FE
 */
struct RfTableCardMessage
{
    uint8_t length;
    uint8_t reader;
    uint8_t command;
    uint8_t card_num;
    uint8_t cards[];
};

struct CardInfo
{
    uint8_t card_num;
    CardId cards[];
};

enum CardPattern
{
    kBadCard = 0,
    kTiao1 = 1,
    kTiao2 = 2,
    kTiao3 = 3,
    kTiao4 = 4,
    kTiao5 = 5,
    kTiao6 = 6,
    kTiao7 = 7,
    kTiao8 = 8,
    kTiao9 = 9,

    kTong1 = 11,
    kTong2 = 12,
    kTong3 = 13,
    kTong4 = 14,
    kTong5 = 15,
    kTong6 = 16,
    kTong7 = 17,
    kTong8 = 18,
    kTong9 = 19,

    kWan1 = 21,
    kWan2 = 22,
    kWan3 = 23,
    kWan4 = 24,
    kWan5 = 25,
    kWan6 = 26,
    kWan7 = 27,
    kWan8 = 28,
    kWan9 = 29,

    kOverTiaoTongWan = 30,
};

enum WorkMode
{
    kRequestReader = 0,
    kWaitReaderAck = 1,
    kBeginStudy = 2,
    kStudying = 3,
    kWaitTxActive,
    kWaitTxFinish,
};

enum ReaderCommand
{
    kReaderReadPai = 0x01,
    kReaderClose = 0x02,
    kReaderReadPaiAck = kReaderReadPai | 0x80,
    kReaderCloseAck = kReaderClose | 0x80,
    kReaderPaiClear = 0xf1,
};

/**
 * 抓牌方式
 */
enum DrawTileMode
{
    kDrawJump = 0,          // 跳拿
    kDrawHorizontal = 1,    // 平拿
    kDrawVertical = 2,      // 竖拿
};

enum Reader
{
    kInReaderEast = 1,
    kInReaderNorth = 2,
    kInReaderWest = 3,
    kInReaderSouth = 4,

    kOutReaderEast = 5,
    kOutReaderNorth = 6,
    kOutReaderWest = 7,
    kOutReaderSouth = 8,
    kReaderCount,
};

#define right    10         //右方
#define answer   20         //对方
#define left     30         //左方


#define PARTY_CNT       9          //桌内方位组数
#define PAINUM          170        //四方牌总数
#define MAX_WALL_CARD   40        //
#define RF_CARD_CLEAR   0xf1

#define CONFIG_BYTES    13

#define kPaiIdCount     40
#define MAX_CALC_STEP   50

enum BankerPosition
{
    kBankerIsUnknow = 0,
    kBankerIsCardShark = 1,
    kBankerIsRight = 2,
    kBankerIsOpposite = 3,
    kBankerIsLeft = 4,
    kBankerCalcError = 5,
};

enum ReaderStatus
{
    kIntReaderRxFail = 0, kIntReaderRxOk = 1,
};

enum PaiReportMode
{
    kJumpReport = 0, kReportNext = 1,
};


#endif /* MAIN_H_ */
