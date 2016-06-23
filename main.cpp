/******************************************************************************
 * @file        main.cpp
 * @brief
 * @version     1.0.0
 * @date        2016年4月28日
 *
 * @note
 * 修改:	 增加颜色
 * Copyright (C) 2016 YUWEI Corp. All rights reserved.
 ******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "main.h"
#include "debug.h"
#include "mbed.h"
#include "CC1101.h"
#include "WT588D.h"
#include "CommBLE.h"
#include "pinmap.h"
#include "CircularBufferEx.hpp"
#include "FlashApp.h"
#include "BufferedSerial.h"
#include "HighPowerReader.h"
#include "Phone.h"
#include "SimpleKey.h"
#include "Voice.h"

//#define KEY_EN  1
const uint8_t kDeviceID = 0x01;
const uint8_t kFirmwareVer = 0x02;
const uint8_t kHardwareVer = 0x01;

#define  BATTERY_EN  0

#define USE_WIFI    0
#define USE_BLE     1

struct EepromContent
{
	uint32_t version;
	uint8_t ssid;
	uint8_t rf_channel;
	uint8_t volume;
	uint8_t res;
	uint32_t uids;
	uint32_t res2;

	uint8_t east_cnt;
	uint8_t north_cnt;
	uint8_t west_cnt;
	uint8_t south_cnt;
};

EepromContent contents;

uint8_t pai_color = 0;
struct ReaderMessage
{
	uint8_t PAI_num;                // 总牌数
	MahjongInfo PAI_Cache_dat[MAX_WALL_CARD];
	uint8_t bearing_bad;            // 坏牌数量
	uint8_t PAI_position;           // 当前读牌位置,从0开始
	uint8_t PAI_err;                // 读头错误回应计数
	uint8_t rxrssi;                 // 读头rssi
	bool is_handled :1;
	bool working :1;
};

enum ReaderResult
{
	kReaderNoMessage, kReaderMessageOK, kReaderMessageOverTime,
};

struct ReaderMessage reader_monitor[kGroup];

struct PhonePaiInfo
{
	bool is_send;                       //!< 是否已经发送
	uint8_t count;                      //!< 待发送牌数, 基本与抓牌数是一样的
	uint8_t send_count;                 //!< 发送给手机次数
	uint8_t transID;                    //!< 最后一次发送的流水号
	MahjongInfo pais[MAX_WALL_CARD];    //!< 待发送给手机的牌
};

struct PhonePaiInfo phone_pais[kGroup];

SystemStatus sys_status;

// WIFI模块
BufferedSerial* phone_uart = NULL;
BufferedSerial* debug_uart = NULL;
BufferedSerial* reader_uart = NULL;

// 实测WIFI模块一条指令返回最多达56个字节，所以缓冲不能太小！
uint8_t wifi_buf[256];
CircularBufferEx circular_wifi(wifi_buf, sizeof(wifi_buf));
BLE* ble = NULL;
DigitalOut *wifi_rst;
DigitalOut* reader_power;
DigitalOut* reader_reset;

// 电源控制和电池电压检测等
#if BATTERY_EN == 1
AnalogIn battery_volt_in(PA_0);
#endif

// LED状态指示灯
DigitalOut* led_power;
DigitalOut* led_X;
Ticker ticker;
DigitalOut* rf_power;

SimpleKey* key = NULL;
SimpleKey* key_info = NULL;

// 2.4G芯片
//SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);
DigitalOut* rfRst;
uint8_t rf_rxbuf[256];
uint8_t rf_txbuf[256];
CircularBufferEx circular_rxbuf(rf_rxbuf, sizeof(rf_rxbuf));
CircularBufferEx circular_txbuf(rf_txbuf, sizeof(rf_txbuf));
CC1101* cc1101;

Phone* phone;
CommBLE* phoneBle;

bool receivedTong = false;
Timer g_timer;
Timer power_tmr;

uint8_t mcu_uid[12] __attribute__((at(0x1FFFF7AC)));
uint16_t kFlashSizeInKBytes __attribute__((at(0x1FFFF7CC)));
// 单片机识别码，x8: b[11..0]=0x440
uint32_t kMcuIdCode __attribute__((at(0x40015800)));
//uint32_t user_uid __attribute__((at(0x8001000)));

#define kVersion    1

int currentReader = 1;
WorkMode RP_mode = kRequestReader;
Timer reader_tmr;

uint8_t rf_buffer[64];
//static uint8_t PAI_Cache_dat[kGroup][MAX_WALL_CARD];    //四方牌总数据组(1:东 2:南 3:西 4:北)
//static uint8_t bearing_bad[kGroup+1];

//uint8_t PAI_position[kGroup+1];

//uint8_t PAI_num[kGroup+1];
uint8_t RP_num = 0;                //读到牌的总数
uint8_t TotalCardNum = 136;

bool IsXPFinished = true;
ReaderStatus RF_rxst = kIntReaderRxFail;

Timer RP_ovtimer;

Timer lastKeyPressTmr;
int keyPressedCount = 0;

//static const int kDefaultMahjongCount[4] = {26, 28, 26, 28};
static const int kDefaultMahjongCount[4] =
{ 34, 34, 34, 34 };

const uint32_t Fml_Constant = 0x34770299; // 输入到公式的常数
const uint8_t* C = (const uint8_t*) &Fml_Constant; //把常量转换为数组
const int eeprom_start = 0x0800fc00;
const int encrypt_uid_addr = 0x0800fc00;
const int kPaiCountAddr = 0x0800fc10;
const int kStudyChannel = 227;
//const int kStudyChannel = 191;

Timer phone_send_timer;
bool phone_send_wait_ack = false;
//uint8_t phone_pai_transid = 0;
uint8_t phone_pai_pos = 0;

static const bool kSendAllPaiAtOneTime = true;  // 同时组合多张牌一起发给手机
const int kMaxAdminPaiCount = 3;
MahjongInfo unsend_pai[kMaxAdminPaiCount];
struct PhoneLivePais
{
	bool is_send;                       //!< 是否已经发送
	bool is_wait;
	uint8_t count;                      //!< 待发送牌数
	uint8_t send_count;                 //!< 发送次数
	uint8_t transID;                    //!< 最后一次发送的流水号
	Timer timer;
	MahjongInfo pais[kMaxAdminPaiCount];                //!< 待发送给手机的牌
};
PhoneLivePais phone_live_pai;
int unsend_count = 0;
Timer unsend_timer;
uint8_t rf_tx_transID = 0;
uint8_t g_round = 0xff;         //!<下一局为0
static Timer pai_led_timer;     //!<指示收到牌LED指示灯显示定时器

extern const uint8_t tab_encrypt[] =
{ 0x64, 0x0a, 0x24, 0x51, 0xc4, 0x51, 0xee, 0x31, 0x26, 0x3c, 0x35, 0x3c, 0x5d,
		0x7d, 0x0e, 0x7b, 0x13, 0xa2, 0x71, 0xb6, 0x70, 0xaf, 0x85, 0x34, 0x10,
		0xfb, 0x30, 0xb4, 0x24, 0x4d, 0xdf, 0xab, 0xe6, 0x36, 0x93, 0xc0, 0x43,
		0xb1, 0x2c, 0x86, 0xc4, 0xfa, 0xfa, 0x79, 0xbe, 0xac, 0x82, 0x3b, 0xba,
		0x1e, 0x8d, 0x0f, 0x27, 0xff, 0x77, 0x96, 0x3f, 0x30, 0xb6, 0xca, 0x36,
		0xe1, 0xa9, 0x38, 0xaa, 0xf0, 0xa3, 0x82, 0x47, 0x7e, 0x37, 0x14, 0xba,
		0xd3, 0xc6, 0x26, 0xd9, 0x74, 0x48, 0xf3, 0x10, 0x83, 0x57, 0x38, 0x0f,
		0x57, 0x81, 0xef, 0xb2, 0x5c, 0xcf, 0x4d, 0x30, 0x2c, 0x91, 0xf9, 0x88,
		0xee, 0x6d, 0x8f, 0x28, 0xef, 0xa7, 0x55, 0xe0, 0x80, 0xb2, 0x3c, 0x07,
		0x0f, 0xf8, 0x1a, 0xec, 0x89, 0xe7, 0x27, 0x80, 0xef, 0x3c, 0xb5, 0x41,
		0x37, 0x94, 0x34, 0x69, 0x65, 0x30, 0x66, 0x58, 0xea, 0x08, 0xde, 0x3f,
		0x3d, 0x13, 0xbf, 0x0e, 0xb8, 0xd6, 0xb3, 0xa1, 0xb5, 0x2b, 0x2a, 0x26,
		0xe9, 0x54, 0xd5, 0xd1, 0x00, 0x40, 0x62, 0xc4, 0x7a, 0x1c, 0x78, 0x39,
		0xc5, 0x1d, 0xf6, 0xf2, 0x9c, 0x8d, 0x69, 0xe2, 0x9f, 0x14, 0xcb, 0x1d,
		0x33, 0x49, 0x83, 0xfd, 0x9d, 0x79, 0x99, 0x96, 0x5a, 0xb6, 0x39, 0x5c,
		0xc1, 0x24, 0x6c, 0x13, 0xdc, 0x80, 0x12, 0xf8, 0x83, 0xf1, 0x22, 0x2e,
		0xba, 0x13, 0x26, 0x6b, 0x4d, 0x41, 0xf1, 0xe3, 0xaa, 0x23, 0xa4, 0x74,
		0xff, 0x79, 0xe0, 0x14, 0x94, 0x27, 0x4d, 0x78, 0x6a, 0x80, 0x4b, 0x07,
		0x15, 0xd7, 0xf8, 0xfe, 0xd7, 0x44, 0x62, 0xe4, 0xfe, 0xb3, 0x0e, 0x30,
		0x7f, 0x33, 0xa9, 0x39, 0xd4, 0x7e, 0x0d, 0x5f, 0x13, 0xc4, 0x77, 0x78,
		0x4f, 0xbc, 0x07, 0x7d, 0x34, 0xed, 0x79, 0x76, 0xde, 0x3a, 0x22, 0xa3,
		0xf9, 0xad, 0x2e, 0xec, 0x1f, 0x85, 0x18, 0x8b, 0x6e, 0x82, 0x6b, 0xf8,
		0x69, 0x6f, 0xb7, 0x15, 0x11, 0xf1, 0xd6, 0x9a, 0x44, 0x90, 0x60, 0xc4,
		0x56, 0x03, 0x6d, 0x39, 0xed, 0xc0, 0x8a, 0x3f, 0x22, 0xc4, 0xfe, 0x1e,
		0xd6, 0xa0, 0x3e, 0xd2, 0x51, 0xc5, 0xae, 0xf6, 0x22, 0x11, 0x9a, 0xed,
		0x32, 0x99, 0x75, 0x53, 0x2a, 0xb9, 0x50, 0xa3, 0x08, 0x5a, 0x9e, 0x29,
		0xfd, 0x83, 0x27, 0x2a, 0x89, 0x23, 0x49, 0x52, 0xda, 0x1d, 0x76, 0x5f,
		0x61, 0x94, 0xeb, 0x0d, 0xb6, 0x79, 0xae, 0x41, 0xae, 0x54, 0xcb, 0x79,
		0xba, 0x4f, 0xcf, 0x73, 0x83, 0x88, 0x85, 0x1c, 0xc3, 0x98, 0xf0, 0xbd,
		0x67, 0x5f, 0x89, 0x61, 0xe3, 0x13, 0xba, 0x89, 0xd5, 0x89, 0x9a, 0x13,
		0xdd, 0xc2, 0xeb, 0x68, 0xbb, 0xb9, 0x51, 0xe5, 0xab, 0x6b, 0xd5, 0x8c,
		0xfe, 0x28, 0x9a, 0xec, 0xf9, 0x19, 0xde, 0x4e, 0xf2, 0x13, 0x34, 0x21,
		0xa9, 0x9b, 0x00, 0xa8, 0xe5, 0x39, 0x33, 0xe2, 0x4f, 0x02, 0x48, 0xba,
		0x93, 0x60, 0x7f, 0x70, 0xb5, 0x25, 0x59, 0x47, 0xb1, 0xd0, 0x55, 0x70,
		0x56, 0x1e, 0xe9, 0x36, 0x64, 0xd4, 0xc3, 0x6c, 0xe5, 0xcc, 0x40, 0x14,
		0xc9, 0x3f, 0x2e, 0x51, 0xc5, 0x51, 0xbe, 0x8f, 0x6d, 0xe2, 0xcd, 0xf0,
		0x90, 0x94, 0x54, 0xfa, 0xd4, 0x18, 0x2d, 0x80, 0x93, 0xbe, 0x08, 0xce,
		0xf5, 0x3d, 0xad, 0x18, 0x4e, 0xbf, 0x74, 0x25, 0xba, 0x30, 0x02, 0x37,
		0xf6, 0xc8, 0x46, 0x3f, 0x82, 0xd7, 0xb3, 0x3f, 0xf5, 0xd1, 0xbf, 0xff,
		0xa0, 0x99, 0x9b, 0xf3, 0x67, 0x12, 0x35, 0x6c, 0xdb, 0xe4, 0x6c, 0xff,
		0x9c, 0x8c, 0x93, 0x33, 0xef, 0xa7, 0x28, 0x6e, 0x9b, 0x7f, 0xd5, 0x22,
		0x09, 0xd6, 0xa8, 0x33, 0x9b, 0xf3, 0x8f, 0xac, 0x4c, 0xeb, 0x18, 0xa3,
		0x8a, 0x32, 0x66, 0x6a, 0x50, 0x61, 0x76, };

/**
 Modulation format = GFSK
 Sync word qualifier mode = 30/32 sync word bits detected
 TX power = 0
 Packet length mode = Variable packet length mode. Packet length configured by the first byte after sync word
 Data rate = 49.9878
 Device address = 0
 Whitening = false
 Packet length = 255
 Deviation = 25.390625
 Carrier frequency = 867.999939
 Data format = Normal mode
 Preamble count = 4
 Modulated = true
 Channel spacing = 199.951172
 Channel number = 0
 CRC autoflush = false
 PA ramping = false
 RX filter BW = 101.562500
 Manchester enable = false
 Address config = No address check
 Base frequency = 867.999939
 CRC enable = true
 **/
