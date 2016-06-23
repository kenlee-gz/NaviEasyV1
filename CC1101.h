/**
 * @author Athanassios Mavrogeorgiadis
 * @author TI CC1101 library  developed by Athanassios Mavrogeorgiadis (tmav Electronics) as template based on TI C8051 SOURCE CODE swrc021f
 * @section LICENSE
 *
 * Copyright (c) 2010 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * CC1101 Low-Power Sub-1 GHz RF Transceiver CC1101.
 *
 * Datasheet:
 *
 * http://focus.ti.com/lit/ds/swrs061f/swrs061f.pdf
 */

#ifndef MBED_CC1101_H
#define MBED_CC1101_H

/**
 * Includes
 */
#include "mbed.h"
#include "SimpleIn.h"

/**
 * Defines
 */
///////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------------
// CC2500/CC1100 STROBE, CONTROL AND STATUS REGSITER
#define CCxxx0_IOCFG2       0x00        // GDO2 output pin configuration
#define CCxxx0_IOCFG1       0x01        // GDO1 output pin configuration
#define CCxxx0_FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define CCxxx0_SYNC1        0x04        // Sync word, high byte
#define CCxxx0_SYNC0        0x05        // Sync word, low byte
#define CCxxx0_IOCFG0       0x02        // GDO0 output pin configuration
#define CCxxx0_FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define CCxxx0_SYNC1        0x04        // Sync word, high byte
#define CCxxx0_SYNC0        0x05        // Sync word, low byte
#define CCxxx0_PKTLEN       0x06        // Packet length
#define CCxxx0_PKTCTRL1     0x07        // Packet automation control
#define CCxxx0_PKTCTRL0     0x08        // Packet automation control
#define CCxxx0_ADDR         0x09        // Device address
#define CCxxx0_CHANNR       0x0A        // Channel number
#define CCxxx0_FSCTRL1      0x0B        // Frequency synthesizer control
#define CCxxx0_FSCTRL0      0x0C        // Frequency synthesizer control
#define CCxxx0_FREQ2        0x0D        // Frequency control word, high byte
#define CCxxx0_FREQ1        0x0E        // Frequency control word, middle byte
#define CCxxx0_FREQ0        0x0F        // Frequency control word, low byte
#define CCxxx0_MDMCFG4      0x10        // Modem configuration
#define CCxxx0_MDMCFG3      0x11        // Modem configuration
#define CCxxx0_MDMCFG2      0x12        // Modem configuration
#define CCxxx0_MDMCFG1      0x13        // Modem configuration
#define CCxxx0_MDMCFG0      0x14        // Modem configuration
#define CCxxx0_DEVIATN      0x15        // Modem deviation setting
#define CCxxx0_MCSM2        0x16        // Main Radio Control State Machine configuration
#define CCxxx0_MCSM1        0x17        // Main Radio Control State Machine configuration
#define CCxxx0_MCSM0        0x18        // Main Radio Control State Machine configuration
#define CCxxx0_FOCCFG       0x19        // Frequency Offset Compensation configuration
#define CCxxx0_BSCFG        0x1A        // Bit Synchronization configuration
#define CCxxx0_AGCCTRL2     0x1B        // AGC control
#define CCxxx0_AGCCTRL1     0x1C        // AGC control
#define CCxxx0_AGCCTRL0     0x1D        // AGC control
#define CCxxx0_WOREVT1      0x1E        // High byte Event 0 timeout
#define CCxxx0_WOREVT0      0x1F        // Low byte Event 0 timeout
#define CCxxx0_WORCTRL      0x20        // Wake On Radio control
#define CCxxx0_FREND1       0x21        // Front end RX configuration
#define CCxxx0_FREND0       0x22        // Front end TX configuration
#define CCxxx0_FSCAL3       0x23        // Frequency synthesizer calibration
#define CCxxx0_FSCAL2       0x24        // Frequency synthesizer calibration
#define CCxxx0_FSCAL1       0x25        // Frequency synthesizer calibration
#define CCxxx0_FSCAL0       0x26        // Frequency synthesizer calibration
#define CCxxx0_RCCTRL1      0x27        // RC oscillator configuration
#define CCxxx0_RCCTRL0      0x28        // RC oscillator configuration
#define CCxxx0_FSTEST       0x29        // Frequency synthesizer calibration control
#define CCxxx0_PTEST        0x2A        // Production test
#define CCxxx0_AGCTEST      0x2B        // AGC test
#define CCxxx0_TEST2        0x2C        // Various test settings
#define CCxxx0_TEST1        0x2D        // Various test settings
#define CCxxx0_TEST0        0x2E        // Various test settings

