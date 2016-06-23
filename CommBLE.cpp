#include	<assert.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

#include	<mbed.h>

#include 	"CommBLE.h"
#include 	"debug.h"
#include 	"FlashApp.h"
#include 	"main.h"
#include 	"Voice.h"

#pragma anon_unions

extern Timer g_timer;

static const int kFrameHead = 0x02;
static const int kFrameTail = 0x03;

static const char* ssids[] =
{ "NAAP1", "NAAP2", "NAAP3", "NAAP4", "NAAP5", "NAAP6", "NAAP7", "NAAP8" };
__weak void WDT_Clear();

CommBLE::CommBLE(BLE* ble) :
        ble_(ble), is_connected_(false), current_(1), state_(), running_cnt_(), key_id_(), color_(), start_id_(), end_id_(), repeat_counter(), some_tiles_unwake(), ssid_(
                1)
{
    tx_transID_ = 0;
    rx_transID_ = 0;
    for (int s = 0; s < 2; s++)
    {
    }
    memset(ref_id_, 0, sizeof(ref_id_));
}

void CommBLE::SendToPhone(const uint8_t *input)
{
    ble_->SendBytes(input, input[1] + 1);
}

/**
 * 计算已打包数据的异或值：第一个字节是0x02，第二个字节是长度
 * @param buf
 * @return
 */
uint8_t CommBLE::CalcCrc(const uint8_t* buf)
{
    return CalcCrc(buf, buf[1] - 1);
}

/**
 * 计算未打包数据的异或值
 * @param buf
 * @param len
 * @return
 */
uint8_t CommBLE::CalcCrc(const uint8_t* buf, int len)
{
    unsigned char crc = 0;
    for (int i = 0; i < len; i++)
    {
        crc ^= *buf++;
    }
    return crc;
}

/**
 * 把数据打包发送给手机，加上包头、包尾、效验字、长度、流水号等
 * @param input
 * @param len
 */
void CommBLE::SendPacket(const void *input, int len)
{
    ble_->ClearCrc();
    ble_->putc(kFrameHead);
    ble_->putc(len + 5);
    ble_->SendBytes(input, len);
    if (*(uint8_t*) input < 0x80)
    {
        tx_transID_++;
        ble_->putc(0xF0);
        ble_->putc(tx_transID_);
    }
    else
    {
        ble_->putc(0xF0);
        ble_->putc(rx_transID_);
    }
    ble_->putc(ble_->GetCrc());
    ble_->putc(kFrameTail);
    tx_phone_tmr.reset();
}

/**
 * 发送心跳包,如：  02 08 77 00 01 F0 02 8E 03
 */
void CommBLE::SendHeartBeat()
{
    static uint16_t heartbeat_cnt = 0;
    unsigned char outbuf[3] =
    { PhoneCmd_HeartBeat, heartbeat_cnt >> 8, heartbeat_cnt & 0xff };

    SendPacket(outbuf, 3);
    heartbeat_cnt++;
    DBG("[%X]heart beat-%X\r\n", g_timer.read_ms(), (int)heartbeat_cnt);
}

void CommBLE::SendTilesReaderData(TileInfo* tile)
{
    ble_->ClearCrc();
    ble_->putc(kFrameHead);
    ble_->putc(sizeof(TileInfo) + 6);
    ble_->putc(/*PhoneCmd_TilesReaderData*/0x44);
    ble_->SendBytes(tile, sizeof(TileInfo));
    ble_->putc(0xf0);
    ble_->putc(++tx_transID_);
    ble_->putc(ble_->GetCrc());
    ble_->putc(kFrameTail);
    tx_phone_tmr.reset();
}

void CommBLE::SendGroupData(int reader, const MahjongInfo* cards, int card_len)
{
    ble_->ClearCrc();
    ble_->putc(kFrameHead);
    ble_->putc(card_len * sizeof(MahjongInfo) + 7);
    ble_->putc(PhoneCmd_TilesReaderData);
    ble_->putc(reader);
//    ble_->putc(card_len);
    ble_->SendBytes(cards, sizeof(MahjongInfo) * card_len);
    ble_->putc(0xf0);
    ble_->putc(++tx_transID_);
    ble_->putc(ble_->GetCrc());
    ble_->putc(kFrameTail);
    tx_phone_tmr.reset();
}

