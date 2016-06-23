/******************************************************************************
 * @file     	BLE.h
 * @brief    
 * @version  	1.0.0
 * @date     	2016Äê6ÔÂ15ÈÕ
 *
 * @note
 * Copyright (C) 2016 YUWEI Corp. All rights reserved.
 ******************************************************************************/

#ifndef BLE_H_
#define BLE_H_

#include <stdint.h>
#include "CircularBufferEx.hpp"
#include "mbed.h"
#include "BufferedSerial.h"

#pragma once

#include <stdint.h>
#include "CircularBufferEx.hpp"
#include "mbed.h"
#include "BufferedSerial.h"

class BLE
{
public:
    enum WifiMode
    {
        WifiModeAP = 0, WifiModeSTA = 1, WifiModeAPSTA = 2, WifiModeUnknow = 3,
    };

enum WifiATCmd
{
  kWifiWANN,
  kWifiSsid,
  kWifiTcpLink,
  kWifiEcho,
  kWifiTransparentMode,
  kWifiCommandMode,
  kWifiReboot,
  kWifiWorkMode,
  kWifiGetVersion,
};

private:
    BufferedSerial* uart_wifi;
    CircularBufferEx& rx_buffer;
    DigitalIn io_link;
    DigitalIn io_ready;
    bool is_command_mode_;
    uint8_t crc;

    int ReadLine(uint8_t* line, uint8_t maxcnt, int timeout);
    void SerialHandler();

public:
    BLE(BufferedSerial* uart, CircularBufferEx& buf, PinName link, PinName rdy);

    CircularBufferEx& GetRxBuffer(){return rx_buffer;}

    bool Baudrate(int baud);
    bool Rename(char* newname);

    bool GetLinkStatus();

    int putc(int c);

    bool SetEcho(bool enable);

    bool SetSsid(const char* ssid);
    bool GetSsid(char* ssid);

    int GetTcpto();

    bool SetWsKey();
    void AsyncCmd(WifiATCmd cmd, const void* arg = NULL);

    int GetGateway(char* mode, char* addr, char* mask, char* gateway);
    int AsyncGetGateway(char* mode, char* addr, char* mask, char* gateway);
    int scanf(const char* format, ...);
    int scanf(int overtime, const char* format, ...);
    bool find(const char* str);
    bool find(int overtime, const char* str);
    bool GetSetting(const char* str, char* buf, int overtime);

    void ClearCrc(){crc = 0;}
    uint8_t GetCrc(){return crc;}

    void Init();
    void Reset();
    bool Reboot();
    bool IsLink();
    void SendBytes(const void* bytes, uint8_t len);

    bool SendCmdAndWaitReply(const char* cmd, int overtime = 300, const char* params = NULL);
};


#endif /* BLE_H_ */
