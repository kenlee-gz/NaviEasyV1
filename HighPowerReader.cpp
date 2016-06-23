/*
 * HighPowerReader.cpp
 *
 *  Created on: 2016年4月22日
 *      Author: Ken
 *      数据：
 *      02 3B 89 90 7C 53 00 01 04 E0 3B 00 00 00 00 00 03 02 3B 89 90 7C 53 00 01 04 E0 3B 00 00 00 00 00 03
 */
#include "HighPowerReader.h"
#include "mbed.h"
#include "BufferedSerial.h"
#include "CircularBufferA.h"

#define     RESET_ON    1
#define     RESET_OFF   0

extern DigitalOut* rf_power;
extern DigitalOut* reader_reset;

static const int kHeaderFlag = 0x02;
static const int kTailFlag = 0x03;
static const int com_adr = 2;
extern BufferedSerial* reader_uart;
CircularBufferA<uint8_t> rd_buffer(128);
static Timer reader_rx_tmr;     //!<读头接收数据定时器,如一定时间没有收到串口数据,重置帧头
static bool is_open = false;
extern Timer g_timer;

static const int kTileBufferSize = 4;

static int rd_message_pos = 0;
static uint8_t tile_msg[32];
static const int kHiReaderDummyLength = 5;
static HiReader_Message* hireader_msg = (HiReader_Message*) tile_msg;

static Timer reader_onoff_timer;        //!<控制开关开关占空比的定时器
//static bool hireader_en = false;

struct RxTileInfo
{
    MahjongInfo mahjong;
    uint32_t rx_time;
};

static RxTileInfo latest_info;
static CircularBufferA<RxTileInfo> tile_backups(10);

enum HiReaderStates
{
    kHiReaderPowerOff,
    kHiReaderPowerOnWithChipReset,
    kHiReaderPowerOnNoReset,
    kHiReaderRFOff,
    kHiReaderRFOn,
};
static HiReaderStates hireader_state = kHiReaderPowerOff;

void HiReader_SerialHandler()
{
    rd_buffer.push(static_cast<uint8_t>(reader_uart->getc()));
}

void HiReader_Init()
{
    *rf_power = 0;  // 关闭电源
    *reader_reset = 0;

//    reader_uart = new BufferedSerial(PA_9, PA_10);
//    reader_uart->baud(115200);
//    reader_uart->printf("Reader Serial\r\n");
//    reader_uart->attach(&HiReader_SerialHandler);
    reader_rx_tmr.start();
    reader_onoff_timer.start();

//    HiReader_CloseRF();
//    HiReader_OpenRF();
}

static bool IsValidHeader(uint8_t ch)
{
    return (kHeaderFlag == ch);
}

static bool IsValidLength(uint8_t ch)
{
    return (ch >= 5 && ch <= 0x0e);
}

static bool IsValidCommand(uint8_t ch)
{
    return (ch == 0x81 || ch == 0x82 || ch == 0x10);
}

bool IsDataPacket(HiReader_Message* msg)
{
    return (msg->command == kHiReaderTileReceived);
}

bool IsNewReaderData(RxTileInfo* msg)
{
    const int kRepeatInterval = 1000;
    bool result = true;
    for (int i = tile_backups.size() - 1; i >= 0; i--)
    {
        if (msg->rx_time - tile_backups[i].rx_time < kRepeatInterval)
        {
            if (memcmp(&tile_backups[i].mahjong, &msg->mahjong, sizeof(MahjongInfo)) == 0)
            {
                result = false;
                break;
            }
        }
        else
        {
            break;
        }
    }
    return result;
}

static int CalcXorCrc(const uint8_t* buf, uint8_t len)
{
    int result = 0;
    for (int i = 0; i < len; i++)
    {
        result ^= buf[i];
    }
    return result;
}

void HiReader_ResetChip()
{
    *reader_reset = RESET_ON;
}