#if 0
const uint8_t cc1101_868MHz_configs[] =
{
//    0x29,  // IOCFG2
//    0x2E,  // IOCFG1
//    0x06,  // IOCFG0
	kGDO_TxRxSyncWord,//IOCFG2
	kGDO_HighImpedance,//IOCFG1
	kGDO_PacketRxWithCrcOK,//IOCFG0
	0x0c,// FIFOTHR
	0xD3,// SYNC1
	0x91,// SYNC0
	0x20,// PKTLEN
	0x04,// PKTCTRL1
	0x05,// PKTCTRL0
	0x00,// ADDR
	0x00,// CHANNR
	0x06,// FSCTRL1
	0x00,// FSCTRL0
	// 868
//    0x21,  // FREQ2
//    0x62,  // FREQ1
//    0x76,  // FREQ0
	// 925
//    0x23,  // FREQ2
//    0x93,  // FREQ1
//    0xb1,  // FREQ0
	// 920
	0x23,// FREQ2
	0x62,// FREQ1
	0x76,// FREQ0
	0xCA,// MDMCFG4
	0xF8,// MDMCFG3
	0x13,// MDMCFG2
	0x22,// MDMCFG1
	0xF8,// MDMCFG0
	0x40,// DEVIATN
	0x07,// MCSM2
	0x30,// MCSM1
	0x18,// MCSM0
	0x16,// FOCCFG
	0x6C,// BSCFG
	0x03,// AGCCTRL2
	0x40,// AGCCTRL1
	0x91,// AGCCTRL0
	0x87,// WOREVT1
	0x6B,// WOREVT0
	0xFB,// WORCTRL
	0x56,// FREND1
	0x10,// FREND0
	0xE9,// FSCAL3
	0x2A,// FSCAL2
	0x00,// FSCAL1
	0x1F,// FSCAL0
	0x41,// RCCTRL1
	0x00,// RCCTRL0
	0x59,// FSTEST
	0x7F,// PTEST
	0x3F,// AGCTEST
	0x81,// TEST2
	0x35,// TEST1
	0x09,// TEST0
};
#endif

#define DEBOUNCE_TIME   20
#define LONG_PRESSED   1500

//#define BAUDRATE	4800
//#define BAUDRATE	9600
#define BAUDRATE	19200
//#define BAUDRATE	50000

const RF_SETTINGS rfNavigator =
{ kGDO_TxRxSyncWord,      //IOCFG2
		kGDO_HighImpedance,     //IOCFG1
		kGDO_PacketRxWithCrcOK, //IOCFG0

		0x0c, //FIFOTHR//TX53；RX12

		0xd3, //SYNC1//前导高位
		0x91, //SYNC0//前导低位

		0x20, //PKTLEN//数据个数
		0x04, //PKTCTRL1//RSSI+LQI
		0x05, //PKTCTRL0
		0x00, //ADDR      //地址
		191, //CHANNR  //频道    //880MHZ+191*0.2

		0x08, //FSCTRL1  //
		0x00, //FSCTRL0
		0x21, //FREQ2       //频率合成器880MHZ
		0xD8, //FREQ1
		0x9D, //FREQ0

#if BAUDRATE == 4800
    0x97,//MDMCFG4//滤波带宽   BW160K
    0x83,//MDMCFG3//数据速率   4.8k
#elif BAUDRATE == 9600
    0x98,//MDMCFG4//滤波带宽   BW160K
    0x83,//MDMCFG3//数据速率   9.6k
#elif BAUDRATE == 19200
    0x99,//MDMCFG4//滤波带宽   BW160K
    0x83,//MDMCFG3//数据速率   19.2k
#elif BAUDRATE == 50000
    0x9A,//MDMCFG4//滤波带宽   BW160K
    0xF8,//MDMCFG3//数据速率   50k
#endif

		0x13, //MDMCFG2(x) 曼切斯特编码-速度减半
//        0x22, //MDMCFG1(x)//前导最小发送数
		0x02, //MDMCFG1(x)//前导最小发送数

		0xf8, //MDMCFG0     //信道空间
		0x47, //DEVIATN     //背离设置

		0x07, //MCSM2
		0x00, //MCSM1       //发送结束后转到接收状态
		0x18, //MCSM0       //校准
		0x1d, //F0CCFG      //频率便宜补偿配置
		0x1c, //BSCFG       //位同步配置

		0xc7, //AGCCTRL2    //自动增益调节
		0x00, //AGCCTRL1
		0xb2, //AGCCTRL0

		0x87, //WOREVT1
		0x6B, //WOREVT0
		0xF8, //WORCTRL

		0x56, //FREND1   //前端RX配置
		0x10, //FREND0   //前端TX配置

		0xea, //FSCAL3  //频率合成器校准
		0x2a, //FSCAL2  //可直接写入历史值完成效准
		0x00, //FSCAL1
		0x11, //FSCAL0

		0x0d, //RCCTRL1
		0x0d, //RCCTRL0

		0x59, //FSTEST
		0x3f, //PTEST
		0xe7, //AGCTEST
		0x81, //TEST2
		0x35, //TEST1
		0x0b, //TEST0
		};

const char* strPai[] =
{ "XX  ", "一条", "二条", "三条", "四条", "五条", "六条", "七条", "八条", "九条", "XX  ", "一筒",
		"二筒", "三筒", "四筒", "五筒", "六筒", "七筒", "八筒", "九筒", "XX  ", "一万", "二万",
		"三万", "四万", "五万", "六万", "七万", "八万", "九万", "XX  ", "东风", "南风", "西风",
		"北风", "中  ", "发财", "白板",   // 37

		"春  "
				"夏  ", "秋  ", "冬  ", "梅  ", "兰  ", "菊  ", "竹  ",

		"补花",   // 46
		};