// Strobe commands
#define CCxxx0_SRES         0x30        // Reset chip.
#define CCxxx0_SFSTXON      0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
// If in RX/TX: Go to a wait state where only the synthesizer is
// running (for quick RX / TX turnaround).
#define CCxxx0_SXOFF        0x32        // Turn off crystal oscillator.
#define CCxxx0_SCAL         0x33        // Calibrate frequency synthesizer and turn it off
// (enables quick start).
#define CCxxx0_SRX          0x34        // Enable RX. Perform calibration first if coming from IDLE and
// MCSM0.FS_AUTOCAL=1.
#define CCxxx0_STX          0x35        // In IDLE state: Enable TX. Perform calibration first if
// MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
// Only go to TX if channel is clear.
#define CCxxx0_SIDLE        0x36        // Exit RX / TX, turn off frequency synthesizer and exit
// Wake-On-Radio mode if applicable.
#define CCxxx0_SAFC         0x37        // Perform AFC adjustment of the frequency synthesizer
#define CCxxx0_SWOR         0x38        // Start automatic RX polling sequence (Wake-on-Radio)
#define CCxxx0_SPWD         0x39        // Enter power down mode when CSn goes high.
#define CCxxx0_SFRX         0x3A        // Flush the RX FIFO buffer.
#define CCxxx0_SFTX         0x3B        // Flush the TX FIFO buffer.
#define CCxxx0_SWORRST      0x3C        // Reset real time clock.
#define CCxxx0_SNOP         0x3D        // No operation. May be used to pad strobe commands to two
// bytes for simpler software.

#define CCxxx0_PARTNUM          0x30
#define CCxxx0_VERSION          0x31
#define CCxxx0_FREQEST          0x32
#define CCxxx0_LQI              0x33
#define CCxxx0_RSSI             0x34
#define CCxxx0_MARCSTATE        0x35
#define CCxxx0_WORTIME1         0x36
#define CCxxx0_WORTIME0         0x37
#define CCxxx0_PKTSTATUS        0x38
#define CCxxx0_VCO_VC_DAC       0x39
#define CCxxx0_TXBYTES          0x3A
#define CCxxx0_RXBYTES          0x3B
#define CCxxx0_RCCTRL1_STATUS   0x3C
#define CCxxx0_RCCTRL0_STATUS   0x3D

