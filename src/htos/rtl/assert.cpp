/*++

MODULE: Assert handler

AUTHOR: Trollycat

ABSTRACT: Runtime library assertion failure

--*/
#include "ke/bug.hpp"
#include "rtl/rtl.hpp"

#include "bugcodes.hpp"

namespace Rtl
{
    HTAPI
    VOID
    AssertFailed(PCSTR Expression,
                 PCSTR File,
                 ULONG Line,
                 PCSTR Message)
    {
        Ke::BugCheckEx(KMODE_EXCEPTION_NOT_HANDLED, 
                       reinterpret_cast<ULONG64>(Expression), 
                       reinterpret_cast<ULONG64>(File), 
                       Line, 
                       reinterpret_cast<ULONG64>(Message));
    }

    HTAPI
    VOID
    AssertFailed(PCSTR Expression,
                 PCSTR File,
                 ULONG Line,
                 PCWSTR Message)
    {
        Ke::BugCheckEx(KMODE_EXCEPTION_NOT_HANDLED, 
                       reinterpret_cast<ULONG64>(Expression), 
                       reinterpret_cast<ULONG64>(File), 
                       Line, 
                       reinterpret_cast<ULONG64>(Message));
    }
} // namespace Rtl