uint8_t ConvertMahjongId2PhoneId(uint8_t mahjongid)
{
	if ((mahjongid & 0x3f) == kMahjongNorth)
	{
		return (mahjongid & 0xc0) | kMahjongSouth;
	}
	else if ((mahjongid & 0x3f) == kMahjongSouth)
	{
		return (mahjongid & 0xc0) | kMahjongNorth;
	}
	return mahjongid;
}

uint8_t ConvertIndexToRFChannel(uint8_t index)
{
	assert(index >= 1 && index <= 8);
	return 187 + index * 4;
}

/**
 * 获取异或和
 * @param buf
 * @param len
 * @return
 */
uint8_t CalcCrc(const void* buf, uint8_t len)
{
	uint8_t result = 0;
	const uint8_t* p = (const uint8_t*) buf;

	while (len--)
	{
		result ^= *p++;
	}
	return result;
}

void PrintfPai(uint8_t* pais)
{
	for (int i = 0; i < kPaiIdCount; i++)
	{
		for (int j = 0; j < pais[i]; j++)
		{
			// pc.printf(" %s", strPai[i]);
		}
	}
	// pc.printf("\r\n");
}

void GeneratePai()
{
#if 0
	static int s_count = 4;
	ReaderMessage* reader;
	for (int i = 0; i < kGroup; i++)
	{
		reader = &reader_monitor[i];
		reader->PAI_num = s_count;
		reader->PAI_position = s_count;
		reader->is_handled = false;
		for (int j = 0; j < 34; j++)
		{
			reader->PAI_Cache_dat[j].dsfID = (34 * i + j) / 4 + 1;
			for (int k = 0; k < 4; k++)
			{
				reader->PAI_Cache_dat[j].uids[0] = k;
				reader->PAI_Cache_dat[j].uids[1] = j;
				reader->PAI_Cache_dat[j].uids[2] = i;
				reader->PAI_Cache_dat[j].uids[3] = reader->PAI_position;
			}
		}
	}
	s_count += 2;
#else
	PhonePaiInfo* reader;
	for (int i = 0; i < kGroup; i++)
	{
		reader = &phone_pais[i];
		reader->count = kDefaultMahjongCount[i];
		reader->is_send = false;
		reader->send_count = 0;
		for (int j = 0; j < 34; j++)
		{
			reader->pais[j].dsfID = (34 * i + j) / 4 + 1;
			for (int k = 0; k < 4; k++)
			{
				reader->pais[j].uids[0] = k;
				reader->pais[j].uids[1] = j;
				reader->pais[j].uids[2] = i;
				reader->pais[j].uids[3] = reader->count;
			}
		}
	}
#endif
}

#if 0
void Test()
{
	uint32_t loop = 0;

	uint32_t tick = us_ticker_read();
	while (0)
	{
		uint8_t rfbuf_tx[7];
		rfbuf_tx[0] = 0x6;                        //总包长
		rfbuf_tx[1] = 0xaa;//读头方位

		rfbuf_tx[2] = loop++;//读牌开始位置
		rfbuf_tx[3] = ~rfbuf_tx[2];//位置反码

		rfbuf_tx[4] = 0;
		rfbuf_tx[5] = 0;
		rfbuf_tx[6] = CalcCrc(rfbuf_tx, 6);
		cc1101->SendPacket(rfbuf_tx, 7);
		cc1101->RXMode();
		wait_ms(200);

	}

	Timer tmr;
	uint8_t buf[50];
	uint8_t rxbuf[64];

	tmr.start();

#if 0
	// 读取cc1101内部寄存器的值并打印出来
	cc1101->ReadBurstReg(0, buf, 47);
	for (int i = 0; i < 47; i++)
	{
		// pc.printf("%02X: %02hhX\r\n", i, buf[i]);
	}
#endif

	for (int i = 0; i < 10000; i++)
	{
		// pc.printf("Send Times: %d\r\n", i);
		tmr.reset();
		led = 0;

		while (tmr.read_ms() < 50)
		{
			if (cc1101->GetGDO0())
			{    // rx finished and CRC OK read the new packet
				rxbuf[0] = sizeof(rxbuf) - 1;
				if (cc1101->ReceivePacket(rxbuf) == 1)
				{ // read the rx packet
					led = 1;
					// pc.printf("Received:");
					for (i = 0; i <= rxbuf[0]; i++)
					{
						// pc.printf(" %02hhX", rxbuf[i]);
					}
					// pc.printf("\r\n");
				}
			}
		}
	}

	// pc.printf("End Test\r\n");

}
#endif

void ClearReaderData(ReaderMessage* reader)
{
	reader->PAI_position = 0;
	reader->is_handled = false;
	reader->PAI_err = 0;
	reader->bearing_bad = 0;
	memset(reader->PAI_Cache_dat, 0xff, MAX_WALL_CARD * sizeof(MahjongInfo));
}

#if CC1101VER == 1
//! @param reader 读头
//! @note 0x04 0x01 0x01 0xfe 0xfa
void Reader_ReadInside(uint8_t reader)
{
	assert(reader >= 1 && reader <= 4);

	if (reader_monitor[reader - 1].PAI_position < reader_monitor[reader - 1].PAI_num)
	{
		if (!reader_monitor[reader - 1].working)
		{
			RF_ResetPaiMessage rf_tx;
			rf_tx.len = sizeof(RF_ResetPaiMessage) - 1; //总包长
			rf_tx.cmd = kReaderPaiClear;
			rf_tx.ncmd = ~rf_tx.cmd;
			rf_tx.reader = reader;//读头方位

			RP_mode = kWaitTxActive;
			cc1101->WriteReg(CCxxx0_IOCFG2, 0x06);

			RF_rxst = kIntReaderRxFail;

			rf_tx.crc = CalcCrc(&rf_tx, sizeof(RF_ResetPaiMessage) - 1);
			cc1101->SendPacketAsync(&rf_tx, sizeof(RF_ResetPaiMessage));
			DBGARR(rf_tx, sizeof(RF_ResetPaiMessage));
		}
		else
		{
			RF_ReadPaiMessage rfbuf_tx;
			rfbuf_tx.len = sizeof(RF_ReadPaiMessage) - 1; //总包长
			rfbuf_tx.reader = reader;//读头方位

			RP_mode = kWaitTxActive;
			cc1101->WriteReg(CCxxx0_IOCFG2, 0x06);

			RF_rxst = kIntReaderRxFail;
#if CC1101VER == 1
			rfbuf_tx.pos = reader_monitor[reader - 1].PAI_position + 1; //读牌开始位置
#else
			rfbuf_tx.pos = reader_monitor[reader - 1].PAI_position;  //读牌开始位置
#endif
			rfbuf_tx.npos = ~rfbuf_tx.pos;
			rfbuf_tx.crc = CalcCrc(&rfbuf_tx, sizeof(RF_ReadPaiMessage) - 1);
			cc1101->SendPacketAsync(&rfbuf_tx, sizeof(RF_ReadPaiMessage));
			DBGARR(rfbuf_tx, sizeof(RF_ReadPaiMessage));
		}

		reader_tmr.reset();
	}
}
#else
//! @param reader 读头
//! @note
//! 05 01 10 2D 00 39
//! 05 02 10 2E 00 39
//! 05 03 10 2F 00 39
//! 05 04 10 30 00 21
void Reader_ReadInside(uint8_t reader)
{
	assert(reader >= 1 && reader <= 4);

	if (reader_monitor[reader - 1].PAI_position
			< reader_monitor[reader - 1].PAI_num)
	{
		if (!reader_monitor[reader - 1].working)
		{
			RF_TxNewRoundMessage rf_tx;
			rf_tx.length = sizeof(RF_TxNewRoundMessage) - 1; //总包长
			rf_tx.reader = reader; //读头方位
			rf_tx.transID = ++rf_tx_transID;
			rf_tx.command = kCmdReaderReset;
			rf_tx.pai_num = reader_monitor[reader - 1].PAI_num;
			rf_tx.round = g_round;
			rf_tx.dummy = 0;

			RP_mode = kWaitTxActive;
			cc1101->WriteReg(CCxxx0_IOCFG2, 0x06);

			RF_rxst = kIntReaderRxFail;

			rf_tx.crc = CalcCrc(&rf_tx, sizeof(RF_TxNewRoundMessage) - 1);
			cc1101->SendPacketAsync(&rf_tx, sizeof(RF_TxNewRoundMessage));
			DBGARR(rf_tx, sizeof(RF_ResetPaiMessage));
		}
		else
		{
			RF_TxMessage rf_tx;
			rf_tx.length = sizeof(RF_TxMessage) - 1; //总包长
			rf_tx.reader = reader; //读头方位
			rf_tx.transID = ++rf_tx_transID;
			rf_tx.command = kCmdReadPai;

			RP_mode = kWaitTxActive;
			cc1101->WriteReg(CCxxx0_IOCFG2, 0x06);

			RF_rxst = kIntReaderRxFail;

			rf_tx.position = reader_monitor[reader - 1].PAI_position + 1; //读牌开始位置
			rf_tx.rssi = reader_monitor[reader - 1].rxrssi;
			rf_tx.crc = CalcCrc(&rf_tx, sizeof(RF_TxMessage) - 1);
			cc1101->SendPacketAsync(&rf_tx, sizeof(RF_TxMessage));
			DBGARR(rf_tx, sizeof(RF_ReadPaiMessage));
		}

		reader_tmr.reset();
	}
}
#endif

void SaveContentsToFlash()
{
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef erase_init;
	uint32_t page_error;
	erase_init.NbPages = 1;
	erase_init.PageAddress = eeprom_start;
	erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
	HAL_StatusTypeDef def = HAL_FLASHEx_Erase(&erase_init, &page_error);

	for (int i = 0; i < sizeof(EepromContent) / 4; i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, eeprom_start + 4 * i,
				*((uint32_t*) &contents + i));
	}
	HAL_FLASH_Lock();
}

void SendRFCommand()
{
	if (IsXPFinished)
	{
		// 避免最后一张牌会计算在下一局上面,因此检查一下超时
		if (RP_ovtimer.read_ms() > 400)
		{
			IsXPFinished = false;
			for (int i = 0; i < kGroup; i++)
			{
				ClearReaderData(&reader_monitor[i]);
			}
			RP_ovtimer.reset();
			++g_round;
		}
	}
	else
	{
		Reader_ReadInside(currentReader);
	}
}

