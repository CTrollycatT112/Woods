/*++

MODULE: AMD64 Cpu Management

AUTHOR: Trollycat

ABSTRACT: AMD64 Specific Code

--*/
#include "ke/amd64/amd64.hpp"

STATIC KDESCRIPTOR_TABLE     DescriptorTable;
STATIC KDESCRIPTOR_TABLE_PTR DescriptorTablePtr;

namespace Ki
{
    VOID
    InitializeGdt()
    {
        //
        // CR0
        // ENABLE (EM, MP)
        //
        ULONG64 cr0 = Ke386ReadCr0();
        cr0 &= ~CR0_EM;
        cr0 |= CR0_MP;
        Ke386WriteCr0(cr0);

        //
        // CR4
        // ENABLE (OSFXSR, OSXMMEXCPT)
        //
        ULONG64 Cr4 = Ke386ReadCr4();
        Cr4 |= (CR4_OSFXSR | CR4_OSXMMEXCPT);
        Ke386WriteCr4(Cr4);

        //
        // KERNEL CODE
        //
        DescriptorTable.Entries[INDEX_GDT_KERNEL_CODE].Access      = GDT_KERNEL_CODE_ACCESS;
        DescriptorTable.Entries[INDEX_GDT_KERNEL_CODE].Granularity = D_GRAN_LONG;

        //
        // KERNEL DATA
        //
        DescriptorTable.Entries[INDEX_GDT_KERNEL_DATA].Access      = GDT_KERNEL_DATA_ACCESS;
       
        //
        // USER DATA
        //
        DescriptorTable.Entries[INDEX_GDT_USER_DATA].Access        = GDT_USER_DATA_ACCESS;
       
        //
        // USER CODE
        //
        DescriptorTable.Entries[INDEX_GDT_USER_CODE].Access        = GDT_USER_CODE_ACCESS;
        DescriptorTable.Entries[INDEX_GDT_USER_CODE].Granularity   = D_GRAN_LONG;

        //
        // TSS
        //
        DescriptorTable.Tss.Length = sizeof(KTASK_DESCRIPTOR_TABLE_DESCRIPTOR);
        DescriptorTable.Tss.Flags0 = (TSS_FLAGS0_PRESENT | SEGMENT_TYPE_TSS);

        DescriptorTablePtr.Limit = sizeof(KDESCRIPTOR_TABLE) - 1;
        DescriptorTablePtr.Base  = (ULONG64)&DescriptorTable;

        Ke386Lgdt(&DescriptorTablePtr);
        KiFlushGdt();
    }
} // namespace Ki