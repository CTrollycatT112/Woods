/*++

MODULE: Serial driver

AUTHOR: Trollycat

ABSTRACT: Serial driver used for early debugging (LARP)

--*/
#pragma once

#include "hal/ioport.hpp"
#include "htbase.hpp"

//
// Serial I/O ports
//
#define SERIAL_COM1_BASE 0x3F8
#define SERIAL_DATA_PORT(base)          ((base))
#define SERIAL_FIFO_COMMAND_PORT(base)  ((base) + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  ((base) + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) ((base) + 4)
#define SERIAL_LINE_STATUS_PORT(base)   ((base) + 5)

//
// Serial baud rates
//
#define SERIAL_BAUD_RATE_38400  38400
#define SERIAL_BAUD_RATE_115200 115200
#define UART_MASTER_CLOCK_HZ    1843200

//
// Line control register flags
//
#define SERIAL_LINE_ENABLE_DLAB   0x80
#define SERIAL_LCR_WORD_LENGTH_8  0x03

//
// Line status register flags
//
#define SERIAL_LSR_DATA_READY     0x01
#define SERIAL_LSR_TRANSMIT_EMPTY 0x20
#define SERIAL_LSR_THRE           0x20

//
// Boot configurations
//
#define SERIAL_FCR_BOOT_CONFIG    0xC7
#define SERIAL_MCR_BOOT_CONFIG    0x03

//
// Data masking
//
#define SERIAL_DATA_MASK_LOW_BYTE  0x00FF
#define SERIAL_DATA_MASK_HIGH_BYTE 8

//
// Interrupts
//
#define SERIAL_INTERRUPT_DISABLE_ALL        0x00
#define SERIAL_INTERRUPT_ENABLE_PORT_OFFSET 1

#define SERIAL_LINE_ENABLE_DLAB 0x80

#define CLEAR_HOST_TERMINAL "\033[2J\033[H"

namespace Hal::Kd
{
    
    /*++

    ROUTINE: WriteByte

    DESCRIPTION: Sends a single byte over the serial port

    ARGUMENTS: Port - Port to send byte to,
               Byte - Byte to send

    RETURNS: VOID

    --*/

    INLINE
    VOID
    WriteByte(USHORT Port,
              CHAR   Byte)
    {
        while ((Hal::READ_PORT_UCHAR(SERIAL_LINE_STATUS_PORT(Port)) & 
                SERIAL_LSR_TRANSMIT_EMPTY) == 0)
        {}

        Hal::WRITE_PORT_UCHAR(SERIAL_DATA_PORT(Port), (UCHAR)Byte);

    }

    
    /*++

    ROUTINE: ConfigureBaudRate

    DESCRIPTION: Set the transmission speed

    ARGUMENTS: Com     - I/O address of serial port
               Baud    - Scaling clock 

    RETURNS: VOID

    --*/

    VOID
    ConfigureBaudRate(USHORT Com,
                      ULONG  Baud);

    /*++

    ROUTINE: ConfigureLine

    DESCRIPTION: Configure serial line, force 8 data bits

    ARGUMENTS: Com - I/O address of serial port

    RETURNS: VOID

    --*/

    VOID
    ConfigureLine(USHORT Com);

    /*++

    ROUTINE: ConfigureFifoBuffer

    DESCRIPTION: Enable hardware buffers

    ARGUMENTS: Com - I/O address of serial port

    RETURNS: VOID

    --*/
    VOID
    ConfigureFifoBuffer(USHORT Com);

    /*++

    ROUTINE: ConfigureModem

    DESCRIPTION: Open lines with the terminal

    ARGUMENTS: Com - I/O address of serial port

    RETURNS: VOID

    --*/
    VOID
    ConfigureModem(USHORT Com);

    /*++

    ROUTINE: Write

    DESCRIPTION: Loop through buffer, send each character to port

    ARGUMENTS: Com    - I/O address of serial port
               Buffer - Character buffer
               Len    - Number of bytes to read from buffer

    RETURNS: INT

    --*/
    INT
    Write(USHORT Com,
          PCHAR  Buffer,
          UINT   Len);

    /*++

    ROUTINE: Configure

    DESCRIPTION: Initialize the KDCOM

    ARGUMENTS: Port - I/O address of the serial port
               Buad - Speed for divisor

    RETURNS: VOID

    --*/
    VOID
    Configure(USHORT Port,
              ULONG  Baud);

} // namespace Hal::Kd