void HiReader_ReleaseReset()
{
    *reader_reset = RESET_OFF;
}

bool HiReader_IsPowerOn()
{
    return *rf_power;
}

void HiReader_PowerOn()
{
    *rf_power = 1;
    HiReader_ResetChip();
    if (reader_uart == NULL)
    {
        reader_uart = new BufferedSerial(PA_9, PA_10);
        reader_uart->baud(115200);
        reader_uart->attach(&HiReader_SerialHandler);
    }
}

void HiReader_PowerOff()
{
    *rf_power = 0;
    *reader_reset = 0;
    if(reader_uart)
    {
        reader_uart->attach(NULL);
        delete reader_uart;
        reader_uart = NULL;

        DigitalIn PA9(PA_9);
        DigitalIn PA10(PA_10);
        PA9.mode(PullNone);
        PA10.mode(PullNone);
    }
}

/**
 * 输入格式为 HiReader_DataMessage
 * @param buf
 */
static void HiReader_SendPacket(const HiReader_DataMessage* buf)
{
    reader_uart->putc(kHeaderFlag);
    for (int i = 0; i < buf->len - 3; i++)
    {
        reader_uart->putc(*((uint8_t*) buf + i));
    }
    int crc = kHeaderFlag ^ CalcXorCrc((uint8_t*) buf, buf->len - 3);
    reader_uart->putc(crc);
    reader_uart->putc(kTailFlag);
}

#if 0
/**
 * 为省电，RF不能一直打开，需要间歇开关读取牌数据。
 */
void HiReader_ControlRFOnOff()
{
    const int kRFOffTime = 150;
    const int kRFOnTime = 30;
    if (hireader_en)
    {
        if (reader_onoff_timer.read_ms() > kRFOnTime)
        {
            HiReader_CloseRF();
            hireader_en = false;
            reader_onoff_timer.reset();
        }
    }
    else
    {
        if (reader_onoff_timer.read_ms() > kRFOffTime)
        {
            HiReader_OpenRF();
            hireader_en = true;
            reader_onoff_timer.reset();
        }
    }
}
#endif

/**
 * 接收到类似：02 0E 10 8A 3A F8 A7 7F 00 01 04 E0 69 03
 * @param msg
 * @return
 */
