/*++

MODULE: Assert handler

AUTHOR: Trollycat

ABSTRACT: Runtime library assertion failure

--*/
#include "rtl/rtl.hpp"

namespace Rtl
{
    HTAPI
    VOID
    AssertFailed(PCSTR Expression,
                 PCSTR File,
                 ULONG Line,
                 PCSTR Message)
    {
        Print("*** ASSERTION FAILED: %s", Expression);
        Print("*** FILE: %s, LINE: %u", File, Line);

        if (Message != NULL)
        {
            Print("*** MESSAGE: %s", Message);
        }

        while (1)
        {
            __asm__ volatile("cli; hlt");
        }
    }

    HTAPI
    VOID
    AssertFailed(PCSTR Expression,
                 PCSTR File,
                 ULONG Line,
                 PCWSTR Message)
    {
        Print("*** ASSERTION FAILED: %s", Expression);
        Print("*** FILE: %s, LINE: %u", File, Line);

        if (Message != NULL)
        {
            Print(L"*** MESSAGE: %S", Message);
        }
        while (1)
        {
            __asm__ volatile("cli; hlt");
        }
    }
} // namespace Rtl