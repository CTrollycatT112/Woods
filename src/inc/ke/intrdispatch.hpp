/*++

MODULE: Interrupt Dispatcher

AUTHOR: Trollycat

ABSTRACT: Dispatcher module that handles interrupts

--*/
#include "ke/amd64/amd64.hpp"

namespace Ki
{
    /*++

    ROUTINE: DispatchHardware

    DESCRIPTION: Routes physical hardware interrupts to device handlers

    ARGUMENTS: TrapFrame - Pointer to the saved processor state

    RETURNS: VOID

    --*/
    EXTERN_C
    VOID
    DispatchHardware(PKTRAP_FRAME TrapFrame);

    /*++

    ROUTINE: DispatchException

    DESCRIPTION: Catches processor errors and halts

    ARGUMENTS: TrapFrame - Pointer to the saved processor state

    RETURNS: VOID

    --*/
    EXTERN_C
    VOID
    DispatchException(PKTRAP_FRAME TrapFrame);
} // namespace Ki