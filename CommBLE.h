/******************************************************************************
 * @file     wifi.h
 * @brief    
 * @version  1.0.0
 * @date     2014��11��13��
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
 * �����ʽ��ͷ      +  ����       +   ����   +   ����   +   ��ˮ��   +   Ч����
 *          0x02  ����Ч��                                                                                       ���
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

	// �ؿڶ�ͷ�Ʒ���
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
	static const int kMsPerTile = 10;	// һ���ƶ�ȡʱ��
	static const int kWakeupInterval = 4;
	static const int kRxOverTime = 20000;
    static const int kHiReaderNumber = 5;

	BLE* ble_;

	bool is_connected_;		// ���ֻ�����ָʾ����ʱû��ô��
	uint8_t tx_transID_;
	int rx_transID_;

	int current_;		// ���ڴ������id

	int state_;				// ״̬�֡�����ÿ��ָ���е�״̬��¼��
	ConnectState wifi_state_;        // WIFI����״̬

	union
	{
	int running_cnt_;		// ����:��һ�η��ͻ���ʱ�䡣����:�ֶ�����ѭ��������
							// ����:��ɨң�ض�ʱ
	int next_wake_time;
	};

	uint8_t frame_buf_[32];	// ���յ����ֻ�ָ���
	uint8_t tx_buf_[40];	// ��Ҫ���͵��ֻ�������
	char gateway_[16];
	// ������1��ʼ��Ԥ��һ������0�Ŀռ䣬������չ������ʼ�����

	Timer rx_phone_tmr;	// ���յ��ֻ����ͼ�ʱ��
	Timer tx_phone_tmr;	// ���͵��ֻ���ʱ��
	Timer rf_tmr;		// ��rfͨѶ��ʱ�������ڻ��Ѽ��㲥����ʱ������7��Ͳ�ӻ�����

	// ����2.4GоƬ���ջ���

    // ����
    int key_id_;
    // ��ǰ��ɫ
	int color_;
	// ��һ����Ч��Idֵ
	int start_id_;
	// ���һ����Ч��Idֵ
	int end_id_;

	// �ź����õ�����ͨѶ����
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
