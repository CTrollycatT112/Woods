/*++

MODULE: String utils

AUTHOR: Trollycat

ABSTRACT: Bare bones string functions so flanterm will work

--*/
#include "htbase.hpp"

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
}