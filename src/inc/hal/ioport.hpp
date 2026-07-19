// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: I/O Port Communication

AUTHOR: Trollycat

ABSTRACT: Raw I/O assembly instructions

--*/
#pragma once

#include "htbase.hpp"

namespace Hal
{
    
    /*++

    ROUTINE: WRITE_PORT_UCHAR

    DESCRIPTION: Write a value to the port

    ARGUMENTS: Port  - The port to write to
               Value - The value to write 

    RETURNS: VOID

    --*/
    INLINE
    VOID
    WRITE_PORT_UCHAR(USHORT Port,
                     UCHAR  Value)
    {
        __asm__ volatile("outb %0, %1" : : "a"(Value), "Nd"(Port));
    }

    
    /*++

    ROUTINE: READ_PORT_UCHAR

    DESCRIPTION: Read a value from the port

    ARGUMENTS: Port - The port to read from

    RETURNS: UCHAR

    --*/
    NODISCARD
    INLINE
    UCHAR
    READ_PORT_UCHAR(USHORT Port)
    {
        UCHAR Value;
        __asm__ volatile ("inb %1, %0" : "=a"(Value) : "Nd"(Port));
        return Value;
    }
} // namespace Hal