void CommBLE::SendAdminReaderData(const MahjongInfo* cards, int card_len)
{
    ble_->ClearCrc();
    ble_->putc(kFrameHead);
    ble_->putc(card_len * sizeof(MahjongInfo) + 7);
    ble_->putc(PhoneCmd_AdminReaderData);
    ble_->putc(kHiReaderNumber);
//    ble_->putc(card_len);
    ble_->SendBytes(cards, sizeof(MahjongInfo) * card_len);
    ble_->putc(0xf0);
    ble_->putc(++tx_transID_);
    ble_->putc(ble_->GetCrc());
    ble_->putc(kFrameTail);
    tx_phone_tmr.reset();
}

/**
 * 发送胸口读头数据，如：02 0D 20 05 01 8A 3A F8 A7 7F F0 0E 45 03
 * @param cards
 */
void CommBLE::SendAdminReaderData(const MahjongInfo* cards)
{
    int card_len = 1;
    ble_->ClearCrc();
    ble_->putc(kFrameHead);
    ble_->putc(card_len * sizeof(MahjongInfo) + 8);
    ble_->putc(PhoneCmd_AdminReaderData);
    ble_->putc(kHiReaderNumber);
    ble_->putc(card_len);
    ble_->SendBytes(cards, sizeof(MahjongInfo) * card_len);
    ble_->putc(0xf0);
    ble_->putc(++tx_transID_);
    ble_->putc(ble_->GetCrc());
    ble_->putc(kFrameTail);
    tx_phone_tmr.reset();
}

const char* CommBLE::GetSsid()
{
    return ssids[ssid_ - 1];
}

void CommBLE::Init()
{
    wait_ms(200);

    ble_->Init();

    tx_phone_tmr.start();
    rx_phone_tmr.start();
}

bool CommBLE::IsLink()
{
    return ble_->IsLink();
}

bool CommBLE::isValidCommand(uint8_t cmd)
{
//    return (cmd == Ack_HeartBeat || cmd == Cmd_PlayVoice || cmd == Cmd_VerifyHashTable );
    return true;
}

/**
 * 解释串口命令，将命令放置在缓冲区
 * 如: 02 08 F7 00 00 F0 00 0D 03
 */
