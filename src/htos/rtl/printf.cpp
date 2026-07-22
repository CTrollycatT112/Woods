/*++

MODULE: Print utils

AUTHOR: Trollycat

ABSTRACT: Print and formatting functions

--*/
#include "rtl/rtl.hpp"

#include "inbv/inbv.hpp"
#include "hal/kdcom.hpp"

EXTERN_C {
    VOID vsnwprintf(PWSTR Buffer,
                    PWSTR Format,
                    VA_LIST List)
    {
        ULONG Index      = 0;
        ULONG64 Number   = 0;

        PCHAR AnsiString = NULL;
        PWCHAR String    = NULL;

        WCHAR NumberBuffer[21] = { 0 };
        PWSTR Temp             = NumberBuffer;

        while (*Format)
        {
            if (*Format != L'%')
            {
                Buffer[Index++] = *Format++;
                continue;
            }

            Format++;

            WCHAR PadCharacter = L' ';
            if (*Format == L'0')
            {
                PadCharacter = L'0';
                Format++;
            }

            ULONG Width = 0;
            while (*Format >= L'0' && *Format <= L'9')
            {
                Width = Width * 10 + (*Format - L'0');
                Format++;
            }


            while (*Format == L'l' || *Format == L'h') {
                Format++;
            }

            switch (*Format++)
            {
                case L'%':
                    Buffer[Index++] = L'%';
                    break;
                case L'c':
                case L'w':
                    Buffer[Index++] = static_cast<WCHAR>(__builtin_va_arg(List, INT));
                    break;
                case L's': {
                    AnsiString = (PCHAR)__builtin_va_arg(List, PCHAR);
                    while (*AnsiString)
                    {
                        Buffer[Index++] = *AnsiString++;
                    }
                    break;
                }
                case L'S': {
                    String = (PWCHAR)__builtin_va_arg(List, PWCHAR);
                    while (*String)
                    {
                        Buffer[Index++] = *String++;
                    }
                    break;
                }

                case L'p': {
                    Buffer[Index++] = L'0';
                    Buffer[Index++] = L'x';

                    Number = (ULONG64)__builtin_va_arg(List, ULONG64);
                    Rtl::IntToString(Number,
                                     NumberBuffer,
                                     16,
                                     16,
                                     L'0');

                    Temp = NumberBuffer;

                    while (*Temp) Buffer[Index++] = *Temp++;
                    break;
                }
                case L'd': {
                    Number = (LONG64)__builtin_va_arg(List, LONG64);
                    Rtl::IntToString(Number, 
                                    NumberBuffer, 
                                      10, 
                             Width, 
                                           PadCharacter);
                 
                    Temp = NumberBuffer;
                    
                    while (*Temp) Buffer[Index++] = *Temp++;
                    break;
                }
                case L'u': {
                    Number = (ULONG64)__builtin_va_arg(List, ULONG64);
                    Rtl::IntToString(Number, NumberBuffer, 10, Width, PadCharacter);
                    Temp = NumberBuffer;
                    while (*Temp) Buffer[Index++] = *Temp++;
                    break;
                }
                case L'x': {
                    Number = (ULONG64)__builtin_va_arg(List, ULONG64);
                    Rtl::IntToString(Number, NumberBuffer, 16, Width, PadCharacter);
                    Temp = NumberBuffer;
                    while (*Temp) Buffer[Index++] = *Temp++;
                    break;
                }
            }
        }

        Buffer[Index] = L'\0';
    }

    VOID vsnprintf(PSTR Buffer,
                   PCSTR Format,
                   VA_LIST List)
    {

        ULONG Index    = 0;
        ULONG64 Number = 0;

        PCHAR AnsiString  = NULL;
        PWCHAR WideString = NULL;

        CHAR NumberBuffer[21] = { 0 };
        PCHAR Temp            = NumberBuffer;

        while (*Format) {

            if (*Format != '%') {
                Buffer[Index++] = *Format++;
                continue;
            }

            Format++;

            CHAR PadChar = ' ';

            if (*Format == '0') {
                PadChar = '0';
                Format++;
            }

            ULONG Width = 0;

            while (*Format >= '0' && *Format <= '9') {
                Width = Width * 10 + (*Format - '0');
                Format++;
            }

            while (*Format == 'l' || *Format == 'h') {
                Format++;
            }

            switch (*Format++) {
                case '%':
                    Buffer[Index++] = '%';
                    break;
                case 'c':
                    Buffer[Index++] = (CHAR)__builtin_va_arg(List, INT);
                    break;
                case 's': {
                    AnsiString = (PCHAR)__builtin_va_arg(List, PCHAR);
                    while (*AnsiString)
                        Buffer[Index++] = *AnsiString++;
                    break;
                }
                case 'S': {
                    WideString = (PWCHAR)__builtin_va_arg(List, PWCHAR);
                    while (*WideString)
                        Buffer[Index++] = (CHAR)*WideString++;
                    break;
                }
                case 'p': {
                    Buffer[Index++] = '0';
                    Buffer[Index++] = 'x';
                    Number = (ULONG64)__builtin_va_arg(List, ULONG64);
                    Rtl::IntToAnsiString(Number, NumberBuffer, 16, 16, '0');
                    Temp = NumberBuffer;
                    while (*Temp) Buffer[Index++] = *Temp++;
                    break;
                }
                case 'd': {
                    Number = (LONG64)__builtin_va_arg(List, LONG64);
                    Rtl::IntToAnsiString(Number, NumberBuffer, 10, Width, PadChar);
                    Temp = NumberBuffer;
                    while (*Temp) Buffer[Index++] = *Temp++;
                    break;
                }
                case 'u': {
                    Number = (ULONG64)__builtin_va_arg(List, ULONG64);
                    Rtl::IntToAnsiString(Number, NumberBuffer, 10, Width, PadChar);
                    Temp = NumberBuffer;
                    while (*Temp) Buffer[Index++] = *Temp++;
                    break;
                }
                case 'x': {
                    Number = (ULONG64)__builtin_va_arg(List, ULONG64);
                    Rtl::IntToAnsiString(Number, NumberBuffer, 16, Width, PadChar);
                    Temp = NumberBuffer;
                    while (*Temp) Buffer[Index++] = *Temp++;
                    break;
                }
            }
        }

        Buffer[Index] = '\0';

    }

    // IMPORTANT NOTE:
    // DO NOT TOUCH THIS
    // THIS IS FOR COMPATIBILITY WITH TLSF
    // THAT'S IT
    // DO NOT CALL OR USE THIS FUNCTION
    int printf(const char* format, ...)
    {
        CHAR Buffer[512] = { 0 };
        VA_LIST Args;
        __builtin_va_start(Args, format);
        ::vsnprintf(Buffer, format, Args);
        __builtin_va_end(Args);
        Inbv::WriteString(Buffer);
        return 0; 
    }
}

