// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Runtime library

AUTHOR: Trollycat

ABSTRACT: Standard library functions and RTL wrappers

--*/
#pragma once

#include "htbase.hpp"

#include "inbv/inbv.hpp"
#include "hal/kdcom.hpp"

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
    NODISCARD ULONG64 strlen(PCSTR String);

    /*++

    ROUTINE: wcslen

    DESCRIPTION: Same thing as strlen but for wide characters (2 bytes)

    ARGUMENTS: String - The string to pull length from

    RETURNS: ULONG64

    --*/
    NODISCARD ULONG64 wcslen(LPCWSTR String);

    /*++

    ROUTINE: vsnwprintf

    DESCRIPTION: Writes (wide) format into a character array

    ARGUMENTS: Buffer - Memory where string will be stroed
               Format - Format specifiers
               List   - Pack of arguments

    RETURNS: VOID

    --*/
    VOID vsnwprintf(PWSTR Buffer,
                    PWSTR Format,
                    VA_LIST List);

    /*++

    ROUTINE: vsnprintf

    DESCRIPTION: Writes (ansi) format into a character array

    ARGUMENTS: Buffer - Memory where string will be stroed
               Format - Format specifiers
               List   - Pack of arguments

    RETURNS: VOID

    --*/
    VOID vsnprintf(PSTR Buffer,
                   PCSTR Format,
                   VA_LIST List);
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
    NODISCARD
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
    NODISCARD
    INLINE
    HTAPI
    ULONG64
    AnsiStringLength(PCSTR String)
    {
        return ::strlen(String);
    }

    
    /*++

    ROUTINE: StringLength

    DESCRIPTION: RTL wrapper for wcslen

    ARGUMENTS: String - The string to pull length from

    RETURNS: ULONG64

    --*/
    NODISCARD
    INLINE
    HTAPI
    ULONG64
    StringLength(PWSTR String)
    {
        return ::wcslen(String);
    }

    /*++

    ROUTINE: ReverseString

    DESCRIPTION: Literally reverses a string

    ARGUMENTS: String - String to reverse..

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    ReverseString(PWSTR String)
    {
        ULONG I, J;
        WCHAR Temp;

        I = 0;
        J = (ULONG)StringLength(String) - 1;

        for (; I < J; I++, J--)
        {
            Temp = String[I];
            String[I] = String[J];
            String[J] = Temp;
        }
    }

    
    /*++

    ROUTINE: IntToString

    DESCRIPTION: Convert's an integer number to a string ""

    ARGUMENTS: Value         - Integer value
               Buffer        - Buffer to store String
               Base          - Radix system for text output
               MinimumDigits - Minimum width of string output

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    IntToString(ULONG64 Value,
                PWSTR Buffer,
                ULONG Base,
                ULONG MinimumDigits,
                WCHAR PadCharacter)
    {
        PWSTR Ptr;
        ULONG NumberOfDigits = 0;

        if (Base < 2 || Base > 36) {
            *Buffer = L'\0';
            return;
        }

        Ptr = Buffer;

        do {
            *Ptr++ = L"0123456789abcdefghijklmnopqrstuvwxyz"[Value % Base];
            Value /= Base;
            NumberOfDigits++;
        } while (Value);

        while (NumberOfDigits < MinimumDigits) {
            *Ptr++ = PadCharacter;
            NumberOfDigits++;
        }

        *Ptr = L'\0';
        ReverseString(Buffer);

    }

    /*++

    ROUTINE: FormatBufferFromArgumentList

    DESCRIPTION: RTL wrapper for vsnwprintf

    ARGUMENTS: Buffer - Memory where string will be stroed
               Format - Format specifiers
               List   - Pack of arguments

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    FormatBufferFromArgumentList(PWSTR Buffer,
                                 PWSTR Format,
                                 VA_LIST List)
    {
        ::vsnwprintf(Buffer,
                    Format,
                    List);
    }

    /*++

    ROUTINE: FormatBuffer

    DESCRIPTION: Wrapper around a wrapper..

    ARGUMENTS: Buffer - Memory where string will be stroed
               Format - Format specifiers
               ...    - Arguments

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    FormatBuffer(PWSTR Buffer, 
                 PWSTR Format,
                 ...)
    {
        VA_LIST ArgumentList;
        __builtin_va_start(ArgumentList, Format);
        FormatBufferFromArgumentList(Buffer, Format, ArgumentList);
        __builtin_va_end(ArgumentList);
    }

    /*++

    ROUTINE: ReverseAnsiString

    DESCRIPTION: Literally reverses (ansi) string

    ARGUMENTS: String - String to reverse..

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    ReverseAnsiString(PCHAR String)
    {
        ULONG I, J;
        CHAR Temp;

        I = 0;
        J = (ULONG) AnsiStringLength(String) - 1;

        for (; I < J; I++, J--)
        {
            Temp = String[I];
            String[I] = String[J];
            String[J] = Temp;
        }
    }

    /*++

    ROUTINE: IntToAnsiString

    DESCRIPTION: Convert's (ansi) integer number to a string ""

    ARGUMENTS: Value         - Integer value
               Buffer        - Buffer to store String
               Base          - Radix system for text output
               MinimumDigits - Minimum width of string output

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    IntToAnsiString(ULONG64 Value,
                    PCHAR Buffer,
                    ULONG Base,
                    ULONG MinimumDigits,
                    CHAR PadCharacter)
    {
        PCHAR Ptr;
        ULONG NumberOfDigits = 0;

        if (Base < 2 || Base > 36) {
            *Buffer = '\0';
            return;
        }

        Ptr = Buffer;

        do {
            *Ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[Value % Base];
            Value /= Base;
            NumberOfDigits++;
        } while (Value);

        while (NumberOfDigits < MinimumDigits) {
            *Ptr++ = PadCharacter;
            NumberOfDigits++;
        }

        *Ptr = '\0';
        ReverseAnsiString(Buffer);

    }

    /*++

    ROUTINE: FormatAnsiBufferFromArgumentList

    DESCRIPTION: RTL wrapper for vsnprintf

    ARGUMENTS: Buffer - Memory where string will be stroed
               Format - Format specifiers
               List   - Pack of arguments

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    FormatAnsiBufferFromArgumentList(PSTR Buffer,
                                     PCSTR Format,
                                     VA_LIST List)
    {
        ::vsnprintf(Buffer,
                    Format,
                    List);
    }

    /*++

    ROUTINE: FormatAnsiBuffer

    DESCRIPTION: Wrapper around (ansi)wrapper..

    ARGUMENTS: Buffer - Memory where string will be stroed
               Format - Format specifiers
               ...    - Arguments

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    FormatAnsiBuffer(PSTR Buffer,
                     PCSTR Format,
                     ...)
    {
        VA_LIST ArgumentList;
        __builtin_va_start(ArgumentList, Format);
        FormatAnsiBufferFromArgumentList(Buffer,
                                        Format,
                                        ArgumentList);
        __builtin_va_end(ArgumentList);
    }

    
    /*++

    ROUTINE: PrintFromArgumentList

    DESCRIPTION: Format an argument list into a buffer and print it

    ARGUMENTS: Format - Format specifiers
               List   - Argument list

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    PrintFromArgumentList(PCSTR Format,
                          VA_LIST List)
    {
        CHAR Buffer[512] = { 0 };

        ::vsnprintf(Buffer, Format, List);

        Inbv::WriteString(Buffer);
        Inbv::WriteString("\r\n");

        UINT Length = 0;
        while (Buffer[Length] != '\0' && Length < 512)
        {
            Length++;
        }

        Hal::Kd::Write(SERIAL_COM1_BASE,
                       Buffer,
                       Length);
        Hal::Kd::Write(SERIAL_COM1_BASE,
                       (PCHAR)"\r\n",
                       2);
    }
    
    /*++

    ROUTINE: Print

    DESCRIPTION: Print a string to the screen

    ARGUMENTS: Format - Format specifiers
               ...    - Arguments

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    Print(PCSTR Format,
          ...)
    {
        VA_LIST ArgumentList;
        __builtin_va_start(ArgumentList, Format);
        PrintFromArgumentList(Format,
                            ArgumentList);
        __builtin_va_end(ArgumentList);
    }
} // namespace Rtl