/*++

MODULE: Runtime library

AUTHOR: Trollycat

ABSTRACT: Standard library functions and RTL wrappers

--*/
#pragma once

#include "htbase.hpp"
#include "mm/mm.hpp"


//
// Converts (c) to uppercase 
//
#define RtlUpperChar(c)               ((c) >= 'a' && (c) <= 'z' ? (c) - ' ' : (c))

//
// Converts (c) to lowercase
//
#define RtlLowerChar(c)               ((c) >= 'A' && (c) <= 'Z' ? (c) + ' ' : (c))

#ifndef RTL_CONSTANT_STRING
#define RTL_CONSTANT_STRING(s)  { sizeof(s)-sizeof((s)[0]), sizeof(s), const_cast<PWCH>(s) }
#endif

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

    ROUTINE: strdup

    DESCRIPTION: Clones a string by allocating new memory,
                 and copying contents

    ARGUMENTS: String - The string to clone

    RETURNS: PSTR

    --*/

    NODISCARD PCWSTR strdup(PCWSTR String);

    /*++

    ROUTINE: strtok

    DESCRIPTION: Split a wide string into pieces

    ARGUMENTS: Input     - The string to split
               Delimiter - The char that breaks the string
               Tokens    - The output tokens
               Count     - The number of tokens

    RETURNS: BOOLEAN

    --*/
    NODISCARD BOOLEAN strtok(PCWSTR  Input,
                             WCHAR   Delimiter,
                             PWSTR** Tokens,
                             ULONG*  Count);

    /*++

    ROUTINE: strncmp

    DESCRIPTION: Compare two wide strings

    ARGUMENTS: String1         - The first string to compare
               String2         - The second string to compare
               CaseInsensitive - If the compare should ignore case

    RETURNS: LONG

    --*/
    NODISCARD LONG strncmp(PCWSTR  String1,
                           PCWSTR  String2,
                           BOOLEAN CaseInsensitive);

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

    ROUTINE: CloneString

    DESCRIPTION: Runtime wrapper for strdup

    ARGUMENTS: String - String to clone

    RETURNS: PCWSTR

    --*/
    NODISCARD
    INLINE
    HTAPI
    PCWSTR
    CloneString(PCWSTR String)
    {
        return ::strdup(String);
    }

    /*++

    ROUTINE: SplitString

    DESCRIPTION: RTL wrapper for strtok

    ARGUMENTS: Input     - The string to split
               Delimiter - The char that breaks the string
               Tokens    - The output tokens
               Count     - The number of tokens

    RETURNS: BOOLEAN

    --*/
    NODISCARD
    INLINE
    HTAPI
    BOOLEAN
    SplitString(PCWSTR  Input,
                WCHAR   Delimiter,
                PWSTR** Tokens,
                ULONG*  Count)
    {
        return ::strtok(Input, Delimiter, Tokens, Count);
    }

    /*++

    ROUTINE: CompareString

    DESCRIPTION: RTL wrapper for strncmp

    ARGUMENTS: String1         - The first string to compare
               String2         - The second string to compare
               CaseInsensitive - If the compare should ignore case

    RETURNS: LONG

    --*/
    NODISCARD
    INLINE
    HTAPI
    LONG
    CompareString(PCWSTR  String1,
                  PCWSTR  String2,
                  BOOLEAN CaseInsensitive)
    {
        return ::strncmp(String1, String2, CaseInsensitive);
    }

    /*++

    ROUTINE: FreeSplitString

    DESCRIPTION: Free memory used by split tokens

    ARGUMENTS: Tokens - The token array to free
               Count  - The number of token blocks

    RETURNS: VOID

    --*/
    INLINE
    HTAPI
    VOID
    FreeSplitString(PWSTR** Tokens, ULONG Count)
    {
        if (!Tokens)
        {
            return;
        }

        for (ULONG I = 0; I < Count; I++)
        {
            if (Tokens[I])
            {
                Mm::FreePool(Tokens[I], MAKE_TAG('R', 't', 'l', ' '));
            }
        }

        Mm::FreePool(Tokens, MAKE_TAG('R', 't', 'l', ' '));
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
    StringLength(PCWSTR String)
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

    ROUTINE: InitUnicodeString

    DESCRIPTION: Fill a unicode string from a wide string

    ARGUMENTS: String   - The unicode string to fill
               SrcString - The wide string to copy from

    RETURNS: VOID

    --*/
    INLINE
    VOID
    InitUnicodeString(PUNICODE_STRING String,
                      PWSTR           SrcString)
    {
        String->Buffer        = SrcString;
        String->Length        = StringLength(SrcString);
        String->MaximumLength = String->Length + 1;
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
            *Ptr++ = BASE36_ALPHABET[Value % Base];
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
            *Ptr++ = BASE36_ALPHABET[Value % Base];
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
    HTAPI
    VOID
    PrintFromArgumentList(PCSTR Format,
                          VA_LIST List);

    HTAPI
    VOID
    PrintFromArgumentList(PCWSTR Format,
                          VA_LIST List);

    HTAPI
    VOID
    KdPrintFromArgumentList(PCWSTR Format,
                            VA_LIST List);
    
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

    INLINE
    HTAPI
    VOID
    Print(PCWSTR Format,
          ...)
    {
        VA_LIST ArgumentList;
        __builtin_va_start(ArgumentList, Format);
        PrintFromArgumentList(Format,
                            ArgumentList);
        __builtin_va_end(ArgumentList);
    }

    INLINE
    HTAPI
    VOID
    KdPrint(PCWSTR Format,
            ...)
    {
        VA_LIST ArgumentList;
        __builtin_va_start(ArgumentList, Format);
        KdPrintFromArgumentList(Format,
                                ArgumentList);
        __builtin_va_end(ArgumentList);
    }

    /*++

    ROUTINE: AssertFailed

    DESCRIPTION: Handles a failed expression

    ARGUMENTS: Expression - The text of the failed condition
               File       - File name
               Line       - Line number
               Message    - Text to print (optional)

    RETURNS: VOID

    --*/
    HTAPI
    VOID
    AssertFailed(PCSTR Expression,
                 PCSTR File,
                 ULONG Line,
                 PCSTR Message = NULL);

    HTAPI
    VOID
    AssertFailed(PCSTR Expression,
                 PCSTR File,
                 ULONG Line,
                 PCWSTR Message);
} // namespace Rtl

//
// ASSERT() MACRO
//
#if defined(HTOS_CHK) && (HTOS_CHK == 1)
#define ASSERT(exp) \
    ((exp) ? (void)0 : Rtl::AssertFailed(#exp, __FILE__, __LINE__))
#define ASSERTMSG(message, exp) \
    ((exp) ? (void)0 : Rtl::AssertFailed(#exp, __FILE__, __LINE__, message))
#else
#define ASSERT(exp) ((void)0)
#define ASSERTMSG(message, exp) ((void)0)
#endif