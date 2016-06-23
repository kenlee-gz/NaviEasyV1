/******************************************************************************
 * @file     	BLE.cpp
 * @brief    
 * @version  	1.0.0
 * @date     	2016年6月15日
 *
 * @note
 * Copyright (C) 2016 YUWEI Corp. All rights reserved.
 ******************************************************************************/
#include "BLE.h"

#include    <assert.h>
#include    <stdint.h>
#include    <stdio.h>
#include    <string.h>
#include    <stdarg.h>

#include    "BLE.h"

static const char* ble_names[] = {"NAAP1", "NAAP2", "NAAP3", "NAAP4", "NAAP5", "NAAP6", "NAAP7", "NAAP8"};

BLE::BLE(BufferedSerial* uart, CircularBufferEx& buf, PinName link, PinName rdy) :
        uart_wifi(uart), rx_buffer(buf), io_link(link), io_ready(rdy), is_command_mode_(), crc()
{
    uart_wifi->attach(this, &BLE::SerialHandler);
}

int BLE::putc(int c)
{
    crc ^= c;
    return uart_wifi->putc(c);
}

bool BLE::find(const char* str)
{
    bool is_success = false;
    int i;

    int len_str = strlen(str);

    while (rx_buffer.size() >= len_str)
    {
        for (i = 0; i < len_str; i++)
        {
            if (rx_buffer[i] != str[i])
            {
                break;
            }
        }

        if (i == len_str)
        {
            is_success = true;
            rx_buffer.erase_begin(len_str);
            break;
        }
        else
        {
            rx_buffer.pop_front();
        }
    }
    return is_success;
}

bool BLE::find(int overtime, const char* str)
{
    Timer tmr;
    bool is_success = false;
    int i;

    int len_str = strlen(str);

    // 读取一行数据
    tmr.start();
    while (tmr.read_ms() < overtime)
    {
        int len = rx_buffer.size();
        if (len >= len_str)
        {
            for (i = 0; i < len_str; i++)
            {
                if (rx_buffer[i] != str[i])
                {
                    break;
                }
            }
            if (i == len_str)
            {
                is_success = true;
                rx_buffer.erase_begin(len_str);
                break;
            }
            rx_buffer.pop_front();
        }
    }
    return is_success;
}

int BLE::scanf(const char* format, ...)
{
    int tmp;
    int result = -1;
    char line_buf[64];
    char* s = line_buf;

// 读取一行数据
    int len = rx_buffer.size();
    for (int i = 0; i < len; i++)
    {
        if (rx_buffer[i] == '\n' || rx_buffer[i] == '\r' || rx_buffer[i] == 0)
        {
            // 跳过开头的回车换行及0字符
            if (i == 0)
            {
                rx_buffer.pop_front();
                break;
            }

            for (int idx = 0; idx < i; idx++)
            {
                tmp = rx_buffer.front();
                if (tmp != '\r')
                {
                    *s = tmp;
                    s++;
                }
                rx_buffer.pop_front();
            }

            // 弹出回车符，加上字符串结束符
            rx_buffer.pop_front();
            *s = 0;

            std::va_list arg;
            va_start(arg, format);
            result = vsscanf(line_buf, format, arg);
            va_end(arg);
            return result;
        }
    }

    return result;
}

/**
 * 针对AT数据做scanf处理
 * @param overtime  超时时间
 * @param format
 * @return
 */
int BLE::scanf(int overtime, const char* format, ...)
{
    int tmp;
    Timer tmr;
    int result = -1;
    char line_buf[64];
    char* s = line_buf;

// 读取一行数据
    tmr.start();
    while (tmr.read_ms() < overtime)
    {
        int len = rx_buffer.size();
        for (int i = 0; i < len; i++)
        {
            if (rx_buffer[i] == '\n' || rx_buffer[i] == '\r' || rx_buffer[i] == 0)
            {
                // 跳过开头的回车换行及0字符
                if (i == 0)
                {
                    rx_buffer.pop_front();
                    break;
                }

                for (int idx = 0; idx < i; idx++)
                {
                    tmp = rx_buffer.front();
                    if (tmp != '\r')
                    {
                        *s = tmp;
                        s++;
                    }
                    rx_buffer.pop_front();
                }

                // 弹出回车符，加上字符串结束符
                rx_buffer.pop_front();
                *s = 0;

                std::va_list arg;
                va_start(arg, format);
                result = vsscanf(line_buf, format, arg);
                va_end(arg);
                return result;
            }
        }
    }

    return result;
}

void BLE::SerialHandler()
{
    rx_buffer.push_back(static_cast<uint8_t>(uart_wifi->getc()));
}

bool BLE::Reboot()
{
    uart_wifi->puts("TTM:RST-SYSTEMRESET");
    return true;
}

bool BLE::IsLink()
{
    return io_link == 0;
}

int BLE::ReadLine(uint8_t* line, uint8_t maxcnt, int timeout)
{
    int cnt = 0;
    int c;
    Timer tmr;

    tmr.start();
    while (tmr.read_ms() < timeout)
    {
        if (uart_wifi->readable())
        {
            c = uart_wifi->getc();
            if (c == 0x0d || c == 0x0a || c == 0)
            {
                if (cnt > 0)
                {
                    break;
                }
            }
            else
            {
                *(line + cnt) = c;
                if(++cnt >= maxcnt) break;
            }
        }
    }

    *(line + cnt) = 0;
    return cnt;
}


bool BLE::SetSsid(const char* ssid)
{
    rx_buffer.clear();
    uart_wifi->printf("TTM:REN-%s\r\n", ssid);
    return find(300, "TTM:OK");
}


void BLE::SendBytes(const void* bytes, uint8_t len)
{
    for (int i = 0; i < len; i++)
    {
        crc ^= *((uint8_t*) bytes + i);
        uart_wifi->putc(*((uint8_t*) bytes + i));
    }
}

void BLE::Init()
{
    Reset();
}

/**
 * 复位WIFI
 */
void BLE::Reset()
{
}

bool BLE::Baudrate(int baud)
{
    uart_wifi->printf("TTM:BPS-%d", baud);
    return find(300, "TTM:OK");
}
