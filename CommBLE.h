/******************************************************************************
 * @file     wifi.h
 * @brief    
 * @version  1.0.0
 * @date     2014年11月13日
 *
 * @note
 * Copyright (C) 2014 YUWEI Corp. All rights reserved.
 ******************************************************************************/
#pragma once

#include <stdint.h>

#include "BLE.h"
#include "Timer.h"
#include "WT588D.h"
#include "Mahjong.h"

typedef unsigned char Color;

enum Eeprom_Operator
{
    kEepromRead = 0,
    kEepromWrite = 1,
};

struct ReferencePai
{
    uint8_t id;
    uint8_t volt;
    uint8_t rssi;
    uint8_t status;
};

/**
 * 命令格式：头      +  长度       +   命令   +   数据   +   流水号   +   效验码
 *          0x02  不含效验                                                                                       异或
 */
struct PhoneMessage
{
    uint8_t head;
    uint8_t len;
    uint8_t cmd;
    uint8_t extras[];
};

struct PhoneMessage_SetPaiCount
{
    uint8_t head;
    uint8_t len;
    uint8_t cmd;
    uint8_t player_cnt;
    uint8_t east_cnt;
    uint8_t north_cnt;
    uint8_t west_cnt;
    uint8_t south_cnt;
};

enum ConnectState
{
    kPhoneNoLink,
    kPhoneConnected,
    kPhoneConnecting,

    kPhonePowerOn,

};

struct PhoneMessage_AckPais
{
    uint8_t head;
    uint8_t len;
    uint8_t cmd;
    uint8_t ack;
    uint8_t extra;
    uint8_t transIdFlag;
    uint8_t transId;
    uint8_t crc;
    uint8_t tail;
};

struct TileInfo
{
    uint8_t reader;
    uint8_t pos;
    MahjongInfo tile;
};

enum PhoneCommand
{
    PhoneCmd_VerifyHashTable = 0x04,
    PhoneCmd_QueryReadersRSSI = 0x0a,
    PhoneCmd_ReadersSelfTest = 0x0b,
    PhoneCmd_VoicePlay = 0x0e,
    PhoneCmd_HeartBeat = 0x77,

    PhoneCmd_Status = 0,
    PhoneCmd_AdminReaderData = 0x20,
    PhoneCmd_TilesReaderData = 0x21,
    PhoneCmd_SetTilesCount = 0x25,
    PhoneCmd_ChangeSSID = 0x26,
    PhoneCmd_SetVolume = 0x27,
    PhoneCmd_EnterStudyMode = 0x35,

    PhoneCmd_StatusAck = PhoneCmd_Status |0x80,
    PhoneCmd_VerifyHashTableAck = PhoneCmd_VerifyHashTable | 0x80,
    PhoneCmd_AdminReaderDataAck = PhoneCmd_AdminReaderData | 0x80,
    PhoneCmd_TilesReaderDataAck = PhoneCmd_TilesReaderData | 0x80,
    PhoneCmd_SetTilesCountAck = PhoneCmd_SetTilesCount | 0x80,
    PhoneCmd_ChangeSSIDAck = 0x80 | PhoneCmd_ChangeSSID,
    PhoneCmd_SetVolumeAck = 0x80 | PhoneCmd_SetVolume,
    PhoneCmd_HeartBeatAck = 0x80 | PhoneCmd_HeartBeat,
    PhoneCmd_EnterStudyModeAck = 0x80 | PhoneCmd_EnterStudyMode,
};


static const int MAX_TILES_CNT = 72;

class CommBLE
{
public:
    CommBLE(BLE* ble);
	void Init();
	bool GetMessage(uint8_t* msg, uint8_t len);
	bool is_connected(){return is_connected_;}

	// 胸口读头牌发送
	void SendGroupData(int reader, const MahjongInfo* cards, int len);
	void SendAdminReaderData(const MahjongInfo* cards, int len);
	void SendAdminReaderData(const MahjongInfo* cards);
	void SendTilesReaderData(TileInfo* tile);

	void SendPacket(const void *input, int len);

	bool IsLink();

	int GetNoTxTime();
    void SendHeartBeat();

    uint8_t tx_transID() {return tx_transID_;}
    uint8_t rx_transID() {return rx_transID_;}
    int ssid() {return ssid_;}
    void SetSsid(int ssid) {ssid_ = ssid;}

private:
	static const int kMsPerTile = 10;	// 一张牌读取时间
	static const int kWakeupInterval = 4;
	static const int kRxOverTime = 20000;
    static const int kHiReaderNumber = 5;

	BLE* ble_;

	bool is_connected_;		// 与手机连接指示。暂时没怎么用
	uint8_t tx_transID_;
	int rx_transID_;

	int current_;		// 正在处理的牌id

	int state_;				// 状态字。用于每条指令中的状态记录。
	ConnectState wifi_state_;        // WIFI连接状态

	union
	{
	int running_cnt_;		// 唤醒:下一次发送唤醒时间。读牌:手动读牌循环次数。
							// 空闲:做扫遥控定时
	int next_wake_time;
	};

	uint8_t frame_buf_[32];	// 接收到的手机指令缓冲
	uint8_t tx_buf_[40];	// 需要发送到手机的数据
	char gateway_[16];
	// 索引从1开始，预留一个索引0的空间，用于扩展或做初始化标记

	Timer rx_phone_tmr;	// 接收到手机发送计时器
	Timer tx_phone_tmr;	// 发送到手机计时器
	Timer rf_tmr;		// 与rf通讯计时器，用于唤醒及广播包计时，超过7秒筒子会休眠

	// 两个2.4G芯片接收缓冲

    // 按键
    int key_id_;
    // 当前颜色
	int color_;
	// 第一个有效的Id值
	int start_id_;
	// 最后一个有效的Id值
	int end_id_;

	// 信号良好的用于通讯的牌
	ReferencePai ref_id_[2];
	ReferencePai next_ref_id_[2];

	int repeat_counter;
	bool some_tiles_unwake;

	const char* GetSsid();
	uint8_t CalcCrc(const uint8_t* buf);
	uint8_t CalcCrc(const uint8_t* buf, int len);
	bool isValidCommand(uint8_t cmd);

	static const int kNoPlaying = 0xff;
	uint8_t voice_chains_[20];
	int ssid_;

protected:
	void SendToPhone(const uint8_t *input);
};
