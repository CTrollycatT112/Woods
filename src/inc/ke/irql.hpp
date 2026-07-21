/*++

MODULE: Interrupt request levels

AUTHOR: Trollycat

ABSTRACT: NT interrupt request level module

--*/
#pragma once

#include "ke/amd64/amd64.hpp"

namespace Ke
{
    /*++

    ROUTINE: GetCurrentIrql

    DESCRIPTION: Reads the hardware interrupt request level

    ARGUMENTS: N/A

    RETURNS: The current KIRQL value

    --*/
    INLINE
    KIRQL
    GetCurrentIrql()
    {
        KIRQL Value;
        __asm__ volatile("mov %%cr8, %0" : "=r"(Value));
        return Value;
    }

    
    /*++

    ROUTINE: RaiseIrql

    DESCRIPTION: Raises the hardware interrupt priority to a higher level

    ARGUMENTS: NewIrql - The priority level,
            OldIrql - Pointer to the previous level

    RETURNS: N/A

    --*/
    INLINE
    VOID
    RaiseIrql(KIRQL NewIrql, PKIRQL OldIrql)
    {
        *OldIrql = GetCurrentIrql();
        if (*OldIrql > NewIrql)
        {
            return;
        }
        __asm__ volatile("mov %0, %%cr8" :: "r"(NewIrql) : "memory");
    }

    /*++

    ROUTINE: LowerIrql

    DESCRIPTION: Lowers hardware interrupt to a safer level

    ARGUMENTS: NewIrql - The target level

    RETURNS: N/A

    --*/
    INLINE
    VOID
    LowerIrql(KIRQL NewIrql)
    {
        if (NewIrql > GetCurrentIrql())
        {
            return;
        }
        __asm__ volatile("mov %0, %%cr8" :: "r"(NewIrql) : "memory");
    }
}