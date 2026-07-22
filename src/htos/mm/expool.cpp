/*++

MODULE: Heap pool allocator

AUTHOR: Trollycat

ABSTRACT: Manages kernel heap pool allocation,
          tracks pool blocks and free entries

--*/
#include "htdef.hpp"
#include "ke/spinlock.hpp"
#include "mm/mi.hpp"
#include "mm/mm.hpp"

#include "rtl/rtl.hpp"

#include <tlsf.h>

// TODO:
// THIS IS A VERY MINIMAL POOL ALLOCATOR BY NSG650
// JUST USING THIS TO GET THE CODE WORKING
// LATER THIS WILL BE COMPLETELY REFACTORED
// THIS CODE CONTAINS SECURITY BUGS
// AND SHORTCUTS
// ALTHOUGH IT DOES WORK, AS LONG AS YOU'RE SAFE

STATIC tlsf_t tlsf_g;

STATIC ULONG64 MmPoolHint       = MI_NON_PAGED_POOL;
STATIC ULONG64 MmPoolBase       = MI_NON_PAGED_POOL;

KSPIN_LOCK     MmPoolLock;

PVOID tlsf_resize(tlsf_t* t, size_t req_size)
{
    UNREFERENCED_PARAMETER(t);
    ULONG64 MaximumPoolSize = (MmPfnCount * PAGE_SIZE) / 2;

    ULONG64 CurrentSize     = MmPoolHint - MmPoolBase;

    if (req_size <= CurrentSize)
    {
        return (PVOID)MmPoolBase;
    }

    ULONG64 Delta = ALIGN_UP(req_size - CurrentSize, PAGE_SIZE);

    if (MmPoolHint + Delta > MmPoolBase + MaximumPoolSize)
    {
        return NULL;
    }

    for (ULONG64 i = 0; i < Delta; i += PAGE_SIZE)
    {
        ULONG64 Page = Mm::AllocatePhysical(MmTypeNonPagedPool);
        ASSERTMSG("FAILED TO RESIZE: PAGE IS ZERO", Page != 0);

        ASSERTMSG("FAILED TO RESIZE: MAPPAGE FAILED...",
                  Mi::MapPage(MiKernelAddressSpace,
                              MmPoolHint + i,
                              Page,
                              TRUE,
                              FALSE,
                              TRUE));
    }

    MmPoolHint += Delta;
    return (PVOID)MmPoolBase;
}

namespace Mm
{
    NODISCARD
    PVOID 
    AllocatePoolWithTag(POOL_TYPE Type,
                        ULONG64   Length,
                        ULONG32   Tag)
    {
        //
        // Length cannot be 0
        // This will be an instant crash
        // This is considered a 'BAD CALLER'
        //
        ASSERTMSG("AllocatePoolWithTag: BAD POOL CALLER, LENGTH IS ZERO",
                  Length != 0);

        //
        // ALIGN THE LENGTH
        //
        Length =  ALIGN_UP(Length, 0x10);
        Length += sizeof(POOL_HEADER);

        //
        // SPIN LOCK
        //
        KIRQL PreviousIrql = 0;
        Ke::AcquireSpinLock(&MmPoolLock,&PreviousIrql);

        //
        // ACTUAL ALLOCATION
        //
        PPOOL_HEADER Allocation = (PPOOL_HEADER)tlsf_aalloc(&tlsf_g, 0x10, Length);

        //
        // ALLOCATION FAILED
        // JUST RETURN NULL
        // WILL BE CALLERS JOB TO CHECK THE POINTER
        // NODISCARD ENFORCES THIS SOMEWHAT..
        //
        if (Allocation == NULL)
        {
            Ke::ReleaseSpinLock(&MmPoolLock, PreviousIrql);
            return NULL;
        }

        //
        // IN THIS CASE
        // THIS ALLOCATION WAS SUCCESSFUL
        //
        Allocation->Tag     = Tag;
        Allocation->Length  = Length;
        Allocation->Type    = static_cast<ULONG32>(Type);

        Ke::ReleaseSpinLock(&MmPoolLock,PreviousIrql);

        //
        // EXECUTABLE FLAG
        //
        if (Type == NonPagedPoolExecute || Type == NonPagedPoolZeroedExecute)
        {
            for (ULONG64 Page = 0; Page < ALIGN_UP(Length, PAGE_SIZE); Page += PAGE_SIZE)
            {
                ASSERT(Mi::FlagPage(reinterpret_cast<PPMLE>(MiKernelAddressSpace), 
                                    reinterpret_cast<ULONG64>(Allocation) + Page, 
                                    TRUE, TRUE, FALSE, TRUE));
            }
        }

        //
        // IF A ZEROED POOL BLOCK WAS REQUESTED
        // ZERO THE MEMORY
        //
        if (Type == NonPagedPoolZeroed || Type == NonPagedPoolZeroedExecute)
        {
            Rtl::ZeroMemory((PVOID)((ULONG64)Allocation + sizeof(POOL_HEADER)), Length - sizeof(POOL_HEADER));
        }

        return reinterpret_cast<PVOID>(Allocation + 1);
    }

    VOID
    FreePool(PVOID   Pool,
             ULONG32 Tag)
    {
        ASSERTMSG("FreePool: POOL ADDRESS IS NULL", Pool != nullptr);
        ASSERTMSG("FreePool: POOL BUFFER CORRUPTION", (reinterpret_cast<ULONG_PTR>(Pool) % 0x10) == 0);

        PPOOL_HEADER Allocation = reinterpret_cast<PPOOL_HEADER>(
            reinterpret_cast<ULONG_PTR>(Pool) - sizeof(POOL_HEADER)
        );

        ASSERTMSG("FAILED TO FREE POOL: BAD TAG AND CALLER",
                  Allocation->Tag == Tag);

        Allocation->Tag = MAKE_TAG('D', 'E', 'A', 'D');

        if (Allocation->Type == NonPagedPoolExecute ||
            Allocation->Type == NonPagedPoolZeroedExecute)
        {
            for (ULONG64 Page = 0; Page < ALIGN_UP(Allocation->Length, PAGE_SIZE);
                 Page += PAGE_SIZE)
            {
                ASSERT(Mi::FlagPage(MiKernelAddressSpace,
                             (ULONG64)Allocation + Page,
                             TRUE,
                             TRUE,
                             FALSE,
                             FALSE));
            }
        }

        KIRQL PreviousIrql = 0;
        Ke::AcquireSpinLock(&MmPoolLock,&PreviousIrql);
        tlsf_free(&tlsf_g, Allocation);
        Ke::ReleaseSpinLock(&MmPoolLock,PreviousIrql);
    }
} // namespace Mm