bool ReceivedRFAck(uint8_t* rxbuf, int max_len)
{
	if (cc1101->GetGDO2())      // TODO
	{
		DBGS("Reading...\r\n");
		return cc1101->ReceivePacket(rxbuf, max_len);
	}
	else
	{
		return false;
	}
}

/**
 * 获取坏牌数量
 * @return
 */
int DAT_GetBadTiles()
{
	uint8_t result = 0;
	uint8_t i;

	for (i = 0; i < kGroup; i++)
	{
		if (reader_monitor[i].bearing_bad)
			result += reader_monitor[i].bearing_bad;
	}

	return result;
}

void Navigator_Init()
{
	reader_tmr.start();
	RP_ovtimer.start();
	reader_monitor[0].PAI_num = contents.east_cnt;
	reader_monitor[1].PAI_num = contents.north_cnt;
	reader_monitor[2].PAI_num = contents.west_cnt;
	reader_monitor[3].PAI_num = contents.south_cnt;

	TotalCardNum = 0;
	for (int i = 0; i < kGroup; i++)
	{
		TotalCardNum += reader_monitor[i].PAI_num;
//        reader_monitor[i].working = false;
		reader_monitor[i].working = true;
		reader_monitor[i].rxrssi = 0;
		ClearReaderData(&reader_monitor[i]);

		phone_pais[i].count = 0;
		phone_pais[i].is_send = false;
		phone_pais[i].send_count = 0;
	}

	phone_live_pai.count = 0;
	phone_live_pai.is_send = false;
	phone_live_pai.is_wait = false;
	phone_live_pai.timer.start();
	phone_live_pai.send_count = 0;
}

/**
 * 洗完牌，检查牌没有发送给手机。没有发送就全部把它发送完。
 */
void DAT_Load(void)
{
	ReaderMessage* reader;

	for (int i = 0; i < kGroup; i++)
	{
		reader = &reader_monitor[i];
		if (RP_num > TotalCardNum / 2 && !reader->is_handled)
		{
//            phone->SendGroupData(i + 1, reader->PAI_Cache_dat, reader->PAI_position);
			memcpy(phone_pais[i].pais, reader->PAI_Cache_dat,
					reader->PAI_position * sizeof(MahjongInfo));
			if (reader->PAI_num > reader->PAI_position)
			{
				memset(&phone_pais[i].pais[reader->PAI_position], 0xff,
						sizeof(MahjongInfo)
								* (reader->PAI_num - reader->PAI_position));
			}
			phone_pais[i].is_send = false;
			phone_pais[i].send_count = 0;
			phone_pais[i].count = reader->PAI_num;
			reader->is_handled = true;
		}
		ClearReaderData(reader);
	}
}

/**
 * 结束洗牌
 */
void Reader_FinishXP()
{
	RP_ovtimer.reset();         //停止并清计时

	for (int i = 0; i < kGroup; i++)
	{
		reader_monitor[i].working = false;
	}

	DAT_Load();             //加载数据到数据缓冲区
	RP_num = 0;
	IsXPFinished = true;          //洗牌结束
}

#if CC1101VER == 1
/**
 * 桌内读头接收数据
 * 有数据: 06 04 22 22 8C 42 CC
 * 无数据: 06 04 21 22 FF 41 BF
 * 回kReaderPaiClear查询(仅一次): 04 04 00 F1 F1
 * @param msg
 */
void Reader_HandleInsideMessage(const RfMessage* msg)
{
	assert(currentReader >= 1 && currentReader <= 4);

	struct ReaderMessage* reader = &reader_monitor[msg->reader - 1];
	RF_rxst = kIntReaderRxOk;

	if (CalcCrc(msg, msg->length + 1) != 0)
	return;

	switch (msg->command)
	{
		case kReaderPaiClear:
		ClearReaderData(reader);
		reader->working = true;
		break;

		default:
		const RfInsideCardMessage* in_msg = (const RfInsideCardMessage*) msg;
		if (in_msg->pai_length > reader->PAI_num)
		{
			reader->working = false;
		}
		else if (in_msg->pai_length >= in_msg->position/*(in_msg->dsfid != 0xff)*/
				&& (in_msg->position == reader->PAI_position + 1))
		{      // 有可能对应位置为0xFF???
			if (reader->PAI_position < reader->PAI_num)
			{
				memcpy(&reader->PAI_Cache_dat[reader->PAI_position], &in_msg->dsfid, sizeof(MahjongInfo));
				reader->PAI_Cache_dat[reader->PAI_position].dsfID = ConvertMahjongId2PhoneId(in_msg->dsfid);
#if 0
				TileInfo info =
				{	currentReader, reader->PAI_position, reader->PAI_Cache_dat[reader->PAI_position]};
				phoneBle->SendTilesReaderData(&info);
#endif
				reader->PAI_position++;
				RP_num++;               //读到的总牌数加一
				RP_ovtimer.reset();
			}
		}
		break;
	}
}
#else

inline bool IsColorChanged(uint8_t pai)
{
	return (pai_color ^ pai) & 0x80;
}

void AddOnePaiToCache(struct ReaderMessage* reader,
		const RF_RxInsideCardMessage* in_msg)
{
	memcpy(reader->PAI_Cache_dat[reader->PAI_position].uids, in_msg->uids,
			sizeof(MahjongInfo) - 1);
	// 因牌南北的ID和手机定义不同,因此需要转换ID
	reader->PAI_Cache_dat[reader->PAI_position].dsfID =
			ConvertMahjongId2PhoneId(in_msg->dsfid);
#if 0
	TileInfo info =
	{	currentReader, reader->PAI_position, reader->PAI_Cache_dat[reader->PAI_position]};
	phoneBle->SendTilesReaderData(&info);
#endif
	reader->PAI_position++;
	RP_num++;               //读到的总牌数加一
	RP_ovtimer.reset();
}

void AttempAddPaiToCache(const RF_RxInsideCardMessage* in_msg)
{
	struct ReaderMessage* reader = &reader_monitor[in_msg->reader - 1];

	if (in_msg->pai_length >= in_msg->position
			&& (in_msg->position == reader->PAI_position + 1))
	{      // TODO: 有可能对应位置为0xFF???, 上面增加了判断牌0xff的条件,看是否需要删除!
		if (in_msg->dsfid != 0xff)
		{
			if (reader->PAI_position < reader->PAI_num)
			{
				memcpy(&reader->PAI_Cache_dat[reader->PAI_position],
						&in_msg->dsfid, sizeof(MahjongInfo));
				reader->PAI_Cache_dat[reader->PAI_position].dsfID =
						ConvertMahjongId2PhoneId(in_msg->dsfid);
#if 0
				TileInfo info =
				{	currentReader, reader->PAI_position, reader->PAI_Cache_dat[reader->PAI_position]};
				phoneBle->SendTilesReaderData(&info);
#endif
				reader->PAI_position++;
				RP_num++;               //读到的总牌数加一
				RP_ovtimer.reset();
			}
			// 指示收到牌
			pai_led_timer.reset();
			*led_X = 1;
		}
		else
		{
			// 不应跑到这里
			assert(false);
		}
	}
	else
	{
		// 可能接收有异常,如灵敏度低导致读到下一盘数据
		if (in_msg->pai_length == 0 && reader->PAI_position >= 1)
		{
			Reader_FinishXP();
		}
	}
}

void AttempAddPaiToCacheEx(const RF_RxInsideCardMessage* in_msg)
{
	struct ReaderMessage* reader = &reader_monitor[in_msg->reader - 1];

	if (in_msg->dsfid != 0xff)
	{
		if (RP_num == 0)	// 第一张牌
		{
			if (in_msg->position == 1)
			{
				AddOnePaiToCache(reader, in_msg);
				pai_color = in_msg->dsfid & 0x80;
			}
		}
		else
		{
			// 判断颜色是否相同,相同才可能是同一局的牌
			if (!IsColorChanged(in_msg->dsfid))
			{
				if (in_msg->pai_length >= in_msg->position)
				{
					if (in_msg->position == reader->PAI_position + 1)
					{
						AddOnePaiToCache(reader, in_msg);
					}
					else
					{
						// 不应跑到这里
					}
				}
				else
				{
					// 颜色相同, 但总牌数少了, 也应是另外一盘
					Reader_FinishXP();
					for (int i = 0; i < 4; i++)
					{
						reader_monitor[i].working = true;
					}
				}
			}
			else
			{
				// 颜色不对,重新设置,但不发送清0了.
				// 本张牌也要作废,需要重新读取
				Reader_FinishXP();
				for (int i = 0; i < 4; i++)
				{
					reader_monitor[i].working = true;
				}
			}
		}
	}
	else
	{
		// 总牌数比以前少,认为已经重新洗过牌了
		if (in_msg->pai_length < reader->PAI_position)
		{
			Reader_FinishXP();
			// 下面读头认为已经工作了,不需重新清0
			for (int i = 0; i < 4; i++)
			{
				reader_monitor[i].working = true;
			}
		}
	}
}

/**
 * 桌内读头接收数据
 * 有数据: 06 04 22 22 8C 42 CC
 * 无数据: 06 04 21 22 FF 41 BF
 * 回kReaderPaiClear查询(仅一次): 04 04 00 F1 F1
 * @param msg
 * 0C 01 90 25 04 01 93 AB AD 63 9F 40 94
 */
