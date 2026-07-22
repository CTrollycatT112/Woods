/*++

MODULE: Physical memory allocator

AUTHOR: Trollycat

ABSTRACT:  Physical memory allocation (API),
           This is mostly the API for the physical allocator,
           Internally, the phys allocator is the PFN

--*/
#include "mm/mi.hpp"
#include "mm/mm.hpp"

namespace Mm
{
    BOOLEAN MiForceHighAlloc = FALSE;

    NODISCARD
    ULONG64
    AllocatePhysicalPfn(MM_VA_TYPE Reason,
                        ULONG64    MinimumPhysAddress,
                        PMMPFN*    Pfn)
    {
        if (MmFreePageListHead.Total == 0 || MmFreePageListHead.Head == NULL)
        {
            if (Pfn)
            {
                *Pfn = NULL;
            }
            return 0;
        }

        PMMPFN CurrentPfn = NULL;
        PLIST_ENTRY ListHead = MmFreePageListHead.Head;
        PLIST_ENTRY NextEntry = ListHead;

        do
        {
            PMMPFN CandidatePfn = CONTAINING_RECORD(NextEntry, MMPFN, Link);
            ULONG64 PhysicalAddress = CandidatePfn->PageFrameNumber * PAGE_SIZE;

            if (PhysicalAddress >= MinimumPhysAddress)
            {
                CurrentPfn = CandidatePfn;

                if (MmFreePageListHead.Total == 1)
                {
                    MmFreePageListHead.Head = NULL;
                }
                else
                {
                    if (MmFreePageListHead.Head == &CurrentPfn->Link)
                    {
                        MmFreePageListHead.Head = CurrentPfn->Link.Flink;
                    }
                    KeRemoveList(&CurrentPfn->Link);
                }

                MmFreePageListHead.Total--;
                break;
            }

            NextEntry = NextEntry->Flink;
        } while (NextEntry != ListHead);

        if (CurrentPfn == NULL)
        {
            if (Pfn)
            {
                *Pfn = NULL;
            }
            return 0;
        }

        ChangePfnVaType(CurrentPfn, Reason);
        CurrentPfn->ReferenceCount++;

        if (Pfn)
        {
            *Pfn = CurrentPfn;
        }

        return CurrentPfn->PageFrameNumber * PAGE_SIZE;
    }


    NODISCARD
    ULONG64
    AllocatePhysical(MM_VA_TYPE Reason,
                    ULONG64     MinimumPhysAddress)
    {
        ULONG64 EffectiveMinimum = (MiForceHighAlloc && MinimumPhysAddress < 0x200000) 
                                   ? 0x200000 
                                   : MinimumPhysAddress;

        return AllocatePhysicalPfn(Reason, EffectiveMinimum, NULL);
    }

    NODISCARD
    ULONG64
    AllocatePhysicalZeroedPfn(MM_VA_TYPE Reason,
                              PMMPFN*    Pfn)
    {
        PMMPFN Pfn1 = ReferencePfnByVaType(MmTypeZeroed);
    
        if (Pfn1 == NULL)
        {
            if (Pfn)
            {
                *Pfn = NULL;
            }

            return 0;
        }

        ChangePfnVaType(Pfn1, Reason);

        Pfn1->ReferenceCount++;

        if (Pfn)
        {
            *Pfn = Pfn1;
        }

        return Pfn1->PageFrameNumber * PAGE_SIZE;
    }

    NODISCARD
    ULONG64
    AllocatePhysicalZeroed(MM_VA_TYPE Reason)
    {
        return AllocatePhysicalZeroedPfn(Reason, NULL);
    }

    VOID
    FreePhysical(ULONG64 Address)
    {
        PMMPFN Pfn = ReferencePfnByAddress(Address);
        Pfn->ReferenceCount--;

        if (Pfn->ReferenceCount == 0)
        {
            ChangePfnVaType(Pfn,MmTypeFree);
        }
    }

    NODISCARD
    ULONG64
    AllocateForHardwareDma()
    {
        return AllocatePhysical(MmTypeDma);
    }

    VOID
    FreeDmaAllocation(ULONG64 Address)
    {
        return FreePhysical(Address);
    }
} // namespace Mm