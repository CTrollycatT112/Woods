/*++

MODULE: Ke386 Assembly Wrappers

AUTHOR: Trollycat

ABSTRACT: Simple __asm__ wrappers

--*/

#include "ke/amd64/amd64.hpp"

namespace Ke
{
    /*++

    ROUTINE: A386SetTr

    DESCRIPTION: Load task register with a selector

    ARGUMENTS: Selector - Segment index of TSS

    RETURNS: VOID

    --*/

    INLINE
    VOID
    A386SetTr(USHORT Selector)
    {
        __asm__ volatile("ltr %0" : : "r"(Selector));
    }

    /*++

    ROUTINE: A386GetTr

    DESCRIPTION: Get the current task register selector

    ARGUMENTS: N/A

    RETURNS: USHORT

    --*/

    NODISCARD
    INLINE
    USHORT
    A386GetTr()
    {
        USHORT Selector;
        __asm__ volatile("str %0" : "=r"(Selector));
        return Selector;
    }

    /*++

    ROUTINE: A386ReadCr0

    DESCRIPTION: Read the CR0 register

    ARGUMENTS: N/A

    RETURNS: ULONG64

    --*/
    NODISCARD
    INLINE
    ULONG64
    A386ReadCr0()
    {
        ULONG64 Value;
        __asm__ volatile("mov %%cr0, %0" : "=r"(Value));
        return Value;
    }

    /*++

    ROUTINE: A386WriteCr0

    DESCRIPTION: Write a value to CR0 register

    ARGUMENTS: Value - The new CR0 mask

    RETURNS: VOID

    --*/
    INLINE
    VOID
    A386WriteCr0(ULONG64 Value)
    {
        __asm__ volatile("mov %0, %%cr0" : : "r"(Value));
    }

    /*++

    ROUTINE: A386ReadCr4

    DESCRIPTION: Read the current CR4 register

    ARGUMENTS: N/A

    RETURNS: ULONG64

    --*/
    NODISCARD
    INLINE
    ULONG64
    A386ReadCr4(VOID)
    {
        ULONG64 Value;
        __asm__ volatile("mov %%cr4, %0" : "=r"(Value));
        return Value;
    }

    /*++

    ROUTINE: A386WriteCr4

    DESCRIPTION: Write a value to CR4

    ARGUMENTS: Value - The new CR4 mask

    RETURNS: N/A

    --*/
    INLINE
    VOID
    A386WriteCr4(ULONG64 Value)
    {
        __asm__ volatile("mov %0, %%cr4" : : "r"(Value));
    }

    /*++

    ROUTINE: A386Lgdt

    DESCRIPTION: Loads the GDT register

    ARGUMENTS: TablePtr - Pointer to KDESCRIPTOR_TABLE_PTR

    RETURNS: N/A

    --*/
    INLINE
    VOID
    A386Lgdt(PKDESCRIPTOR_TABLE_PTR TablePtr)
    {
        __asm__ volatile("lgdt (%0)" : : "r"(TablePtr) : "memory");
    }

}