bool CommBLE::GetMessage(uint8_t* msg, uint8_t len)
{
#if 1
    // 命令格式： 包头0x55    包长度    命令字    流水号    数据(…)    效验字
    const int MIN_FRAME_BYTES = 4;
    const int MAX_FRAME_BYTES = 20;
    enum CommunicateStatus
    {
        None = 0, WaitFrameStart, WaitDataLength, WaitAddress, WaitBody, WaitFrameEnd
    };

    static enum CommunicateStatus commStatus = WaitFrameStart;
    static int lenToReceived = 0;
    static int bufPos;
    bool result = false;
    int receivedData;

    CircularBufferEx& rx_buffer = ble_->GetRxBuffer();

    while (!rx_buffer.empty())
    {
        receivedData = rx_buffer.front();
        rx_buffer.pop_front();

        switch (commStatus)
        {
        case WaitFrameStart:
            if (kFrameHead == receivedData)
            {
                commStatus = WaitDataLength;
            }
            break;

        case WaitDataLength:
            // 检查长度有效性
            if (receivedData >= MIN_FRAME_BYTES && receivedData <= MAX_FRAME_BYTES)
            {
                lenToReceived = receivedData;
                frame_buf_[0] = kFrameHead;
                frame_buf_[1] = receivedData;
                commStatus = WaitAddress;
            }
            else if (kFrameHead != receivedData)
            {
                commStatus = WaitFrameStart;
            }
            break;

        case WaitAddress:
            // 检查命令的有效性
            if (isValidCommand(receivedData))
            {
                frame_buf_[2] = receivedData;
                bufPos = 3;
                commStatus = WaitBody;
            }
            else if (kFrameHead == receivedData)
            {
                commStatus = WaitDataLength;
            }
            else
            {
                commStatus = WaitFrameStart;
            }
            break;

        case WaitBody:
            if (bufPos < lenToReceived)		// 接收数据
            {
                frame_buf_[bufPos++] = receivedData;
            }
            else if (bufPos == lenToReceived)	// 效验和
            {
                if (receivedData == kFrameTail)
                {
                    frame_buf_[bufPos++] = receivedData;
                    uint8_t chk = CalcCrc(frame_buf_, lenToReceived);

                    if (chk == 0)
                    {
                        result = true;
                        memcpy(msg, frame_buf_, lenToReceived + 1);
                        if ((frame_buf_[2] < 0x80) && (frame_buf_[lenToReceived - 3] == 0xf0))
                        {
                            rx_transID_ = frame_buf_[lenToReceived - 2];
                        }
                        commStatus = WaitFrameStart;
                    }
                    else if (kFrameHead == receivedData)
                    {
                        commStatus = WaitDataLength;
                    }
                    else
                    {
                        commStatus = WaitFrameStart;
                    }
                }
                else
                {
                    commStatus = WaitFrameStart;
                }
            }
            break;

        default:
            break;
        }

        if (result == true)
        {
            is_connected_ = true;
            rx_phone_tmr.reset();
            break;
        }
        else if (rx_phone_tmr.read_ms() >= kRxOverTime)
        {
            // 长时间没有数据,可能是手机App关闭了.
            is_connected_ = false;
        }
    }
    return result;
#else
    const int kFrameHead = 0x02;
    const int FRAME_END = 0x03;
    const int MIN_FRAME_BYTES = 4;
    const int MAX_FRAME_BYTES = 0x30;
    enum CommunicateStatus
    {
        None = 0, WaitFrameStart, WaitDataLength, WaitAddress, WaitBody, WaitFrameEnd
    };

    static enum CommunicateStatus commStatus = WaitFrameStart;
    static int lenToReceived = 0;
    static int bufPos;
    bool result = false;
    int receivedData;

    CircularBuffer<uint8_t, 64>& rx_buffer = ble_->GetRxBuffer();

    while (!rx_buffer.empty())
    {
        receivedData = rx_buffer.front();
        rx_buffer.pop_front();

        switch (commStatus)
        {
            case WaitFrameStart:
            if (kFrameHead == receivedData)
            {
                commStatus = WaitDataLength;
            }
            break;

            case WaitDataLength:
            // 检查长度有效性
            if (receivedData >= MIN_FRAME_BYTES && receivedData <= MAX_FRAME_BYTES)
            {
                lenToReceived = receivedData;
                frame_buf_[0] = kFrameHead;
                frame_buf_[1] = receivedData;
                commStatus = WaitAddress;
            }
            else if(kFrameHead != receivedData)
            {
                commStatus = WaitFrameStart;
            }
            break;

            case WaitAddress:
            // 检查命令的有效性
            if (isValidCommand(receivedData))
            {
                frame_buf_[2] = receivedData;
                bufPos = 3;
                commStatus = WaitBody;
            }
            else if(kFrameHead == receivedData)
            {
                commStatus = WaitDataLength;
            }
            else
            {
                commStatus = WaitFrameStart;
            }
            break;

            case WaitBody:
            if (bufPos < lenToReceived - 1)		// 接收数据
            {
                frame_buf_[bufPos++] = receivedData;
            }
            else if (bufPos == lenToReceived - 1)	// 效验和
            {
                uint8_t chk = 0;
                for (int i = 0; i < lenToReceived - 1; i++)
                {
                    chk ^= frame_buf_[i];
                }

                if (chk == receivedData)
                {
                    frame_buf_[bufPos++] = receivedData;
                }
                else if(kFrameHead == receivedData)
                {
                    commStatus = WaitDataLength;
                }
                else
                {
                    commStatus = WaitFrameStart;
                }
            }
            else if (receivedData == FRAME_END)
            {
                frame_buf_[bufPos] = receivedData;
                commStatus = WaitFrameStart;
                result = true;
            }
            else if(kFrameHead == receivedData)
            {
                commStatus = WaitDataLength;
            }
            else
            {
                // 帧尾错误
                commStatus = WaitFrameStart;
            }
            break;

            default:
            break;
        }

        if(result == true) break;
    }
    return result;
#endif
}

int CommBLE::GetNoTxTime()
{
    return tx_phone_tmr.read_ms();
}