#define CCxxx0_PATABLE          0x3E
#define CCxxx0_TXFIFO           0x3F
#define CCxxx0_RXFIFO           0x3F
///////////////////////////////////////////////////////////////////////////////////////
// RF_SETTINGS is a data structure which contains all relevant CCxxx0 registers
typedef struct S_RF_SETTINGS {
    unsigned char IOCFG2;    // GDO2 output pin configuration
    unsigned char IOCFG1;
    unsigned char IOCFG0;    // GDO0 output pin configuration
    unsigned char FIFOTHR;   // RXFIFO and TXFIFO thresholds.
    unsigned char SYNC1;
    unsigned char SYNC0;
    unsigned char PKTLEN;    // Packet length.
    unsigned char PKTCTRL1;  // Packet automation control.
    unsigned char PKTCTRL0;  // Packet automation control.
    unsigned char ADDR;      // Device address.
    unsigned char CHANNR;    // Channel number.
    unsigned char FSCTRL1;   // Frequency synthesizer control.
    unsigned char FSCTRL0;   // Frequency synthesizer control.
    unsigned char FREQ2;     // Frequency control word, high byte.
    unsigned char FREQ1;     // Frequency control word, middle byte.
    unsigned char FREQ0;     // Frequency control word, low byte.
    unsigned char MDMCFG4;   // Modem configuration.
    unsigned char MDMCFG3;   // Modem configuration.
    unsigned char MDMCFG2;   // Modem configuration.
    unsigned char MDMCFG1;   // Modem configuration.
    unsigned char MDMCFG0;   // Modem configuration.
    unsigned char DEVIATN;   // Modem deviation setting (when FSK modulation is enabled).
    unsigned char MCSM2;
    unsigned char MCSM1;     // Main Radio Control State Machine configuration.
    unsigned char MCSM0;     // Main Radio Control State Machine configuration.
    unsigned char FOCCFG;    // Frequency Offset Compensation Configuration.
    unsigned char BSCFG;     // Bit synchronization Configuration.
    unsigned char AGCCTRL2;  // AGC control.
    unsigned char AGCCTRL1;  // AGC control.
    unsigned char AGCCTRL0;  // AGC control.
    unsigned char WOREVT1;
    unsigned char WOREVT0;
    unsigned char WORCTRL;
    unsigned char FREND1;    // Front end RX configuration.
    unsigned char FREND0;    // Front end RX configuration.
    unsigned char FSCAL3;    // Frequency synthesizer calibration.
    unsigned char FSCAL2;    // Frequency synthesizer calibration.
    unsigned char FSCAL1;    // Frequency synthesizer calibration.
    unsigned char FSCAL0;    // Frequency synthesizer calibration.
    unsigned char RCCCTRL1;
    unsigned char RCCCTRL0;
    unsigned char FSTEST;    // Frequency synthesizer calibration control
    unsigned char PTEST;
    unsigned char AGCTEST;
    unsigned char TEST2;     // Various test settings.
    unsigned char TEST1;     // Various test settings.
    unsigned char TEST0;     // Various test settings.
} RF_SETTINGS;
///////////////////////////////////////////////////////////////////////////////////////
// Definitions to support burst/single access:
#define WRITE_BURST     0x40
#define READ_SINGLE     0x80
#define READ_BURST      0xC0
///////////////////////////////////////////////////////////////////////////////////////
#define CRC_OK              0x80
#define RSSI                0
#define LQI                 1
#define BYTES_IN_RXFIFO     0x7F
///////////////////////////////////////////////////////////////////////////////////////
// Definitions for chip status
#define CHIP_RDY                        0x80
#define CHIP_STATE_MASK                 0x70
#define CHIP_STATE_IDLE                 0x00
#define CHIP_STATE_RX                   0x10
#define CHIP_STATE_TX                   0x20
#define CHIP_STATE_FSTON                0x30
#define CHIP_STATE_CALIBRATE            0x40
#define CHIP_STATE_SETTLING             0x50
#define CHIP_STATE_RXFIFO_OVERFLOW      0x60
#define CHIP_STATE_TXFIFO_UNDERFLOW     0x70
#define FIFO_BYTES_MASK                 0x0F
///////////////////////////////////////////////////////////////////////////////////////
/**
 * CC1101 Low-Power Sub-1 GHz RF Transceiver .
 */

enum Cc1101GDOConfig
{
    kGDO_TxRxSyncWord = 0x06,
    kGDO_PacketRxWithCrcOK = 0x07,
    kGDO_HighImpedance = 0x2e,
};

enum CC1101_TxPowers
{
	kTx12dBm = 0xc0,
	kTx10dBm = 0xc5,
	kTx7dBm = 0xcd,
	kTx5dBm = 0x86,
	kTx0dBm = 0x50,
	kTx_6dBm = 0x37,
	kTx_10dBm = 0x26,
	kTx_15dBm = 0x1d,
	kTx_20dBm = 0x17,
	kTx_30dBm = 0x03,
};

class CC1101
{
public:
    /**
     * Constructor.
     *
     * @param mosi mbed pin to use for MOSI line of SPI interface.
     * @param miso mbed pin to use for MISO line of SPI interface.
     * @param clk mbed pin to use for SCK line of SPI interface.
     * @param csn mbed pin to use for not chip select line of SPI interface.
     * @param gdo2 mbed pin connected to GDO2 pin for TX checking.
     */
    CC1101(PinName mosi, PinName miso, PinName clk, PinName csn, PinName gdo2);

    /**
     * Initialize CC1101 parameters.
     */
    void init(const RF_SETTINGS *pRfSettings);
    void init(const uint8_t *pRfSettings);

    /**
     * This function gets the value of a single specified CCxxx0 register.
     *
     * @param addr Value of the accessed CCxxx0 register
     * @return Value of the accessed CCxxx0 register
     */
    unsigned char ReadReg(unsigned char addr);