void Reader_HandleInsideMessage(const RF_Message* msg)
{
	assert(currentReader >= 1 && currentReader <= 4);

	struct ReaderMessage* reader = &reader_monitor[msg->reader - 1];
	reader->rxrssi = cc1101->rssi();
	RF_rxst = kIntReaderRxOk;
	switch (msg->command)
	{
	case kCmdReaderResetAck:
		ClearReaderData(reader);
		reader->working = true;
		break;

	case kCmdReadPaiAck:
		AttempAddPaiToCacheEx((const RF_RxInsideCardMessage*) msg);
		if (0)
		{
			const RF_RxInsideCardMessage* in_msg =
					(const RF_RxInsideCardMessage*) msg;
			if (in_msg->pai_length >= in_msg->position
					&& (in_msg->position == reader->PAI_position + 1))
			{      // TODO: 有可能对应位置为0xFF???, 上面增加了判断牌0xff的条件,看是否需要删除!
				if (in_msg->dsfid != 0xff)
				{
					if (reader->PAI_position < reader->PAI_num)
					{
						memcpy(&reader->PAI_Cache_dat[reader->PAI_position],
								&in_msg->dsfid, sizeof(MahjongInfo));
						reader->PAI_Cache_dat[reader->PAI_position].dsfID =
								ConvertMahjongId2PhoneId(in_msg->dsfid);
#if 0
						TileInfo info =
						{	currentReader, reader->PAI_position, reader->PAI_Cache_dat[reader->PAI_position]};
						phoneBle->SendTilesReaderData(&info);
#endif
						reader->PAI_position++;
						RP_num++;               //读到的总牌数加一
						RP_ovtimer.reset();
					}
					// 指示收到牌
					pai_led_timer.reset();
					*led_X = 1;
				}
				else
				{
					// 不应跑到这里
					assert(false);
				}
			}
			else
			{
				// 可能接收有异常,如灵敏度低导致读到下一盘数据
				if (in_msg->pai_length == 0 && reader->PAI_position >= 1)
				{
					Reader_FinishXP();
				}
			}
		}
		break;

	default:
		break;
	}
}
#endif

/**
 *
 * @param msg
 * @note 0A 04 02 01 0D 5B 6B 4D 9F 49 AB
 */
void Reader_DispatchMessage(const uint8_t* msg)
{
	const RF_Message* info = (const RF_Message*) msg;

	DBG("Reader%d Received:", currentReader);DBGARR(msg, msg[0]+1);

	if (info->reader == currentReader)
	{
		Reader_HandleInsideMessage(info);
		RP_mode = kRequestReader;
	}
	else
	{
		cc1101->RXMode();
	}
}

void SendGroupTileToPhone()
{
	bool isAllSend = true;

	struct ReaderMessage* reader;
	for (int i = 0; i < kGroup; i++)
	{
		reader = &reader_monitor[i];
		if (!reader->is_handled)
		{
			// 最后一组牌不允许先发送
			if (i == kGroup - 1 && !isAllSend)
				break;

			if (reader->PAI_position >= reader->PAI_num)
			{
//                phone->SendGroupData(i + 1, reader->PAI_Cache_dat, reader->PAI_position);
				memcpy(phone_pais[i].pais, reader->PAI_Cache_dat,
						reader->PAI_position * sizeof(MahjongInfo));
				if (reader->PAI_num > reader->PAI_position)
				{
					memset(&phone_pais[i].pais[reader->PAI_position], 0xff,
							sizeof(MahjongInfo)
									* (reader->PAI_num - reader->PAI_position));
				}
				phone_pais[i].is_send = false;
				phone_pais[i].send_count = 0;
				phone_pais[i].count = reader->PAI_num;
				reader->is_handled = true;
			}
			else
			{
				isAllSend = false;
//                break;
			}
		}
	}
}

/**
 *
 */
void Reader_CheckReceived()
{
	struct ReaderMessage* reader;

	//超时判断
	RP_mode = kRequestReader;

	reader = &reader_monitor[currentReader - 1];
	if (RF_rxst == kIntReaderRxFail)       //判断桌内是否接正确的回应包
	{
		reader->PAI_err++;

		if (!reader->working && (reader->PAI_err > 3))
		{
			reader->PAI_position = 0x00;
			reader->working = true;
			reader->PAI_err = 0;
		}
	}

	SendGroupTileToPhone();

	for (int i = 0; i < 4; i++)
	{
		if (++currentReader > 4)
		{
			currentReader = 1;
		}
		if (reader_monitor[currentReader - 1].PAI_position
				< reader_monitor[currentReader - 1].PAI_num)
		{
			break;
		}
	}
}

const int kReadPaiMiss1OverTime = 30000;
const int kReadPaiMiss2OrMoreOverTime = 35000;
const int kReadPaiMissMuchOverTime = 40000;

void Reader_CheckOverTime()
{
	//判断洗牌是否超时
	switch (TotalCardNum - RP_num)
	{
	case 0:
		Reader_FinishXP();
		break;

	case 1:
		if (RP_ovtimer.read_ms() > kReadPaiMiss2OrMoreOverTime)
		{
			Reader_FinishXP();
		}
		break;

	case 2:
	case 3:
	case 4:
	case 5:
		if (RP_ovtimer.read_ms() > kReadPaiMiss2OrMoreOverTime)
		{
			Reader_FinishXP();
		}
		break;

	default:
		if (RP_ovtimer.read_ms() > kReadPaiMissMuchOverTime && RP_num > 0)
		{
			RP_ovtimer.reset();         //停止并清计时

			for (int i = 0; i < kGroup; i++)
			{
				reader_monitor[i].working = false;
			}

			DAT_Load();             //加载数据到数据缓冲区

			IsXPFinished = true;          //洗牌结束
			RP_num = 0;
		}
		break;
	}
}

#define EnableVoice() spk_en = 0
#define DisableVoice() spk_en = 1

#define VOLT_FULL     41500
#define VOLT_HIGH     40000
#define VOLT_LOW      36000
void IndicateVolt(int volt)
{
	DBG("[%d]v=%X\r\n", g_timer.read_ms(), volt);
	// 12位AD, 4096, 减去一个底数（电池2.5V就认为没电了）
	if (volt >= VOLT_FULL)
	{
	}
	else if (volt >= VOLT_HIGH)
	{
	}
	else if (volt >= VOLT_LOW)
	{
	}
	else
	{
	}
}

#if BATTERY_EN == 1
/**
 * 监控电池电压
 */
uint16_t GetBatteryVoltage()
{
	return battery_volt_in.read_u16();
}

bool IsLowVoltage(int volt)
{
	const int kLowVoltageThreshold = 500;

	return volt < kLowVoltageThreshold;
}

/**
 * 读取带隙电压，不考虑太高精度，认为其是1.2V标准，用于校正电池电压检测值。
 * @return
 */
int ReadBandgapVolt()
{
	ADC_HandleTypeDef hadc;

	hadc.Instance = ADC1;
	hadc.Init.ClockPrescaler = 0;
	return HAL_ADC_GetValue(&hadc);
}

/**
 * 是否低电压
 * @param volt
 * @return
 */
bool IsLowBattery(int volt)
{
	return false;
}

/**
 * 监控电池电压
 */
void MonitorBatteryVoltage()
{
//  int volt_1V2 =
	int volt = battery_volt_in.read_u16();
	if (IsLowBattery(volt))
	{
		DBG("[%X]Low battery\r\n", g_timer.read_ms());
		PowerOff();
	}
	else
	{
		IndicateVolt(volt);
	}
}

/**
 * 唤醒测试
 */
static void TestWakeup()
{
	CommRF* tiles_;
	TileFullId id;
	id.year = 15;
	id.month = 5;
	id.day = 14;
	id.id = 3;
	tiles_->set_full_id(0, id);
	tiles_->set_full_id(1, id);
//  tiles_->set_tiles_cnt(40);
	phoneBle->WakeupTiles();
}

/**
 * 语音播放测试
 */
void TestVoicePlay()
{
	EnableVoice();
	wait_ms(50);

	for (int i = 0; i < 100; i++)
	{
		while (voice.IsBusy())
		;

		voice.play(i);
	}
}
#endif

/**
 * 读取数据到环形缓冲区
 */
void ReceivedRFFrame()
{

}

void ToggleLed()
{
	static int cnt = 0;

	switch (sys_status)
	{
	case kSysPowerOn:          // 上电
		if ((++cnt % 20) == 0)
		{
			*led_power = 1;
		}
		else
		{
			*led_power = 0;
		}
		break;

	case kSysWakeuped:      // 唤醒
		if ((++cnt % 5) == 0)
		{
			*led_power = !led_power->read();
		}
		break;

	case kSysIsLink:
		if (phoneBle->is_connected() == kPhoneConnected)
		{
			*led_power = 1;
		}
		else
		{
			if ((++cnt % 20) == 0)
			{
				*led_power = !led_power->read();
			}
		}
		break;

	case kSysClosing:
		if ((++cnt % 5) == 0)
		{
			*led_power = !led_power->read();
		}
		break;

	case kSysLearnMode:
		if ((++cnt % 4) == 0)
		{
			*led_power = !led_power->read();
			*led_X = *led_power;
		}
		break;

	case kSysDisLink:
	case kSysWaitConnectPhone:
		if ((++cnt % 20) == 0)
		{
			*led_power = !led_power->read();
		}
		break;

	case kSysClosed:
		*led_power = 0;
		break;

	case kSysWifiReceived:
		cnt = 0;
		*led_power = 1;
		sys_status = kSysWifiLedOff;
		break;

	case kSysWifiLedOff:
		if (++cnt >= 3)
		{
			*led_power = 0;
			sys_status = kSysNormal;
		}
		break;

	default:
		if (phoneBle->is_connected())
		{
			*led_power = 1;
		}
		else
		{
			*led_power = (cnt++ % 25) != 0;
		}
		break;
	}
}

/**
 * 系统自检
 */
void SystemSelfTest()
{
	// 检测各个IO口电平是否正常
	if (led_power->read() == 1)
		printf("led1=1; \r\n");
	if (*wifi_rst == 1)
		printf("wifi_rst=1; \r\n");
	if (*rfRst == 1)
		printf("rfRst=1; \r\n");
}

void RF_Init()
{
	cc1101 = new CC1101(SPI_MOSI, SPI_MISO, SPI_SCK, PA_1, PA_4);
	cc1101->init(&rfNavigator);
}

