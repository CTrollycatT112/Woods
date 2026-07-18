// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Runtime library

AUTHOR: Trollycat

ABSTRACT Bare bones RTL functions

--*/
#pragma once

#include "htbase.hpp"

EXTERN_C {
    PVOID memset(PVOID Dest, INT Ch, QWORD Count);
    PVOID memcpy(PVOID Dest, PCVOID Src, QWORD Count);
    PVOID memmove(PVOID Dest, PCVOID Src, QWORD Count);
    INT   memcmp(PCVOID Lhs, PCVOID Rhs, QWORD Count);
}

namespace Rtl 
{
    STATIC
    HTAPI 
    VOID 
    FillMemory(PVOID Destination, 
               QWORD Length, 
               UCHAR Fill) 
    {
        ::memset(Destination, Fill, Length);
    }

    STATIC
    HTAPI 
    VOID 
    ZeroMemory(PVOID Destination, 
               QWORD Length) 
    {
        ::memset(Destination, 0, Length);
    }

    STATIC
    HTAPI 
    VOID 
    CopyMemory(PVOID Destination, 
               PCVOID Source, 
               QWORD Length) 
    {
        ::memcpy(Destination, Source, Length);
    }

    STATIC
    HTAPI 
    VOID 
    MoveMemory(PVOID Destination, 
               PCVOID Source, 
               QWORD Length) 
    {
        ::memmove(Destination, Source, Length);
    }

    STATIC
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
}