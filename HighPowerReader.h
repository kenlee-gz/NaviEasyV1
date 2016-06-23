/*
 * HighPowerReader.h
 *
 *  Created on: 2016Äê4ÔÂ22ÈÕ
 *      Author: Ken
 */

#ifndef HIGHPOWERREADER_H_
#define HIGHPOWERREADER_H_

#include <stdint.h>
#include "Mahjong.h"

enum HiReader_Command
{
    kHiReaderOpen = 0x01,
    kHiReaderClose = 0x02,
    kHiReaderTileReceived = 0x10,

    kHiReaderOpenAck = kHiReaderOpen | 0x80,
    kHiReaderCloseAck = kHiReaderClose | 0x80,
};

struct HiReader_Message
{
    uint8_t header;
    uint8_t len;
    uint8_t command;
    uint8_t extras[];
};

struct HiReader_TileMessage
{
    uint8_t header;
    uint8_t len;
    uint8_t command;
    MahjongInfoEx tile;
    uint8_t crc;
    uint8_t tail;
};

struct HiReader_AckMessage
{
    uint8_t header;
    uint8_t len;
    uint8_t command;
    uint8_t state;
    uint8_t crc;
    uint8_t tail;
};

struct HiReader_DataMessage
{
    uint8_t len;
    uint8_t command;
    uint8_t extras[];
};

enum HiReaderStatus
{
    kHiReaderNoMessage = 0,
    kHiReaderMessage = 1,
    kHiReaderRepeatPai = 2,
    kHiReaderNewPai = 3,
};

void HiReader_Init();
void HiReader_CloseRF();
void HiReader_OpenRF();
bool HiReader_IsOpen();
void HiReader_ControlRFOnOff();
void HiReader_SetScanMode();
HiReaderStatus HiReader_GetMessage(HiReader_DataMessage* msg);
MahjongInfo GetLastReceivedTile();

void HiReader_ResetChip();
void HiReader_ReleaseReset();
bool HiReader_IsPowerOn();
void HiReader_PowerOn();
void HiReader_PowerOff();

#endif /* HIGHPOWERREADER_H_ */
