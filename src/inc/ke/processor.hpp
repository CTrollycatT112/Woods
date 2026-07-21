/*++

MODULE: CPU PROCESSORS

AUTHOR: Trollycat

ABSTRACT: A CPU has multiple 'cores',
          Otherwise known as 'processors',
          This file handles events related to processors.

--*/
#include "ke/amd64/amd64.hpp"

namespace Ki
{
    /*++

    ROUTINE: InitializePrcb

    DESCRIPTION: Initialize a basic process control block

    ARGUMENTS: Processor -
                Process control block object to fill

    RETURNS: VOID

    --*/
    VOID
    InitializePrcb(PKPRCB Processor);
} // namespace Ki

namespace Ke
{
    /*++

    ROUTINE: QueryCurrentProcessor

    DESCRIPTION: Looks up the structure of the active core

    ARGUMENTS: N/A

    RETURNS: Pointer to the KPRCB structure

    --*/
    INLINE
    PKPRCB
    QueryCurrentProcessor()
    {
        PKPRCB Prcb;
        __asm__ volatile("mov %%gs:0, %0" : "=r"(Prcb));
        return Prcb;
    }
} // namespace Ke