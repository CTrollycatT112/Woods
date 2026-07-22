/*++

MODULE: AMD64 Cpu Management

AUTHOR: Trollycat

ABSTRACT: AMD64 Specific Code

--*/
#include "htdef.hpp"
#include "ke/amd64/amd64.hpp"
#include "ke/asm386.hpp"


#include "rtl/rtl.hpp"

STATIC KDESCRIPTOR_TABLE     DescriptorTable;
STATIC KDESCRIPTOR_TABLE_PTR DescriptorTablePtr;
STATIC KTASK_STATE           TaskState;

namespace Ki
{
    CODESEG(".init")
    VOID
    InitializeGdt()
    {
        //
        // CR0
        // ENABLE (EM, MP)
        //
        ULONG64 cr0 = Ke::A386ReadCr0();
        cr0 &= ~CR0_EM;
        cr0 |= CR0_MP;
        Ke::A386WriteCr0(cr0);

        //
        // CR4
        // ENABLE (OSFXSR, OSXMMEXCPT)
        //
        ULONG64 Cr4 = Ke::A386ReadCr4();
        Cr4 |= (CR4_OSFXSR | CR4_OSXMMEXCPT);
        Ke::A386WriteCr4(Cr4);

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
        union
        {
            ULONG64 Address;
            struct
            {
                USHORT Low;
                UCHAR  Mid;
                UCHAR  High;
                ULONG  Upper32;
            } Parts;
        } TssSplit;

        Rtl::ZeroMemory(&TaskState,
                        sizeof(KTASK_STATE));
        TaskState.IopbOffset = sizeof(KTASK_STATE);

        ULONG64 TssBase = (ULONG64)&TaskState;

        DescriptorTable.Tss.Length      = (USHORT)(sizeof(KTASK_STATE) - 1);
        DescriptorTable.Tss.Flags0      = (TSS_FLAGS0_PRESENT | SEGMENT_TYPE_TSS);
        DescriptorTable.Tss.Flags1      = 0;

        TssSplit.Address = TssBase;

        DescriptorTable.Tss.BaseLow     = TssSplit.Parts.Low;
        DescriptorTable.Tss.BaseMid     = TssSplit.Parts.Mid;
        DescriptorTable.Tss.BaseHigh    = TssSplit.Parts.High;
        DescriptorTable.Tss.BaseUpper32 = TssSplit.Parts.Upper32;
        DescriptorTable.Tss.Reserved    = 0;

        //
        // LOAD AND FLUSH
        //
        DescriptorTablePtr.Limit = sizeof(KDESCRIPTOR_TABLE) - 1;
        DescriptorTablePtr.Base  = (ULONG64)&DescriptorTable;

        Ke::A386Lgdt(&DescriptorTablePtr);
        KiFlushGdt();

        Ke::A386SetTr(OFFSETOF(KDESCRIPTOR_TABLE, Tss));

    }
} // namespace Ki