#if 0
extern uint8_t eepromDatas[];
void TestFlashOperation()
{
	// 测试Flash的擦除和读写
	const uint8_t xdatas[] =
	{	0x55, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0xff, 0x00, 0xaa};
	uint8_t bufs[sizeof(xdatas) + 10];
	if (!EraseFlash((uint32_t) eepromDatas))
	{
		phone_uart->printf("\r\nErase Error!\r\n");
	}
	ReadFromFlash((uint32_t) eepromDatas, bufs, sizeof(xdatas));
	phone_uart->printf("\r\nOrgin:");
	for (int i = 0; i < sizeof(xdatas); i++)
	{
		phone_uart->printf(" %hhX", eepromDatas[i]);
	}
	phone_uart->printf("\r\n");
	if (!WriteToFlash((uint32_t) eepromDatas, xdatas, sizeof(xdatas)))
	{
		phone_uart->printf("Write Flash Timeout!\r\n");
	}
	memset(bufs, 0, sizeof(bufs));
	phone_uart->printf("\r\nAfter:");
	for (int i = 0; i < sizeof(xdatas); i++)
	{
		phone_uart->printf(" %hhX", eepromDatas[i]);
	}
	phone_uart->printf("\r\n");
}
#endif

static IWDG_HandleTypeDef IwdgHandle;
void WDT_Init()
{
	IwdgHandle.Instance = IWDG;

	IwdgHandle.Init.Prescaler = IWDG_PRESCALER_256;
	IwdgHandle.Init.Reload = 255;
	IwdgHandle.Init.Window = IWDG_WINDOW_DISABLE;

	if (HAL_IWDG_Init(&IwdgHandle) != HAL_OK)
	{
	}

	/*##-4- Start the IWDG #####################################################*/
	if (HAL_IWDG_Start(&IwdgHandle) != HAL_OK)
	{
	}
}

/**
 * 读牌超时时间. 当读头少时, 时间会长一点.
 * @return
 */
int Reader_GetPollingOverTime()
{
	int unfinish_reader = 4;

	for (int i = 0; i < 4; i++)
	{
		if (reader_monitor[i].PAI_position >= reader_monitor[i].PAI_num)
		{
			unfinish_reader--;
		}
	}

	if (unfinish_reader == 0)
		return 240;
	else if (unfinish_reader == 1)
		return 180;
	else
		return 240 / unfinish_reader;
}

ReaderResult Reader_GetMessage(uint8_t* msg, uint8_t len)
{
	ReaderResult ack = kReaderNoMessage;

	if (RP_mode == kRequestReader)
	{
		SendRFCommand();
	}
	else if (RP_mode == kWaitTxActive)
	{
		if (cc1101->GetGDO2() == 1 || reader_tmr.read_ms() > 30)
		{
			RP_mode = kWaitTxFinish;
		}
	}
	else if (RP_mode == kWaitTxFinish)
	{
		if (cc1101->GetGDO2() == 0)
		{
			RP_mode = kWaitReaderAck;
			cc1101->WriteReg(CCxxx0_IOCFG2, 0x07);
			cc1101->RXMode();
			reader_tmr.reset();
		}
	}
	else if (RP_mode == kWaitReaderAck)
	{
		if (ReceivedRFAck(msg, len - 1))
		{
			ack = kReaderMessageOK;
		}
		else if (reader_tmr.read_ms() > Reader_GetPollingOverTime())
		{
			ack = kReaderMessageOverTime;
		}
	}
	return ack;
}

void PowerOn_Init()
{
	wifi_rst = new DigitalOut(PB_4);
#if BOARD_VERSION == 1
	led_X = new DigitalOut(PB_14);
	led_power = new DigitalOut(PB_15);

	rfRst = new DigitalOut(PA_8);

	Speech_Init(PA_12, PA_11, PA_8);
	rf_power = new DigitalOut(PB_13);
	key = new SimpleKey(PA_0);
	key_info = new SimpleKey(PB_11);
	reader_reset = new DigitalOut(PB_2);;
#elif BOARD_VERSION == 2
	led_power = new DigitalOut(PA_11);
	led_X = new DigitalOut(PA_12);

	rfRst = new DigitalOut(PA_8);

	Speech_Init(PC_13, PC_14, PA_0);

	reader_power = new DigitalOut(PA_8);
	rf_power = new DigitalOut(PB_3);
	key = new SimpleKey(PB_15);
	key_info = new SimpleKey(PB_12);
	reader_reset = new DigitalOut(PA_15);
#else
#error "Board Version Not Set!"
#endif
	*led_X = 0;
	// 为避免串口高电平漏电到WIFI模块导致复位不良，先转为普通IO口
	DigitalOut txio(PA_2);
	DigitalOut rxio(PA_3);

	ticker.attach(ToggleLed, 0.02);
	*led_power = 1;
	sys_status = kSysPowerOn;
	txio = 0;
	rxio = 0;

	*wifi_rst = 0;
	*rfRst = 0;

	// 检测是否开机(快速按3下按键)
	g_timer.start();
}

void Reader_Init()
{
	currentReader = 1;
	RP_num = 0;
}

inline void WDT_Clear()
{
	HAL_IWDG_Refresh(&IwdgHandle);
}

void Powered_Init()
{
	memcpy(&contents, (void*) eeprom_start, sizeof(EepromContent));

	if (contents.east_cnt > 40 || contents.north_cnt > 40
			|| contents.west_cnt > 40 || contents.south_cnt > 40
			|| contents.east_cnt < 10 || contents.north_cnt < 10
			|| contents.west_cnt < 10 || contents.south_cnt < 10)
	{
		contents.east_cnt = kDefaultMahjongCount[0];
		contents.north_cnt = kDefaultMahjongCount[1];
		contents.west_cnt = kDefaultMahjongCount[2];
		contents.south_cnt = kDefaultMahjongCount[3];
	}

	if (contents.rf_channel > 8 || contents.rf_channel < 1)
	{
		contents.rf_channel = 1;
	}

	if (contents.ssid > 8 || contents.ssid < 1)
	{
		contents.ssid = 1;
	}

	if (contents.volume > 7)
	{
		contents.volume = 7;
	}

	contents.res = 0xff;
	contents.res2 = 0xffffffff;
//    spi.frequency(500000);
//    spi.format(8, 1);

	phone_uart = new BufferedSerial(PA_2, PA_3, 800);

#if USE_BLE == 1
    ble = new BLE(phone_uart, circular_wifi, PB_6, PB_5);
	phoneBle = new CommBLE(ble);
    phone_uart->baud(115200);
#else
    wifi = new WifiObj(phone_uart, circular_wifi, PB_6, PB_5);
	phoneBle = new CommWifi(wifi);
    phone_uart->baud(115200);
    DBGS("System Reset:\r\n");
    phone_uart->printf("Wifi Serial\r\n");
#endif
	debug_uart = phone_uart;

	cc1101 = new CC1101(SPI_MOSI, SPI_MISO, SPI_SCK, PA_1, PA_4);


	// 设置串口优先级最高。
	NVIC_SetPriority(USART1_IRQn, 0);

	wait_ms(50);

	*wifi_rst = 1;
	*rfRst = 1;

	wait_ms(50);

	cc1101->init(&rfNavigator);
	cc1101->SetChannel(ConvertIndexToRFChannel(contents.rf_channel));

	phoneBle->SetSsid(contents.ssid);
	phoneBle->Init();

	sys_status = kSysWaitConnectPhone;
	DBG("Tong Start[%s-%d]\r\n", __DATE__, g_timer.read_ms());
	power_tmr.start();
	Navigator_Init();
	Reader_Init();
	HiReader_Init();
}

/**
 * 发送数据给手机
 */
void HiReader_DispatchMessage(HiReader_DataMessage* msg)
{
	switch (msg->command)
	{
	case kHiReaderTileReceived:
		if (kSendAllPaiAtOneTime)
		{
			unsend_pai[unsend_count] = *(MahjongInfo*) msg->extras;
			unsend_pai[unsend_count].dsfID = ConvertMahjongId2PhoneId(
					unsend_pai[unsend_count].dsfID);
			unsend_count++;
			unsend_timer.reset();
		}
		else
		{
			unsend_pai[0] = *(MahjongInfo*) msg->extras;
			unsend_pai[0].dsfID = ConvertMahjongId2PhoneId(unsend_pai[0].dsfID);
			phoneBle->SendAdminReaderData(&unsend_pai[0]);
		}
		pai_led_timer.reset();
		*led_X = 1;
		break;

	case kHiReaderOpenAck:
	case kHiReaderCloseAck:

		break;

	default:
		break;
	}
}

#if USE_BLE == 1
void HiReader_SendPai()
{
    const int kMaxWaitSendTime = 200;

    if (kSendAllPaiAtOneTime)
    {
        if (unsend_count >= kMaxAdminPaiCount
                || (unsend_count > 0
                        && unsend_timer.read_ms() > kMaxWaitSendTime))
        {
            // 满足发送条件就马上转到手机发送缓冲里
            phone_live_pai.count = unsend_count;
            phone_live_pai.is_send = false;
            phone_live_pai.is_wait = false;
            memcpy(phone_live_pai.pais, unsend_pai,
                    unsend_count * sizeof(MahjongInfo));
            unsend_count = 0;
            phone_live_pai.send_count = 0;
        }
    }
}
#else
void HiReader_SendPai()
{
	const int kMaxWaitSendTime = 200;

	if (phoneBle->is_connected() && kSendAllPaiAtOneTime)
	{
		if (unsend_count >= kMaxAdminPaiCount
				|| (unsend_count > 0
						&& unsend_timer.read_ms() > kMaxWaitSendTime))
		{
			// 满足发送条件就马上转到手机发送缓冲里
			phone_live_pai.count = unsend_count;
			phone_live_pai.is_send = false;
			phone_live_pai.is_wait = false;
			memcpy(phone_live_pai.pais, unsend_pai,
					unsend_count * sizeof(MahjongInfo));
			unsend_count = 0;
			phone_live_pai.send_count = 0;
		}
	}
}
#endif

uint8_t GetBatteryVolt()
{
	return 0;
}

uint16_t Formula_CRC16(uint8_t *p, uint8_t len)
{
	uint16_t Fml_CRC16 = 0;
	while (len--)
	{
		for (int i = 0x80; i != 0; i >>= 1)
		{
			if ((Fml_CRC16 & 0x8000) != 0)
			{
				Fml_CRC16 <<= 1;
				Fml_CRC16 ^= 0x1021;
			}
			else
			{
				Fml_CRC16 <<= 1;
			}
			if ((*p & i) != 0)
			{
				Fml_CRC16 ^= 0x1021;
			}
		}
		p++;
	}
	return Fml_CRC16;
}

