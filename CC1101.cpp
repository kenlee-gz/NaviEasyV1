#include "CC1101.h"

#include "Debug.h"
#include "SimpleIn.h"

//#define RF_0db
#define RF_10db

CC1101::CC1101(PinName mosi, PinName miso, PinName clk, PinName csn, PinName gdo2) :
        _spi(mosi, miso, clk), _csn(csn), _RDmiso(miso), _gdo2(gdo2)
{
}

#ifdef RF_0db
// PATABLE (0 dBm output power)
char paTable[] =
{ 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#endif

#ifdef RF_10db
// PATABLE (10 dBm output power)
char paTable[] =
{   kTx12dBm, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Macro to reset the CCxxx0 and wait for it to be ready
void CC1101::RESET_CCxxx0(void)
{
//  while (_RDmiso);
    _csn = 0;
    wait_us(2);
    while (_RDmiso)
        ;
    _spi.write(CCxxx0_SRES);
    wait_us(2);
    _csn = 1;
}
///////////////////////////////////////////////////////////////////////////////////////
// Macro to reset the CCxxx0 after power_on and wait for it to be ready
// IMPORTANT NOTICE:
// The file Wait.c must be included if this macro shall be used
// The file is located under: ..\Lib\Chipcon\Hal\CCxx00
//
//                 min 40 us
//             <----------------------->
// CSn      |--|  |--------------------|          |-----------
//          |  |  |                    |          |
//              --                      ----------
//
// MISO                                       |---------------
//          - - - - - - - - - - - - - - - -|  |
//                                          --
//               Unknown / don't care
//
// MOSI     - - - - - - - - - - - - - - - ---------- - - - - -
//                                         | SRES |
//          - - - - - - - - - - - - - - - ---------- - - - - -
//
void CC1101::POWER_UP_RESET_CCxxx0(void)
{
    _csn = 1;
    wait_us(1);
    _csn = 0;
    wait_us(1);
    _csn = 1;
    wait_us(41);
    RESET_CCxxx0();
}
///////////////////////////////////////////////////////////////////////////////////////
//  void Strobe(unsigned char strobe)
//
//  DESCRIPTION:
//      Function for writing a strobe command to the CCxxx0
//
//  ARGUMENTS:
//      unsigned char strobe
//          Strobe command
///////////////////////////////////////////////////////////////////////////////////////
unsigned char CC1101::Strobe(unsigned char strobe)
{
    unsigned char x;
    wait_us(5);
    _csn = 0;
    wait_us(2);
    while (_RDmiso)
        ;
    x = _spi.write(strobe);
    wait_us(2);
    _csn = 1;
    return x;
}  // Strobe
///////////////////////////////////////////////////////////////////////////////////////
//  unsigned char ReadStatus(unsigned char addr)
//
//  DESCRIPTION:
//      This function reads a CCxxx0 status register.
//
//  ARGUMENTS:
//      unsigned char addr
//          Address of the CCxxx0 status register to be accessed.
//
//  RETURN VALUE:
//      unsigned char
//          Value of the accessed CCxxx0 status register.
///////////////////////////////////////////////////////////////////////////////////////
unsigned char CC1101::ReadStatus(unsigned char addr)
{
    unsigned char x;
    _csn = 0;
    wait_us(2);
    while (_RDmiso)
        ;
    _spi.write(addr | READ_BURST);
    x = _spi.write(0);
    wait_us(2);
    _csn = 1;
    return x;
}

///////////////////////////////////////////////////////////////////////////////////////
//  void WriteRfSettings(RF_SETTINGS *pRfSettings)
//
//  DESCRIPTION:
//      This function is used to configure the CCxxx0 based on a given rf setting
//
//  ARGUMENTS:
//      RF_SETTINGS *pRfSettings
//          Pointer to a struct containing rf register settings
///////////////////////////////////////////////////////////////////////////////////////
void CC1101::WriteRfSettings(const RF_SETTINGS *pRfSettings)
{
#if 1
    // Write register settings
    WriteReg(CCxxx0_FSCTRL1, pRfSettings->FSCTRL1);
    WriteReg(CCxxx0_FSCTRL0, pRfSettings->FSCTRL0);
    WriteReg(CCxxx0_FREQ2, pRfSettings->FREQ2);
    WriteReg(CCxxx0_FREQ1, pRfSettings->FREQ1);
    WriteReg(CCxxx0_FREQ0, pRfSettings->FREQ0);
    WriteReg(CCxxx0_MDMCFG4, pRfSettings->MDMCFG4);
    WriteReg(CCxxx0_MDMCFG3, pRfSettings->MDMCFG3);
    WriteReg(CCxxx0_MDMCFG2, pRfSettings->MDMCFG2);
    WriteReg(CCxxx0_MDMCFG1, pRfSettings->MDMCFG1);
    WriteReg(CCxxx0_MDMCFG0, pRfSettings->MDMCFG0);
    WriteReg(CCxxx0_CHANNR, pRfSettings->CHANNR);
    WriteReg(CCxxx0_DEVIATN, pRfSettings->DEVIATN);
    WriteReg(CCxxx0_FREND1, pRfSettings->FREND1);
    WriteReg(CCxxx0_FREND0, pRfSettings->FREND0);
    WriteReg(CCxxx0_MCSM0, pRfSettings->MCSM0);
    WriteReg(CCxxx0_FOCCFG, pRfSettings->FOCCFG);
    WriteReg(CCxxx0_BSCFG, pRfSettings->BSCFG);
    WriteReg(CCxxx0_AGCCTRL2, pRfSettings->AGCCTRL2);
    WriteReg(CCxxx0_AGCCTRL1, pRfSettings->AGCCTRL1);
    WriteReg(CCxxx0_AGCCTRL0, pRfSettings->AGCCTRL0);
    WriteReg(CCxxx0_FSCAL3, pRfSettings->FSCAL3);
    WriteReg(CCxxx0_FSCAL2, pRfSettings->FSCAL2);
    WriteReg(CCxxx0_FSCAL1, pRfSettings->FSCAL1);
    WriteReg(CCxxx0_FSCAL0, pRfSettings->FSCAL0);
    WriteReg(CCxxx0_FSTEST, pRfSettings->FSTEST);
    WriteReg(CCxxx0_TEST2, pRfSettings->TEST2);
    WriteReg(CCxxx0_TEST1, pRfSettings->TEST1);
    WriteReg(CCxxx0_TEST0, pRfSettings->TEST0);
    WriteReg(CCxxx0_FIFOTHR, pRfSettings->FIFOTHR);
    WriteReg(CCxxx0_IOCFG2, pRfSettings->IOCFG2);
    WriteReg(CCxxx0_IOCFG0, pRfSettings->IOCFG0);
    WriteReg(CCxxx0_PKTCTRL1, pRfSettings->PKTCTRL1);
    WriteReg(CCxxx0_PKTCTRL0, pRfSettings->PKTCTRL0);
    WriteReg(CCxxx0_ADDR, pRfSettings->ADDR);
    WriteReg(CCxxx0_PKTLEN, pRfSettings->PKTLEN);
    WriteReg(CCxxx0_MCSM1, pRfSettings->MCSM1);

    WriteReg(CCxxx0_MCSM2, pRfSettings->MCSM2);
    WriteReg(CCxxx0_SYNC1, pRfSettings->SYNC1);
    WriteReg(CCxxx0_SYNC0, pRfSettings->SYNC0);

    channel = pRfSettings->CHANNR;
//    WriteReg(CCxxx0_WOREVT1, pRfSettings->WOREVT1);
//    WriteReg(CCxxx0_WOREVT0, pRfSettings->WOREVT0);
//    WriteReg(CCxxx0_WORCTRL, pRfSettings->WORCTRL);
//    WriteReg(CCxxx0_RCCTRL1, pRfSettings->RCCCTRL1);
//    WriteReg(CCxxx0_RCCTRL0, pRfSettings->RCCCTRL0);
//    WriteReg(CCxxx0_PTEST, pRfSettings->PTEST);
//    WriteReg(CCxxx0_AGCTEST, pRfSettings->AGCTEST);
#else
    for(int i = 0; i < 47; i++)
    {
        WriteReg(i, *((uint8_t*)pRfSettings+i));
    }
#endif
    RXMode();

}  // WriteRfSettings

///////////////////////////////////////////////////////////////////////////////////////
void CC1101::init(const RF_SETTINGS *pRfSettings)
{
    _csn = 1;

    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    _spi.format(8, 0);
    _spi.frequency(500000);

    wait_ms(5);
    POWER_UP_RESET_CCxxx0();
    Strobe(CCxxx0_SRX);
    wait_ms(5);

    WriteRfSettings(pRfSettings);
    WriteReg(CCxxx0_PATABLE, paTable[0]);
}

void CC1101::init(const uint8_t *pRfSettings)
{
    _csn = 1;
//    _gdo0.mode(PullUp);
    _gdo2.mode(PullUp);

    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    _spi.format(8, 0);
    _spi.frequency(500000);
    wait_ms(5);

    POWER_UP_RESET_CCxxx0();
    Strobe(CCxxx0_SRX);

    wait_ms(5);
    for(int i = 0; i < 47; i++)
    {
        WriteReg(i, *((uint8_t*)pRfSettings+i));
    }
    WriteReg(CCxxx0_PATABLE, paTable[0]);
}

///////////////////////////////////////////////////////////////////////////////////////
//  unsigned char ReadReg(unsigned char addr)
//
//  DESCRIPTION:
//      This function gets the value of a single specified CCxxx0 register.
//
//  ARGUMENTS:
//      unsigned char addr
//          Address of the CCxxx0 register to be accessed.
//
//  RETURN VALUE:
//      unsigned char
//          Value of the accessed CCxxx0 register.
///////////////////////////////////////////////////////////////////////////////////////
unsigned char CC1101::ReadReg(unsigned char addr)
{
    unsigned char x;
    wait_us(5);
    _csn = 0;
    wait_us(2);
    while (_RDmiso)
        ;
    _spi.write(addr | READ_SINGLE);
    x = _spi.write(0);
    wait_us(2);
    _csn = 1;
    return x;
}  // ReadReg

///////////////////////////////////////////////////////////////////////////////////////
//  void ReadBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count)
//
//  DESCRIPTION:
//      This function reads multiple CCxxx0 register, using SPI burst access.
//
//  ARGUMENTS:
//      unsigned char addr
//          Address of the first CCxxx0 register to be accessed.
//      unsigned char *buffer
//          Pointer to a byte array which stores the values read from a
//          corresponding range of CCxxx0 registers.
//      unsigned char count
//          Number of bytes to be read from the subsequent CCxxx0 registers.
///////////////////////////////////////////////////////////////////////////////////////
void CC1101::ReadBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count)
{
    unsigned char i;
    wait_us(5);
    _csn = 0;
    wait_us(2);
    while (_RDmiso)
        ;
    _spi.write(addr | READ_BURST);
    for (i = 0; i < count; i++)
    {
        buffer[i] = _spi.write(0);
    }
    wait_us(2);
    _csn = 1;
}  // ReadBurstReg

///////////////////////////////////////////////////////////////////////////////////////
//  void WriteReg(unsigned char addr, unsigned char value)
//
//  DESCRIPTION:
//      Function for writing to a single CCxxx0 register
//
//  ARGUMENTS:
//      unsigned char addr
//          Address of a specific CCxxx0 register to accessed.
//      unsigned char value
//          Value to be written to the specified CCxxx0 register.
///////////////////////////////////////////////////////////////////////////////////////
void CC1101::WriteReg(unsigned char addr, unsigned char value)
{
    wait_us(5);
    _csn = 0;
    wait_us(2);
    while (_RDmiso)
        ;
    _spi.write(addr);
    _spi.write(value);
    wait_us(2);
    _csn = 1;
}  // WriteReg

///////////////////////////////////////////////////////////////////////////////////////
//  void WriteBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count)
//
//  DESCRIPTION:
//      This function writes to multiple CCxxx0 register, using SPI burst access.
//
//  ARGUMENTS:
//      unsigned char addr
//          Address of the first CCxxx0 register to be accessed.
//      unsigned char *buffer
//          Array of bytes to be written into a corresponding range of
//          CCxx00 registers, starting by the address specified in _addr_.
//      unsigned char count
//          Number of bytes to be written to the subsequent CCxxx0 registers.
///////////////////////////////////////////////////////////////////////////////////////
void CC1101::WriteBurstReg(unsigned char addr, const void *buffer, unsigned char count)
{
    unsigned char i;
    wait_us(5);
    _csn = 0;
    wait_us(2);
    while (_RDmiso)
        ;
    _spi.write(addr | WRITE_BURST);
    for (i = 0; i < count; i++)
    {
        _spi.write(*((const uint8_t*)buffer+i));
    }
    wait_us(2);
    _csn = 1;
}  // WriteBurstReg

///////////////////////////////////////////////////////////////////////////////////////
unsigned char CC1101::rssi(void)
{
    return rssi_;
}

unsigned char CC1101::RdRSSI(void)
{
    unsigned char crssi;

    if (rssi_ >= 128)
    {
        crssi = 255 - rssi_;
        crssi /= 2;
    }
    else
    {
        crssi = rssi_ / 2;
    }
	crssi += 74;
    return crssi;
}
///////////////////////////////////////////////////////////////////////////////////////
unsigned char CC1101::RdLQI(void)
{
    unsigned char clqi;
    clqi = 0x3F - (lqi_ & 0x3F);

    return clqi;
}
///////////////////////////////////////////////////////////////////////////////////////
unsigned char CC1101::RxFifoEmpty(void)
{
    unsigned char RxFifoStatus;

    Strobe(CCxxx0_SRX);
    RxFifoStatus = ReadStatus(CCxxx0_RXBYTES);

    if (RxFifoStatus & 0x80)
    {    // check for RXFIFO overflow
        // Make sure that the radio is in IDLE state before flushing the FIFO
        // (Unless RXOFF_MODE has been changed, the radio should be in IDLE state at this point)
        Strobe(CCxxx0_SIDLE);

        // Flush RX FIFO
        Strobe(CCxxx0_SFRX);
    }
    if (RxFifoStatus & ~0x80)
    {
        return 0;
    }
    else
        return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------------
//  BOOL ReceivePacket(unsigned char *rxBuffer, unsigned char *length)
//
//  DESCRIPTION:
//      This function can be used to receive a packet of variable packet length (first byte in the packet
//      must be the length byte). The packet length should not exceed the RX FIFO size.
//
//  ARGUMENTS:
//      unsigned char *rxBuffer
//          Pointer to the buffer where the incoming data should be stored
//      unsigned char *length
//          Pointer to a variable containing the size of the buffer where the incoming data should be
//          stored. After this function returns, that variable holds the packet length.
//
//  RETURN VALUE:
//      BOOL
//          1:   CRC OK
//          0:  CRC NOT OK (or no packet was put in the RX FIFO due to filtering)
///////////////////////////////////////////////////////////////////////////////////////

int CC1101::ReceivePacket(unsigned char *rxBuffer, unsigned char *length)
{
    unsigned char status[2];
    unsigned char packetLength;

    packetLength = ReadStatus(CCxxx0_RXBYTES);
    if (packetLength & BYTES_IN_RXFIFO)
    {
        // Read length byte
        packetLength = ReadReg(CCxxx0_RXFIFO);

        // Read data from RX FIFO and store in rxBuffer
        if (packetLength <= *length)
        {
            ReadBurstReg(CCxxx0_RXFIFO, rxBuffer, packetLength);
            *length = packetLength;

            // Read the 2 appended status bytes (status[0] = RSSI, status[1] = LQI)
            ReadBurstReg(CCxxx0_RXFIFO, status, 2);

            rssi_ = status[RSSI];
            lqi_ = status[LQI];
            // MSB of LQI is the CRC_OK bit
//            return (status[LQI] & CRC_OK);
            if (status[LQI] & CRC_OK)
            {
                return 1;
            }
        }
        else
        {
//            printf("too big packet length=%u/%u RXBYTES=%u\r\n", packetLength, *length, bytes);
            *length = packetLength;

            // Make sure that the radio is in IDLE state before flushing the FIFO
            // (Unless RXOFF_MODE has been changed, the radio should be in IDLE state at this point)
            Strobe(CCxxx0_SIDLE);

            // Flush RX FIFO
            Strobe(CCxxx0_SFRX);
            return 0;
        }
    }
    else
        return 0;
    return 0;
}            // halRfReceivePacket

int CC1101::ReceivePacket(unsigned char *rxBuffer)
{
    unsigned char status[2];
    unsigned char packetLength;

    packetLength = ReadStatus(CCxxx0_RXBYTES);
    if (packetLength & BYTES_IN_RXFIFO)
    {
        // Read length byte
        packetLength = ReadReg(CCxxx0_RXFIFO);

        // Read data from RX FIFO and store in rxBuffer
        if (packetLength <= rxBuffer[0])
        {
            ReadBurstReg(CCxxx0_RXFIFO, rxBuffer+1, packetLength);
            rxBuffer[0] = packetLength;

            // Read the 2 appended status bytes (status[0] = RSSI, status[1] = LQI)
            ReadBurstReg(CCxxx0_RXFIFO, status, 2);

            rssi_ = status[RSSI];
            lqi_ = status[LQI];
            // MSB of LQI is the CRC_OK bit
//            return (status[LQI] & CRC_OK);
            if (status[LQI] & CRC_OK)
            {
                return 1;
            }
        }
        else
        {
//            printf("too big packet length=%u/%u RXBYTES=%u\r\n", packetLength, *length, bytes);
            rxBuffer[0] = packetLength;

            // Make sure that the radio is in IDLE state before flushing the FIFO
            // (Unless RXOFF_MODE has been changed, the radio should be in IDLE state at this point)
            Strobe(CCxxx0_SIDLE);

            // Flush RX FIFO
            Strobe(CCxxx0_SFRX);
            return 0;
        }
    }
    else
        return 0;
    return 0;
}            // halRfReceivePacket

bool CC1101::ReceivePacket(unsigned char *rxBuffer, int max_len)
{
    unsigned char status[2];
    unsigned char packetLength;

    packetLength = ReadStatus(CCxxx0_RXBYTES);
    if (packetLength & BYTES_IN_RXFIFO)
    {
        packetLength = ReadReg(CCxxx0_RXFIFO);

        if (packetLength <= max_len)
        {
            ReadBurstReg(CCxxx0_RXFIFO, rxBuffer+1, packetLength);
            rxBuffer[0] = packetLength;

            ReadBurstReg(CCxxx0_RXFIFO, status, 2);

            rssi_ = status[RSSI];
            lqi_ = status[LQI];
            if (status[LQI] & CRC_OK)
            {
                return true;
            }
        }
        else
        {
            rxBuffer[0] = packetLength;

            Strobe(CCxxx0_SIDLE);

            Strobe(CCxxx0_SFRX);
        }
    }
    else
    {
        packetLength = ReadReg(CCxxx0_RXFIFO);
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////
unsigned char CC1101::TxFifoEmpty(void)
{
    unsigned char TxFifoStatus;

    Strobe(CCxxx0_STX);
    TxFifoStatus = ReadStatus(CCxxx0_TXBYTES);

    if (TxFifoStatus & 0x80)
    {    // check for TXFIFO underflow
        // Make sure that the radio is in IDLE state before flushing the FIFO
        Strobe(CCxxx0_SIDLE);

        // Flush TX FIFO
        Strobe(CCxxx0_SFTX);
    }
    if (TxFifoStatus & ~0x80)
    {
        return 0;
    }
    else
        return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
//  void halRfSendPacket(unsigned char *txBuffer, unsigned char size)
//
//  DESCRIPTION:
//      This function can be used to transmit a packet with packet length up to 63 bytes.
//
//  ARGUMENTS:
//      unsigned char *txBuffer
//          Pointer to a buffer containing the data that are going to be transmitted
//
//      unsigned char size
//          The size of the txBuffer
///////////////////////////////////////////////////////////////////////////////////////
void CC1101::SendPacket(const void *txBuffer, unsigned char size)
{
    unsigned char i;
//    unsigned char buf[64];

//    buf[0] = size;
//    for (i = 0; i < size; i++)
//        buf[i + 1] = txBuffer[i];

//    WriteBurstReg(CCxxx0_TXFIFO, buf, size + 1);
#if 1
    FlushTX();
    WriteBurstReg(CCxxx0_TXFIFO, txBuffer, size);
#else
    WriteBurstReg(CCxxx0_TXFIFO, txBuffer, size);
    Strobe(CCxxx0_SIDLE);
#endif
    Strobe(CCxxx0_STX);

    //////////////////////////////////////////////////////////////////////////////////////////////
    // To send subsequent packet we should ensure packet is sent before return this function
    //////////////////////////////////////////////////////////////////////////////////////////////
    // Following check routine is valid when IOCFG2=0x06 is set.
    unsigned t;
    const unsigned safe_check_count = 1000000;

    t = 0;
    // Wait for GDO2 to be set -> sync transmitted
    while (!_gdo2 && t++ < safe_check_count)
        ;
    if (t == safe_check_count)
    {
//        printf("check timeout 1\r\n");
    }

    // Wait for GDO2 to be cleared -> end of packet
    t = 0;
    while (_gdo2 && t++ < safe_check_count)
        ;

    if (t == safe_check_count)
    {
//        printf("check timeout 2\r\n");
    }

}    // halRfSendPacket

void CC1101::SendPacketAsync(const void *txBuffer, unsigned char size)
{
    FlushTX();
    WriteBurstReg(CCxxx0_TXFIFO, txBuffer, size);
    Strobe(CCxxx0_STX);
}

///////////////////////////////////////////////////////////////////////////////////////
unsigned char CC1101::ReadChipStatusTX(void)
{
    unsigned char x;

    x = Strobe(CCxxx0_SNOP);
    return x;
}
///////////////////////////////////////////////////////////////////////////////////////
unsigned char CC1101::ReadChipStatusRX(void)
{
    unsigned char x;
    wait_us(5);
    _csn = 0;
    wait_us(2);
    while (_RDmiso)
        ;
    x = _spi.write(CCxxx0_PARTNUM | READ_BURST);
    wait_us(2);
    _csn = 1;
    return x;
}
///////////////////////////////////////////////////////////////////////////////////////
void CC1101::FlushRX(void)
{
    // Make sure that the radio is in IDLE state before flushing the FIFO
    Strobe(CCxxx0_SIDLE);

    // Flush RX FIFO
    Strobe(CCxxx0_SFRX);
}
///////////////////////////////////////////////////////////////////////////////////////
void CC1101::FlushTX(void)
{
    // Make sure that the radio is in IDLE state before flushing the FIFO
    Strobe(CCxxx0_SIDLE);

    // Flush TX FIFO
    Strobe(CCxxx0_SFTX);
}
///////////////////////////////////////////////////////////////////////////////////////
void CC1101::RXMode(void)
{
    Strobe(CCxxx0_SIDLE);
    Strobe(CCxxx0_SFRX);
    Strobe(CCxxx0_SRX);
}
///////////////////////////////////////////////////////////////////////////////////////
int CC1101::GetGDO0(void)
{
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////
int CC1101::GetGDO2(void)
{
    return _gdo2;
}
///////////////////////////////////////////////////////////////////////////////////////

void CC1101::SetChannel(uint8_t ch)
{
    WriteReg(CCxxx0_CHANNR, ch);
    channel = ch;
}

uint8_t CC1101::GetChannel()
{
    return channel;
}