namespace Rtl
{
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
    }

    HTAPI
    VOID
    PrintFromArgumentList(PCWSTR Format,
                          VA_LIST List)
    {
        WCHAR Buffer[512] = { 0 };

        ::vsnwprintf(Buffer,
                    const_cast<PWSTR>(Format),
                    List);

        CHAR AnsiBuffer[512] = { 0 };
        UINT Length          = 0;

        while (Buffer[Length] != L'\0' && Length < 511)
        {
            AnsiBuffer[Length] = static_cast<CHAR>(Buffer[Length]);
            Length++;
        }
        
        AnsiBuffer[Length] = '\0';

        Inbv::WriteString(AnsiBuffer);
        Inbv::WriteString("\r\n");
    }

    HTAPI
    VOID
    KdPrintFromArgumentList(PCWSTR Format,
                            VA_LIST List)
    {
        WCHAR Buffer[512] = {  0 };

        ::vsnwprintf(Buffer,
                     const_cast<PWSTR>(Format),
                     List);

        CHAR AnsiBuffer[512] = {  0 };
        UINT Length          = 0;

        while (Buffer[Length] != L'\0' && Length < 511)
        {
            AnsiBuffer[Length] = static_cast<CHAR>(Buffer[Length]);
            Length++;
        }

        AnsiBuffer[Length] = '\0';

        Hal::Kd::Write(SERIAL_COM1_BASE,
                       AnsiBuffer,
                       Length);
        Hal::Kd::Write(SERIAL_COM1_BASE,
                       (PCHAR)"\r\n",
                       2);
    }
} // namespace Rtl