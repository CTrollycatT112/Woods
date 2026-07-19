// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Serial driver

AUTHOR: Trollycat

ABSTRACT: Serial driver used for early debugging (LARP)

--*/
#include "hal/kdcom.hpp"
#include "hal/ioport.hpp"

namespace Hal::Kd
{

    VOID
    ConfigureBaudRate(USHORT Com,
                      ULONG  Baud)
    {
        USHORT Divisor = (USHORT)(UART_MASTER_CLOCK_HZ / Baud); 

        WRITE_PORT_UCHAR(SERIAL_LINE_COMMAND_PORT(Com),
                         SERIAL_LINE_ENABLE_DLAB);
        WRITE_PORT_UCHAR(SERIAL_DATA_PORT(Com),
                         Divisor & SERIAL_DATA_MASK_LOW_BYTE);
        WRITE_PORT_UCHAR(SERIAL_DATA_PORT(Com) + SERIAL_INTERRUPT_ENABLE_PORT_OFFSET,
                         (Divisor >> SERIAL_DATA_MASK_HIGH_BYTE) & SERIAL_DATA_MASK_LOW_BYTE);

        WRITE_PORT_UCHAR(SERIAL_LINE_COMMAND_PORT(Com),
                         SERIAL_LCR_WORD_LENGTH_8);
    }

    VOID
    ConfigureLine(USHORT Com)
    {
        WRITE_PORT_UCHAR(SERIAL_LINE_COMMAND_PORT(Com),
                         SERIAL_LCR_WORD_LENGTH_8);
    }

    VOID
    ConfigureFifoBuffer(USHORT Com)
    {
        WRITE_PORT_UCHAR(SERIAL_FIFO_COMMAND_PORT(Com),
                         SERIAL_FCR_BOOT_CONFIG);
    }

    VOID
    ConfigureModem(USHORT Com)
    {
        WRITE_PORT_UCHAR(SERIAL_MODEM_COMMAND_PORT(Com),
                         SERIAL_MCR_BOOT_CONFIG);
    }

    INT
    Write(USHORT Com,
          PCHAR  Buffer,
          UINT   Len)
    {
        UINT IndexToBuffer = 0;

        while (IndexToBuffer < Len)
        {
            if (Buffer[IndexToBuffer] == '\n')
            {
                WriteByte(Com, '\r');
            }
            WriteByte(Com, Buffer[IndexToBuffer]);
            IndexToBuffer++;
        }

        return (INT)IndexToBuffer;
    }

    VOID
    Configure(USHORT Port,
              ULONG  Baud)
    {
        WRITE_PORT_UCHAR(Port + SERIAL_INTERRUPT_ENABLE_PORT_OFFSET, 
                         SERIAL_INTERRUPT_DISABLE_ALL);
        ConfigureBaudRate(Port, Baud);
        ConfigureLine(Port);
        ConfigureFifoBuffer(Port);
        ConfigureModem(Port);
    }
} // namespace Hal::Kd
