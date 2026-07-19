// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Runtime library

AUTHOR: Trollycat

ABSTRACT: Bare bones RTL functions

--*/
#pragma once

#include "htbase.hpp"

//
// Converts (c) to uppercase 
//
#define RtlUpperChar(c)               ((c) >= 'a' && (c) <= 'z' ? (c) - ' ' : (c))

//
// Converts (c) to lowercase
//
#define RtlLowerChar(c)               ((c) >= 'A' && (c) <= 'Z' ? (c) + ' ' : (c))

EXTERN_C {
        
    /*++

    ROUTINE: memset

    DESCRIPTION: Fill a block of memory with a byte

    ARGUMENTS: Dest - Block of memory, Ch - Value to be set, Count - Number of bytes to be set to the value

    RETURNS: PVOID

    --*/
    PVOID memset(PVOID Dest, INT Ch, QWORD Count);

    /*++

    ROUTINE: memcpy

    DESCRIPTION: Copy a number of bytes from one spot to another

    ARGUMENTS: Dest - Where to copy to, Src - Block to copy from, Count - Number of bytes to copy

    RETURNS: PVOID

    --*/
    PVOID memcpy(PVOID Dest, PCVOID Src, QWORD Count);

    /*++

    ROUTINE: memmove

    DESCRIPTION: A safer version of memcpy

    ARGUMENTS: Dest - Where to copy to, Src - Block to copy from, Count - Number of bytes to copy

    RETURNS: PVOID

    --*/
    PVOID memmove(PVOID Dest, PCVOID Src, QWORD Count);

    /*++

    ROUTINE: memcmp

    DESCRIPTION: Compares the first n bytes of two memory blocks byte by byte

    ARGUMENTS: Lhs - First memory block, Rhs - Second memory block, Count - Number of bytes

    RETURNS: INT

    --*/
    INT memcmp(PCVOID Lhs, PCVOID Rhs, QWORD Count);

    /*++

    ROUTINE: strlen

    DESCRIPTION: Get the length of a string (doesn't include null terminator)

    ARGUMENTS: String - The string to pull length from

    RETURNS: ULONG64

    --*/
    ULONG64 strlen(PCSTR String);
}

namespace Rtl 
{
    /*++

    ROUTINE: FillMemory

    DESCRIPTION: RTL wrapper for memset

    ARGUMENTS: Destination - Block of memory, Length - Number of bytes to be set, Fill - Value to be set

    RETURNS: VOID

    --*/
    INLINE
    HTAPI 
    VOID 
    FillMemory(PVOID Destination, 
               QWORD Length, 
               UCHAR Fill) 
    {
        ::memset(Destination, Fill, Length);
    }

    /*++

    ROUTINE: ZeroMemory

    DESCRIPTION: RTL wrapper to zero out memory block

    ARGUMENTS: Destination - Block of memory, Length - Number of bytes to zero

    RETURNS: VOID

    --*/
    INLINE
    HTAPI 
    VOID 
    ZeroMemory(PVOID Destination, 
               QWORD Length) 
    {
        ::memset(Destination, 0, Length);
    }

    /*++

    ROUTINE: CopyMemory

    DESCRIPTION: RTL wrapper for memcpy

    ARGUMENTS: Destination - Where to copy to, Source - Block to copy from, Length - Number of bytes to copy

    RETURNS: VOID

    --*/
    INLINE
    HTAPI 
    VOID 
    CopyMemory(PVOID Destination, 
               PCVOID Source, 
               QWORD Length) 
    {
        ::memcpy(Destination, Source, Length);
    }

    /*++

    ROUTINE: MoveMemory

    DESCRIPTION: RTL wrapper for memmove

    ARGUMENTS: Destination - Where to copy to, Source - Block to copy from, Length - Number of bytes to copy

    RETURNS: VOID

    --*/
    INLINE
    HTAPI 
    VOID 
    MoveMemory(PVOID Destination, 
               PCVOID Source, 
               QWORD Length) 
    {
        ::memmove(Destination, Source, Length);
    }

    /*++

    ROUTINE: CompareMemory

    DESCRIPTION: Compares two blocks and returns the count of matching bytes

    ARGUMENTS: Source1 - First memory block, Source2 - Second memory block, Length - Number of bytes to compare

    RETURNS: QWORD

    --*/
    INLINE
    HTAPI 
    QWORD 
    CompareMemory(PCVOID Source1, 
                  PCVOID Source2, 
                  QWORD Length) 
    {
        PCUCHAR Ptr1 = static_cast<PCUCHAR>(Source1);
        PCUCHAR Ptr2 = static_cast<PCUCHAR>(Source2);
        QWORD MatchCount = 0;

        while (MatchCount < Length && Ptr1[MatchCount] == Ptr2[MatchCount]) {
            MatchCount++;
        }

        return MatchCount;
    }

    /*++

    ROUTINE: AnsiStringLength

    DESCRIPTION: RTL wrapper for strlen

    ARGUMENTS: String - The string to pull length from

    RETURNS: ULONG64

    --*/
    INLINE
    HTAPI
    ULONG64
    AnsiStringLength(PCSTR String)
    {
        return ::strlen(String);
    }
} // namespace Rtl