/*++

MODULE: Page frame number database

AUTHOR: Trollycat

ABSTRACT: Tracks page frame entries,
          manages page allocation data,
          and supports physical memory keeping

--*/
#include "mm/mi.hpp"

#include "ke/amd64/amd64.hpp"

PMMPFN     MmPfnDatabase           = NULL;
ULONG64    MmPfnCount              = 0;
KSPIN_LOCK PfnDatabaseLock;
MMPFN_LIST MmZeroedPageListHead;
MMPFN_LIST MmFreePageListHead;

namespace Mm
{
    VOID
    ChangePfnVaType(PMMPFN     Pfn,
                    MM_VA_TYPE Type)
    {
        if (Pfn->VaType == Type)
        {
            return;
        }

        if (Pfn->VaType == MmTypeFree)
        {
            RemovePfnListEntry(&MmFreePageListHead,Pfn);
        }

        if (Pfn->VaType == MmTypeZeroed)
        {
            RemovePfnListEntry(&MmZeroedPageListHead, Pfn);
        }

        if (Type == MmTypeFree)
        {
            InsertPfnListEntry(&MmFreePageListHead, Pfn);
        }

        if (Type == MmTypeZeroed)
        {
            InsertPfnListEntry(&MmZeroedPageListHead,Pfn);
        }

        Pfn->VaType = Type;
    }

    VOID
    ChangePfnRangeVaType(ULONG64 Address,
                         ULONG64 Length,
                         MM_VA_TYPE Type)
    {
        ULONG64 Start      = Address / PAGE_SIZE;
        ULONG64 End        = (Address + Length) / PAGE_SIZE;
        KIRQL PreviousIrql = 0;

        Ke::AcquireSpinLock(&PfnDatabaseLock, 
                        &PreviousIrql);

        for (ULONG64 Page = Start; Page < End; Page++) 
        {
            ChangePfnVaType(&MmPfnDatabase[Page], Type);
        }

        Ke::ReleaseSpinLock(&PfnDatabaseLock, PreviousIrql);

    }

    NODISCARD
    PMMPFN
    ReferencePfnByAddress(ULONG64 Address)
    {
        KIRQL PreviousIrql = 0;
        Ke::AcquireSpinLock(&PfnDatabaseLock, &PreviousIrql);

        ULONG64 PageFrameNumber = Address / PAGE_SIZE;
        PMMPFN  Res             = &MmPfnDatabase[PageFrameNumber];

        Ke::ReleaseSpinLock(&PfnDatabaseLock, PreviousIrql);
        return Res;
    }

    NODISCARD
    PMMPFN
    ReferencePfnByVaType(MM_VA_TYPE Type)
    {
        PMMPFN Pfn           = NULL;

        if (Type == MmTypeFree)
        {
            Pfn = PopPfnListEntry(&MmFreePageListHead);

            if (Pfn == NULL)
            {
                Pfn = ReferencePfnByVaType(MmTypeZeroed);
            }
        }

        if (Type == MmTypeZeroed)
        {
            Pfn = PopPfnListEntry(&MmZeroedPageListHead);

            if (Pfn == NULL)
            {
                Pfn = PopPfnListEntry(&MmZeroedPageListHead);
            }
        }

        return Pfn;
    }
} // namespace Mm