uint32_t Formula_85(uint8_t* D)
{
	uint32_t dat = 0;
	uint8_t* Result = (uint8_t*) &dat;
	Result[0] = C[0] + D[0] | (D[2] ^ (D[8] & D[9]) ^ D[8]);
	Result[1] = C[1] + D[5] - D[6] | (D[7] ^ (D[10] & D[11]));
	Result[2] = C[2] + D[4];
	Result[3] = C[3] + D[3] - D[1] ^ D[8];

	return dat;
}

/**
 * 唯一号加密
 */
bool UIDEncrypt()
{

	debug_uart->printf("UID is:");
	for (int i = 0; i < 12; i++)
	{
		debug_uart->printf(" %hhx", mcu_uid[i]);
	}
	debug_uart->printf("\r\n");

	uint32_t shouldbe = Formula_85(mcu_uid);
	if (contents.uids == shouldbe)
	{
		return true;
	}
	else
	{
		contents.uids = 0xffffffff;
		return false;
	}
}

void Phone_SendStatus()
{
	// 命令字  机型  硬件版本  软件版本  电池电压    预留数据
	uint8_t buf[9] =
	{ PhoneCmd_StatusAck, kDeviceID, kFirmwareVer, kHardwareVer };
	buf[4] = GetBatteryVolt();
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;
	buf[8] = 0;
	phoneBle->SendPacket(buf, 9);
}

void Phone_AckCmd()
{
	uint8_t buf[2] =
	{ PhoneCmd_VoicePlay | 0x80, 0 };
	phoneBle->SendPacket(buf, 2);
}

void Phone_SendHashTable(int hash_in)
{
	// 参考测试：55 05 04 00 00 07 22 CE
	// 回应：55-05-84-0A-00-64-00-09
	uint8_t tx_buf[4] =
	{ PhoneCmd_VerifyHashTableAck, 0 };
	tx_buf[2] = tab_encrypt[hash_in % 512];
	tx_buf[3] = 0;
	phoneBle->SendPacket(tx_buf, 4);
}

void Phone_SetVolume(uint8_t vol)
{
	assert(vol < 8);
	if (contents.volume != vol)
	{
		contents.volume = vol;
		SaveContentsToFlash();
	}

	uint8_t buf[2] =
	{ PhoneCmd_SetVolumeAck, vol };
	phoneBle->SendPacket(buf, 2);
}

/**
 * 设置牌数量
 */
void Phone_SetPaiCount(PhoneMessage_SetPaiCount* msg)
{
	if (msg->east_cnt != reader_monitor[0].PAI_num
			|| msg->north_cnt != reader_monitor[1].PAI_num
			|| msg->west_cnt != reader_monitor[2].PAI_num
			|| msg->south_cnt != reader_monitor[3].PAI_num)
	{
		reader_monitor[0].PAI_num = msg->east_cnt;
		reader_monitor[1].PAI_num = msg->north_cnt;
		reader_monitor[2].PAI_num = msg->west_cnt;
		reader_monitor[3].PAI_num = msg->south_cnt;
		TotalCardNum = reader_monitor[0].PAI_num + reader_monitor[1].PAI_num
				+ reader_monitor[2].PAI_num + reader_monitor[3].PAI_num;

		contents.east_cnt = msg->east_cnt;
		contents.north_cnt = msg->north_cnt;
		contents.west_cnt = msg->west_cnt;
		contents.south_cnt = msg->south_cnt;
		SaveContentsToFlash();
	}
	uint8_t buf[2] =
	{ PhoneCmd_SetTilesCountAck, TotalCardNum };
	phoneBle->SendPacket(buf, 2);
}

void SaveRFChannelToFlash(uint8_t rf_channel)
{
	assert(rf_channel >= 1 && rf_channel <= 8);
	if (rf_channel != contents.rf_channel)
	{
		contents.rf_channel = rf_channel;
		SaveContentsToFlash();
	}
}

/**
 * 进入学习模式,不停发送学习指令10秒,检查读头的回应,如果全部学习成功就退出学习模式.
 * 在学习模式里,不在响应其它输入,如高功率读头和WIFI模块.
 */
void Phone_EnterStudyMode(uint8_t rf_channel)
{
	Timer led_tmr;
	assert(rf_channel >= 1 && rf_channel <= 8);
	struct LearnAckMessage
	{
		uint8_t length;
		uint8_t reader;
		uint8_t command;
		uint8_t transID;
		uint8_t channel_idx;
		uint8_t channel_val;
		uint8_t dsfid;
	};

	uint8_t phone_tx_buf[3] =
	{ PhoneCmd_EnterStudyMode | 0x80, rf_channel, 0 };
	phoneBle->SendPacket(phone_tx_buf, 3);

	if (HiReader_IsOpen())
	{
		HiReader_CloseRF();
	}

	SaveRFChannelToFlash(rf_channel);

//    cc1101->FlushRX();
//    cc1101->init(&rfNavigator);
	cc1101->SetChannel(kStudyChannel);
	RF_TxLearnMessage study_msg;
	study_msg.length = sizeof(study_msg) - 1;
	study_msg.channel = rf_channel;
	study_msg.reader = 0;
	study_msg.mode = 0;

	// 指明只有未学习过的读头才学习或者全部学习
	Timer tmr;
	tmr.start();
	uint8_t rx_buf[32];
	int isLearned[4] =
	{ 0, 0, 0, 0 };
	Speech_Add(kVoiceLearn);

	RF_rxst = kIntReaderRxFail;
	int status = 0;
	reader_tmr.start();
	led_tmr.start();

//    SystemStatus sys_bak = sys_status;
//    sys_status = kSysLearnMode;

	while (tmr.read_ms() < 600000)
	{
		Speech_Play();
//        WDT_Clear();
		// 快速闪烁LED
		if (led_tmr.read_ms() > 100)
		{
			led_tmr.reset();
			*led_X = !(*led_X);
		}

		switch (status)
		{
		case 0:
			cc1101->WriteReg(CCxxx0_IOCFG2, 0x06);
			do
			{
				if (++study_msg.command > 4)
				{
					study_msg.command = 1;
				}
			} while (isLearned[study_msg.command - 1] >= 2);
			study_msg.mode = isLearned[study_msg.command - 1];

			study_msg.transID = ++rf_tx_transID;
			study_msg.crc = CalcCrc(&study_msg, sizeof(study_msg) - 1);
			cc1101->SendPacketAsync(&study_msg, sizeof(study_msg));
			reader_tmr.reset();
			status++;
			break;

		case 1:
			if (cc1101->GetGDO2() == 1 || reader_tmr.read_ms() > 30)
			{
				reader_tmr.reset();
				status++;
			}
			break;

		case 2:
			if (cc1101->GetGDO2() == 0 || reader_tmr.read_ms() > 30)
			{
				cc1101->WriteReg(CCxxx0_IOCFG2, 0x07);
				cc1101->RXMode();
				reader_tmr.reset();
				status++;
			}
			break;

		case 3:
			if (reader_tmr.read_ms() > 10
					&& cc1101->ReceivePacket(rx_buf, sizeof(rx_buf)))
			{
				// 06 03 83 4A 07 BF 74
				const uint8_t tab_voices[] =
				{ kVoiceEast, kVoiceNorth, kVoiceWest, kVoiceSouth };
				LearnAckMessage* msg = (LearnAckMessage*) rx_buf;
				if (msg->command == (msg->reader | 0x80) && msg->reader >= 1
						&& msg->reader <= 4)
				{
					if (isLearned[msg->reader - 1] == 0)
					{
						Speech_Add(kVoiceLearn);
						Speech_Add(tab_voices[msg->reader - 1]);
						Speech_Add(kVoiceSuccess);
					}
					isLearned[msg->reader - 1]++;
					status = 0;
				}
				else
				{
					cc1101->RXMode();
				}
			}
			else if (reader_tmr.read_ms() > 100)
			{
				status = 0;
			}
			break;

		default:
			break;
		}

		if (isLearned[0] > 0 && isLearned[1] > 0 && isLearned[2] > 0
				&& isLearned[3] > 0)
		{
			break;
		}
	}
//    sys_status = sys_bak;

	cc1101->SetChannel(ConvertIndexToRFChannel(contents.rf_channel));

	phone_tx_buf[2] = 0x80;
	if (isLearned[0] > 0)
		phone_tx_buf[2] |= 0x01;
	if (isLearned[1] > 0)
		phone_tx_buf[2] |= 0x02;
	if (isLearned[2] > 0)
		phone_tx_buf[2] |= 0x04;
	if (isLearned[3] > 0)
		phone_tx_buf[2] |= 0x08;
	phoneBle->SendPacket(phone_tx_buf, 3);
}

void Phone_SetVoiceVolume(int vol)
{
	assert(vol < 8);

	Speech_SetVolume(vol);
	Speech_AddNum(1);
	Speech_AddNum(2);
	Speech_AddNum(3);
	Speech_AddNum(4);
}

/**
 * 设置SSID
 */
void Phone_ChangeSSID(int ssid)
{
#if 0
	assert(ssid >= 1 && ssid <= 8);

	if (ssid != phoneBle->ssid())
	{
		contents.ssid = ssid;
		SaveContentsToFlash();
	}
	uint8_t buf[2] =
	{ PhoneCmd_ChangeSSID, TotalCardNum };
	phoneBle->SendPacket(buf, 2);

	phoneBle->ChangeSsid(ssid);
	sys_status = kSysWaitConnectPhone;
#endif
}