HiReaderStatus HiReader_GetMessage(HiReader_DataMessage* msg)
{
    static int rd_status = 0;
    HiReaderStatus result = kHiReaderNoMessage;
    uint8_t ch;
    uint8_t crc;
    const int kHiReaderPowerOnTime = 150;
    const int kHiReaderResetTime = 20;
    const int kRFOffTime = 150;
    const int kRFOnTime = 50;

    if(*rf_power == 0)
    {
        hireader_state = kHiReaderPowerOff;
    }

    switch(hireader_state)
    {
    case kHiReaderPowerOff:
        HiReader_PowerOn();
        reader_onoff_timer.reset();
        hireader_state = kHiReaderPowerOnWithChipReset;
        break;

    case kHiReaderPowerOnWithChipReset:
        if(reader_onoff_timer.read_ms() > kHiReaderPowerOnTime)
        {
            reader_onoff_timer.reset();
            HiReader_ReleaseReset();
            hireader_state = kHiReaderPowerOnNoReset;
        }
        break;

    case kHiReaderPowerOnNoReset:
        if(reader_onoff_timer.read_ms() > kHiReaderResetTime)
        {
            reader_onoff_timer.reset();
            hireader_state = kHiReaderRFOff;
        }
        break;

    case kHiReaderRFOff:
        if (reader_onoff_timer.read_ms() > kRFOffTime)
        {
            HiReader_OpenRF();
            hireader_state = kHiReaderRFOn;
            reader_onoff_timer.reset();
        }
        break;

    case kHiReaderRFOn:
        if (reader_onoff_timer.read_ms() > kRFOnTime)
        {
            HiReader_CloseRF();
            hireader_state = kHiReaderRFOff;
            reader_onoff_timer.reset();
        }
        break;

    default:
        break;
    }

    while (rd_buffer.pop(ch))
    {
        if (reader_rx_tmr.read_ms() > 100)
        {
            rd_status = 0;
        }

        reader_rx_tmr.reset();
        switch (rd_status)
        {
        case 0:     // 帧头检测
            if (IsValidHeader(ch))
            {
                rd_status = 1;
                rd_message_pos = 0;
                hireader_msg->header = kHeaderFlag;
            }
            break;

        case 1:     // 长度检测
            if (IsValidLength(ch))
            {
                rd_status = 2;
                hireader_msg->len = ch;
            }
            else if (!IsValidHeader(ch))
            {
                rd_status = 0;
            }
            break;

        case 2:     // 命令字检测
            if (IsValidCommand(ch))
            {
                hireader_msg->command = ch;
                if (hireader_msg->len > kHiReaderDummyLength)
                {
                    rd_status = 3;
                    rd_message_pos = 0;
                }
                else
                {
                    rd_status = 4;
                }
            }
            else
            {
                rd_status = 0;
            }
            break;

        case 3:     // 附加数据接收
            hireader_msg->extras[rd_message_pos++] = ch;
            if (rd_message_pos + kHiReaderDummyLength >= hireader_msg->len)
            {
                rd_status = 4;
            }
            break;

        case 4:     // 效验字检测
            if (CalcXorCrc((const uint8_t*) hireader_msg, hireader_msg->len - 2) == ch)
            {
                rd_status = 5;
            }
            else
            {
                rd_status = 0;
            }
            break;

        case 5:
            if (kTailFlag == ch)
            {
                // 检查短时间内是否收到重复的数据，如有重复，则本次数据滤除
                if (IsDataPacket(hireader_msg))
                {
                    latest_info.rx_time = g_timer.read_ms();
                    memcpy(&latest_info.mahjong, &hireader_msg->extras[0], sizeof(MahjongInfo));
                    memcpy(msg, &hireader_msg->len, sizeof(MahjongInfo) + 2);
                    if (IsNewReaderData(&latest_info))
                    {
                        result = kHiReaderNewPai;
                    }
                    else
                    {
                        result = kHiReaderRepeatPai;
                    }
                    tile_backups.push(latest_info);

                    if (hireader_state == kHiReaderRFOn)
                    {
                        reader_onoff_timer.reset();
                    }
                }
                else
                {
                    result = kHiReaderMessage;
                }
            }
            rd_status = 0;
            break;

        default:
            break;
        }
    }
    return result;
}

MahjongInfo GetLastReceivedTile()
{
    return latest_info.mahjong;
}

#if 0
static int CalcISO15693Crc(const uint8_t* buf, uint8_t len)
{
    const int kPolyNomial = 0x8408;     // Crc多项式
    const int kPresetValue = 0xffff;

    int result = kPresetValue;
    for (int i = 0; i < len; i++)
    {
        result ^= buf[i];
        for (int j = 0; j < 8; j++)
        {
            if (result & 0x0001)
            {
                result = (result >> 1) ^ kPolyNomial;
            }
            else
            {
                result >>= 1;
            }
        }
    }
    return result;
}
#endif

/**
 * 关读头：02 05 02 05 03
 * 回应：     02 06 82 00 86 03
 */
void HiReader_CloseRF()
{
    uint8_t buf[] =
    { 0x05, 0x02 };
    HiReader_SendPacket((const HiReader_DataMessage*) buf);
    is_open = false;
}

/**
 * 开读头：02 05 01 06 03
 * 回应：     02 06 81 00 85 03
 */
void HiReader_OpenRF()
{
    uint8_t buf[] =
    { 0x05, 0x01 };

    HiReader_SendPacket((const HiReader_DataMessage*) buf);
    is_open = true;
}

bool HiReader_IsOpen()
{
    return is_open;
}
