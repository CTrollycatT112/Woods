/*++

MODULE: Interrupt Descriptor Table

AUTHOR: Trollycat

ABSTRACT: Loads and fills the interrupt descriptor table

--*/
#include "hal/hal.hpp"
#include "ke/amd64/amd64.hpp"

STATIC KINTERRUPT_DESCRIPTOR     InterruptDescriptorTable[256];
STATIC KINTERRUPT_DESCRIPTOR_PTR InterruptDescriptorTablePtr;

EXTERN ULONG64 KxIntHandlerTable[];

namespace Hal
{
    CODESEG(".init")
    VOID LoadIdt()
    {
        __asm__ volatile (
            "lidt %0"
            :
            : "m" (InterruptDescriptorTablePtr)
            : "memory"
        );
    }

    CODESEG(".init")
    VOID
    InitializeIdt()
    {
        for (INT I = 0; I < 256; I++)
        {
            InterruptDescriptorTable[I].OffsetLow  =  (USHORT)KxIntHandlerTable[I];
            InterruptDescriptorTable[I].OffsetMid  =  (USHORT)(KxIntHandlerTable[I] >> 16);
            InterruptDescriptorTable[I].OffsetHigh =  (ULONG32)(KxIntHandlerTable[I] >> 32);
            InterruptDescriptorTable[I].Type =        SEGMENT_TYPE_INTERRUPT_GATE;
            InterruptDescriptorTable[I].SegmentSelector = GDT_KERNEL_CODE;
            InterruptDescriptorTable[I].PrivilegeLevel  = 0;
            InterruptDescriptorTable[I].Present         = 1;
        }

        InterruptDescriptorTable[3].PrivilegeLevel = 3;
        InterruptDescriptorTable[0x29].PrivilegeLevel = 3;
        InterruptDescriptorTable[0x2C].PrivilegeLevel = 3;

        InterruptDescriptorTablePtr.Address = reinterpret_cast<ULONG64>(InterruptDescriptorTable);
        InterruptDescriptorTablePtr.Size = sizeof(InterruptDescriptorTable) - 1;

        LoadIdt();

    }
} // namespace Hal