void Phone_DispatchMessage(uint8_t* msg)
{
	PhoneMessage* phone_msg = (PhoneMessage*) msg;
	switch (phone_msg->cmd)
	{
	case PhoneCmd_VerifyHashTable:   // 读码表
		Phone_SendHashTable(phone_msg->extras[2] * 256 + phone_msg->extras[1]);
		break;

	case PhoneCmd_Status:
		Phone_SendStatus();
		break;

	case PhoneCmd_EnterStudyMode:
		if (phone_msg->extras[0] >= 1 && phone_msg->extras[0] <= 8)
		{
			Phone_EnterStudyMode(phone_msg->extras[0]);
		}
		break;

	case PhoneCmd_ChangeSSID:
		if ((phone_msg->extras[0] ^ phone_msg->extras[1]) == 0xff)
		{
			Phone_ChangeSSID(phone_msg->extras[0]);
		}
		break;

	case PhoneCmd_VoicePlay:
		// 例子: 02 08 0E 10 20 F0 0D C9 03
		// 播放放到主程序处理，因语音需要时间
		Speech_Add(phone_msg->extras, phone_msg->len - 6);
		{
			uint8_t buf[2] =
			{ PhoneCmd_VoicePlay | 0x80, 0 };
			phoneBle->SendPacket(buf, 2);
		}
		break;

	case PhoneCmd_SetTilesCount:
		Phone_SetPaiCount((PhoneMessage_SetPaiCount*) phone_msg);
		break;

	case PhoneCmd_SetVolume:
		Phone_SetVolume(phone_msg->extras[0]);
		break;

	case PhoneCmd_TilesReaderDataAck:
	{
		// 02 08 A1 00 00 F0 18 43 03
		PhoneMessage_AckPais* ack_msg = (PhoneMessage_AckPais*) msg;
		if (ack_msg->len == 8 && (ack_msg->transIdFlag == 0xf0))
		{
			if (ack_msg->ack >= 1 && ack_msg->ack <= 4)
			{
				if (ack_msg->transId == phone_pais[ack_msg->ack - 1].transID)
				{
					phone_send_wait_ack = false;
					phone_pais[ack_msg->ack - 1].is_send = true;
				}
				else
				{
					// 流水号不对
				}
			}
		}
		else if (ack_msg->len == 6)
		{
			// 不带流水号!
			if (ack_msg->ack >= 1 && ack_msg->ack <= 4)
			{
				phone_send_wait_ack = false;
				phone_pais[ack_msg->ack - 1].is_send = true;
			}
		}
	}
		break;

	case PhoneCmd_AdminReaderDataAck:
	{
		PhoneMessage_AckPais* ack_msg = (PhoneMessage_AckPais*) msg;
		if (ack_msg->len == 8 && (ack_msg->transIdFlag == 0xf0)
				&& (ack_msg->transId == phone_live_pai.transID))
		{
			if (ack_msg->ack == 5)
			{
				phone_live_pai.is_wait = false;
				phone_live_pai.is_send = true;
			}
		}
		else if (ack_msg->len == 6)
		{
			// 不带流水号!
			if (ack_msg->ack == 5)
			{
				phone_live_pai.is_wait = false;
				phone_live_pai.is_send = true;
			}
		}
	}
		break;

	case PhoneCmd_HeartBeatAck:
		break;

	default:
		break;
	}
}

void Phone_Heartbeat()
{
	if (phoneBle->GetNoTxTime() > 6000)
	{
		phoneBle->SendHeartBeat();
	}
}

uint32_t sys_count = 0;
extern "C" void SysTick_Handler()
{
	sys_count++;
}

void ReportDeviceInfo()
{
	Speech_AddNum(TotalCardNum);
	Speech_Add(kVoiceVersion);
	Speech_AddNum(kFirmwareVer);
	Speech_Add(kVoiceAddress);
	Speech_AddNum(phoneBle->ssid());
	Speech_Add(kVoiceChannel);
	Speech_AddNum(contents.rf_channel);
}

void ReportDeviceUID()
{
	uint8_t rfbuf_tx[20];
	RF_Message* msg = (RF_Message*) rfbuf_tx;
	msg->length = 16;
	msg->reader = 0x11;
	msg->command = 0x11;
	msg->transID = 0x11;
	for (int i = 0; i < 12; i++)
	{
		msg->extras[i] = mcu_uid[i];
	}

	msg->extras[12] = CalcCrc(msg, 15);

	cc1101->SendPacketAsync(msg, 16);
}

void SendInformation()
{
	uint8_t rfbuf_tx[20];
	RF_Message* msg = (RF_Message*) rfbuf_tx;
	msg->length = 0x0a;
	msg->reader = 0x10;
	msg->command = 0x10;
	msg->transID = 0x10;
	msg->extras[0] = reader_monitor[0].PAI_num;
	msg->extras[1] = reader_monitor[1].PAI_num;
	msg->extras[2] = reader_monitor[2].PAI_num;
	msg->extras[3] = reader_monitor[3].PAI_num;
	msg->extras[4] = kHardwareVer;
	msg->extras[5] = kFirmwareVer;

	msg->extras[6] = CalcCrc(msg, 9);

	cc1101->SendPacketAsync(msg, 10);

	phone_uart->printf("WInfo:%hhX %hhX %hhX %hhX\r\n",
			reader_monitor[0].PAI_num, reader_monitor[1].PAI_num,
			reader_monitor[2].PAI_num, reader_monitor[3].PAI_num);
}

void SendCC1101Params()
{
	uint8_t buf[60];
	for (int i = 0; i < 47; i++)
	{
		buf[i] = cc1101->ReadReg(i);
	}
	for (int i = 0; i < 47; i++)
	{
		phone_uart->putc(buf[i]);
	}
}

const int kRetryCount = 3;
void Phone_SendReaderPais()
{
	// 发送桌内牌
	if (!phone_send_wait_ack
			|| phone_send_timer.read_ms() > (phone_pai_pos == 4 ? 2000 : 1000))
	{
		for (int i = 0; i < kGroup; i++)
		{
			if (!phone_pais[i].is_send)
			{
				if (phone_pais[i].count > 0)
				{
					if (phone_pais[i].send_count < kRetryCount)
					{
						phone_pai_pos = i + 1;
						phoneBle->SendGroupData(i + 1, phone_pais[i].pais,
								phone_pais[i].count);
						phone_send_timer.reset();
						phone_pais[i].transID = phoneBle->tx_transID();
						phone_pais[i].send_count++;

//                      phone_pai_transid = phone->tx_transID();
						phone_send_wait_ack = true;
//                      phone_pais[i].is_send = true;   //--------
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	// 发送胸口牌
	if (!phone_live_pai.is_wait || phone_live_pai.timer.read_ms() > 1000)
	{
		if (phone_live_pai.count > 0 && !phone_live_pai.is_send
				&& phone_live_pai.send_count < kRetryCount)
		{
			phoneBle->SendAdminReaderData(phone_live_pai.pais,
					phone_live_pai.count);
			phone_live_pai.is_wait = true;
			phone_live_pai.transID = phoneBle->tx_transID();
			phone_live_pai.timer.reset();
			phone_live_pai.send_count++;
		}
	}
}

static uint8_t admin_tile_msg[32];
static HiReader_DataMessage* hireader_msg =
		(HiReader_DataMessage*) admin_tile_msg;
static uint8_t phone_buf[32];
/**
 *
 * @return
 */
int main()
{
	static uint32_t loop = 0;

	// 关闭HSE后，PF0和PF1才能当做普通IO口用。
	__HAL_RCC_HSE_CONFIG(RCC_HSE_OFF);

	PowerOn_Init();

#ifdef KEY_EN
	PowerUp();
#endif

	Powered_Init();

#if USE_WDT == 1
	WDT_Init();
#endif

	UIDEncrypt();

//    HiReader_OpenRF();

//    wait_ms(60);
	ReportDeviceUID();
	wait_ms(30);
	SendInformation();
	wait_ms(30);
	SendCC1101Params();

	unsend_timer.start();
	phone_send_timer.start();
	pai_led_timer.start();
	sys_count++;
	wait_ms(60);
	key->Init();
	key_info->Init();

//  GeneratePai();

	while (1)
	{
#ifdef KEY_EN
		// 检测按键，处理开关机。
		HandleKeyProcess();
#endif

#if USE_WDT == 1
		WDT_Clear();
#endif

		if (key->ScanKey() >= 0)
		{
#if KEY_FUNCTION == 1
			GeneratePai();
#elif KEY_FUNCTION == 2
			Phone_EnterStudyMode(1);
#elif KEY_FUNCTION == 3
			static int ssid_test = 1;
			Phone_ChangeSSID((++ssid_test % 8) + 1);
#elif KEY_FUNCTION == 4
			static int vol = 7;
			if (++vol > 7)
				vol = 0;
			Phone_SetVoiceVolume(vol);
#endif
		}
		if (key_info->ScanKey() >= 0)
		{
			ReportDeviceInfo();
		}

		Speech_Play();

		// 读取读头数据
		switch (Reader_GetMessage(rf_buffer, sizeof(rf_buffer)))
		{
		case kReaderMessageOK:
			Reader_DispatchMessage(rf_buffer);
			Reader_CheckReceived();
			if (TotalCardNum == RP_num)
			{
				Reader_FinishXP();
			}
			break;

		case kReaderMessageOverTime:
			Reader_CheckReceived();
			Reader_CheckOverTime();
			break;

		default:
			break;
		}

        if (phoneBle->IsLink())
        {
            sys_status = kSysIsLink;
            // 胸口读头间歇读取牌数据，读到牌数据就马上传给手机。
                switch (HiReader_GetMessage(hireader_msg))
                {
                case kHiReaderNewPai:
                    HiReader_DispatchMessage(hireader_msg);
                    break;

                default:
                    break;
                }
            HiReader_SendPai();

            Phone_Heartbeat();
            if (phoneBle->GetMessage(phone_buf, sizeof(phone_buf)))
            {
                power_tmr.reset();
                Phone_DispatchMessage(phone_buf);
            }

            if (phoneBle->is_connected())
            {
                Phone_SendReaderPais();
            }
        }
        else
        {
            sys_status = kSysDisLink;
            if (HiReader_IsPowerOn())
            {
                HiReader_PowerOff();
            }
        }
		loop++;

		const int kBits = 12;
		if ((loop & ((1 << kBits) - 1)) == 0)
			DBG("Loop:%x\r\n", loop >> kBits);
		if (*led_X == 1 && pai_led_timer.read_ms() > 100)
		{
			*led_X = 0;
		}
	}
}

