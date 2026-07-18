// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: String utils

AUTHOR: Trollycat

ABSTRACT: Bare bones string functions so flanterm will work

--*/
#include "htbase.hpp"

EXTERN_C {

    ULONG64
    strlen(CONST CHAR* String)
    {
        ULONG64 I = 0;

        while (String[I])
        {
            I++;
        }

        return I;
    }
}