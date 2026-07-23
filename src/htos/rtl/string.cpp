/*++

MODULE: String utils

AUTHOR: Trollycat

ABSTRACT: Bare bones string functions so flanterm will work

--*/
#include "htbase.hpp"

#include "rtl/rtl.hpp"

EXTERN_C {

    NODISCARD
    ULONG64
    strlen(PCSTR String)
    {
        ULONG64 I = 0;

        while (String[I])
        {
            I++;
        }

        return I;
    }

    NODISCARD
    ULONG64
    wcslen(LPCWSTR String)
    {
        ULONG64 I = 0;

        while (String[I])
            I++;
        
        return I;
    }

    NODISCARD
    PCWSTR
    strdup(PCWSTR String)
    {
        ULONG64 Length = (Rtl::StringLength(String) + 1) * sizeof(WCHAR);
        PCWSTR  Return = (PCWSTR)Mm::AllocatePoolWithTag(
            NonPagedPool,
            Length,
            MAKE_TAG('R', 't', 'l', ' ')
        );

        if (Return == NULL)
        {
            return NULL;
        }

        Rtl::CopyMemory((PVOID)Return, String, Length);
        return Return;
    }

    NODISCARD
    BOOLEAN
    strtok(PCWSTR  Input, 
           WCHAR   Delimiter, 
           PWSTR** Tokens, 
           ULONG*  Count)
    {
        if (!Input || !Tokens || !Count)
        {
            return FALSE;
        }

        *Tokens = NULL;
        *Count  = 0;

        ULONG  tokenCount = 1;
        PCWSTR p          = Input;

        while (*p != L'\0')
        {
            if (*p == Delimiter)
            {
                tokenCount++;
            }

            p++;
        }

        PWSTR* tokenArray = (PWSTR*)Mm::AllocatePoolWithTag(
            NonPagedPool, 
            tokenCount * sizeof(PWSTR), 
            MAKE_TAG('R', 't', 'l', ' ')
        );

        if (!tokenArray)
        {
            return FALSE;
        }

        Rtl::ZeroMemory(
            tokenArray, 
            tokenCount * sizeof(PWSTR)
        );

        ULONG  index  = 0;
        PCWSTR pStart = Input;
        p             = Input;

        while (TRUE) 
        {
            if (*p == Delimiter || *p == L'\0') 
            {
                ULONG64 charCount = (ULONG64)(p - pStart);
                ULONG64 byteCount = (charCount + 1) * sizeof(WCHAR);

                PWSTR token = (PWSTR)Mm::AllocatePoolWithTag(
                    NonPagedPool, 
                    byteCount, 
                    MAKE_TAG('R', 't', 'l', ' ')
                );

                if (!token) 
                {
                    for (ULONG i = 0; i < index; i++)
                    {
                        Mm::FreePool(tokenArray[i],
                                     MAKE_TAG('R', 't', 'l', ' '));
                    }

                    Mm::FreePool(tokenArray, 
                                 MAKE_TAG('R', 't', 'l', ' '));
                }

                Rtl::CopyMemory(token, pStart, charCount * sizeof(WCHAR));
                token[charCount] = L'\0';

                tokenArray[index++] = token;
                pStart              = p + 1;
            }

            if (*p == L'\0')
            {
                break;
            }

            p++;
        }

        *Tokens = tokenArray;
        *Count  = tokenCount;
        return TRUE;
    }

    NODISCARD LONG strncmp(PCWSTR  String1,
                           PCWSTR  String2,
                           BOOLEAN CaseInsensitive)
    {
        if (!CaseInsensitive) 
        {
            while (*String1 && *String2 && *String1 == *String2) 
            {
                String1++, String2++;
            }

            return *String1 - *String2;
        }

        else 
        {
            while (*String1 && *String2 && 
                   RtlUpperChar(*String1) == 
                   RtlUpperChar(*String2)) 
            {
                String1++, String2++;
            }

            return RtlUpperChar(*String1) - RtlUpperChar(*String2);
        }
    }
}