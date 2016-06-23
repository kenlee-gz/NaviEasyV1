
/**
 * @file    BufferedSerial.h
 * @brief   Software Buffer - Extends mbed Serial functionallity adding irq driven TX and RX
 * @author  sam grove
 * @version 1.0
 * @see     
 *
 * Copyright (c) 2013
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BUFFEREDSERIALEX_H
#define BUFFEREDSERIALEX_H
 
#include "mbed.h"
#include "CircularBufferA.h"

/** A serial port (UART) for communication with other serial devices
 *
 * Can be used for Full Duplex communication, or Simplex by specifying
 * one pin as NC (Not Connected)
 *
 * Example:
 * @code
 *  #include "mbed.h"
 *  #include "BufferedSerial.h"
 *
 *  BufferedSerialEx pc(USBTX, USBRX);
 *
 *  int main()
 *  { 
 *      while(1)
 *      {
 *          Timer s;
 *        
 *          s.start();
 *          pc.printf("Hello World - buffered\n");
 *          int buffered_time = s.read_us();
 *          wait(0.1f); // give time for the buffer to empty
 *        
 *          s.reset();
 *          printf("Hello World - blocking\n");
 *          int polled_time = s.read_us();
 *          s.stop();
 *          wait(0.1f); // give time for the buffer to empty
 *        
 *          pc.printf("printf buffered took %d us\n", buffered_time);
 *          pc.printf("printf blocking took %d us\n", polled_time);
 *          wait(0.5f);
 *      }
 *  }
 * @endcode
 */

/**
 *  @class BufferedSerialEx
 *  @brief Software buffers and interrupt driven tx and rx for Serial
 */  
class BufferedSerialEx : public SerialBase
{
private:
    CircularBufferA <char> _txbuf;
    uint32_t      _tx_multiple;
 
    void txIrq(void);
    void prime(void);
    
public:
    /** Create a BufferedSerial port, connected to the specified transmit and receive pins
     *  @param tx Transmit pin
     *  @param rx Receive pin
     *  @param buf_size printf() buffer size
     *  @param tx_multiple amount of max printf() present in the internal ring buffer at one time
     *  @param name optional name
     *  @note Either tx or rx may be specified as NC if unused
     */
    BufferedSerialEx(PinName tx, PinName rx);
    
    /** Destroy a BufferedSerial port
     */
    virtual ~BufferedSerialEx(void);
    
    /** Check to see if the tx buffer has room
     *  @return 1 always has room and can overwrite previous content if too small / slow
     */
    virtual int writeable(void);
    
    /** Get a single byte from the BufferedSerial Port.
     *  Should check readable() before calling this.
     *  @return A byte that came in on the Serial Port
     */
    int getc(void);
    
    /** Write a single byte to the BufferedSerial Port.
     *  @param c The byte to write to the Serial Port
     *  @return The byte that was written to the Serial Port Buffer
     */
    int putc(int c);
    
    /** Write a string to the BufferedSerial Port. Must be NULL terminated
     *  @param s The string to write to the Serial Port
     *  @return The number of bytes written to the Serial Port Buffer
     */
    int puts(const char *s);
    
    /** Write a formatted string to the BufferedSerial Port.
     *  @param format The string + format specifiers to write to the Serial Port
     *  @return The number of bytes written to the Serial Port Buffer
     */
    int printf(const char* format, ...);
    
    /** Write data to the Buffered Serial Port
     *  @param s A pointer to data to send
     *  @param length The amount of data being pointed to
     *  @return The number of bytes written to the Serial Port Buffer
     */
    virtual ssize_t write(const void *s, std::size_t length);
};

#endif
