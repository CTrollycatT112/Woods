// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Kernel initialization

AUTHOR: Trollycat

ABSTRACT: Kernel entry point

--*/
#include "htbase.hpp"

namespace Ki
{
    /*++

    ROUTINE: SystemStartup

    DESCRIPTION: Kernel entry point
    
    ARGUMENTS: N/A

    RETURNS: NORETURN

    --*/
    EXTERN_C
    NORETURN
    CODESEG(".text.startup")
    VOID SystemStartup()
    {
        // TODO: REMOVE INFINITE HALT HEHE
        for (;;)
        {
            asm volatile("hlt");
        }
    }
} // namespce Ki