    /**
     * This function reads multiple CCxxx0 register, using SPI burst access.
     *
     * @param addr Value of the accessed CCxxx0 register
     * @param *buffer Pointer to a byte array which stores the values read from a
     *                corresponding range of CCxxx0 registers.
     * @param count Number of bytes to be read from the subsequent CCxxx0 registers.
     */
    void ReadBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count);

    /**
     * Function for writing to a single CCxxx0 register
     *
     * @param addr Address of the first CCxxx0 register to be accessed.
     * @param value Value to be written to the specified CCxxx0 register.
     */
    void WriteReg(unsigned char addr, unsigned char value);

    /**
     * This function writes to multiple CCxxx0 register, using SPI burst access.
     *
     * @param addr Address of the first CCxxx0 register to be accessed.
     * @param *buffer Array of bytes to be written into a corresponding range of
     *                CCxx00 registers, starting by the address specified in _addr_.
     * @param count Number of bytes to be written to the subsequent CCxxx0 registers.
     */
    void WriteBurstReg(unsigned char addr, const void *buffer, unsigned char count);

    /**
     * This function can be used to transmit a packet with packet length up to 63 bytes.
     *
     * @param *txBuffer Pointer to a buffer containing the data that are going to be transmitted
     * @param size The size of the txBuffer
     */
    void SendPacket(const void *txBuffer, unsigned char size);
    void SendPacketAsync(const void *txBuffer, unsigned char size);

    /**
     * This function check if the TX FIFO is empty
     *
     * @return Return value is 1 if the TX FIFO buffer is empty or else 0
     */
    unsigned char TxFifoEmpty(void);

    /**
     * This function can be used to receive a packet of variable packet length (first byte in the packet
     * must be the length byte). The packet length should not exceed the RX FIFO size.
     *
     * @param *rxBuffer Pointer to the buffer where the incoming data should be stored
     * @param *length Pointer to a variable containing the size of the buffer where the incoming data should be
     *                stored. After this function returns, that variable holds the packet length.
     * @return Return value is 1 if CRC OK or else 0 if CRC NOT OK (or no packet was put in the RX FIFO due to filtering)
     */
    int ReceivePacket(unsigned char *rxBuffer, unsigned char *length);
    // 与上面函数不同的是,长度字节放在缓冲的第一个字节里面
    int ReceivePacket(unsigned char *rxBuffer);
    bool ReceivePacket(unsigned char *rxBuffer, int max_len);

    /**
     * This function check if the RX FIFO is empty
     *
     * @return Return value is 1 if the RX FIFO buffer is empty or else 0
     */
    unsigned char RxFifoEmpty(void);

    /**
     * This function returns the Chip Status RX register
     *
     * @return Return the Chip Status RX register
     */
    unsigned char ReadChipStatusRX(void);

    /**
     * This function returns the Chip Status TX register
     *
     * @return Return the Chip Status TX register
     */
    unsigned char ReadChipStatusTX(void);

    /**
     * This function returns the RSSI value based from the last packet received
     *
     * @return Return the RSSI value.
     */
    unsigned char RdRSSI(void);
    unsigned char rssi(void);

    /**
     * This function returns the LQI value based from the last packet received
     *
     * @return Return the LQI value.
     */
    unsigned char RdLQI(void);

    /**
     * This function flushes the RX FIFO buffer.
     */
    void FlushRX(void);

    /**
     * This function flushes the TX FIFO buffer.
     */
    void FlushTX(void);

    /**
     * This function change the state of CC1101 to RX mode.
     */
    void RXMode(void);

    /**
     * This function return GDO0 input pin status.
     */
    int GetGDO0(void);

    /**
     * This function return GDO2 input pin status.
     */
    int GetGDO2(void);

    void SetChannel(uint8_t channel);
    uint8_t GetChannel();


    unsigned char ReadStatus(unsigned char addr);

protected:
    void RESET_CCxxx0(void);
    void POWER_UP_RESET_CCxxx0(void);
    unsigned char Strobe(unsigned char strobe);
    void WriteRfSettings(const RF_SETTINGS *pRfSettings);

    SPI _spi;
    DigitalOut _csn;
    SimpleIn _RDmiso;
    unsigned char rssi_;
    unsigned char lqi_;
    DigitalIn _gdo2;
    uint8_t channel;
};
///////////////////////////////////////////////////////////////////////////////////////
#endif

