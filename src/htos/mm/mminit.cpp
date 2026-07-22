/*++

MODULE: Memory manager Initialization

AUTHOR: Trollycat

ABSTRACT: Initializes the memory manager,
          After multi-threading there will be 2 parts (as per NT design),
          for now there is 1 stage to it

--*/

#include "ke/amd64/amd64.hpp"
#include "limine.h"
#include "mm/mi.hpp"
#include "mm/mm.hpp"
#include "rtl/rtl.hpp"

STATIC
VOLATILE
struct limine_memmap_request memmap_request =
{
    .id         = LIMINE_MEMMAP_REQUEST_ID,
    .revision   = 0,
    .response   = NULL
};

STATIC
VOLATILE
struct limine_hhdm_request hhdm_request =
{
    .id       = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

STATIC
VOLATILE
struct limine_executable_file_request kernel_file_request =
{
    .id       = LIMINE_EXECUTABLE_FILE_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

ULONG64 MmPhysicalOffset     = 0;
PPMLE   MiKernelAddressSpace = NULL;
ULONG64 MmKernelBase         = 0;
ULONG64 MmKernelEnd          = 0;
ULONG64 KdKernelBase         = 0;
ULONG64 KdKernelSize         = 0;

namespace Mm
{
    VOID
    InitializeMemoryManager()
    {
        struct limine_memmap_entry **memmap = memmap_request.response->entries;
        SIZE_T memmap_entries = memmap_request.response->entry_count;

        MmPhysicalOffset = hhdm_request.response->offset;
        ULONG64 DatabaseLength = 0;
        ULONG64 DatabasePhysical = 0;
        ULONG64 HighestPfn = 0;

        for (ULONG64 i = 0; i < memmap_entries; i++)
        {
            if (memmap[i]->type != LIMINE_MEMMAP_USABLE)
            {
                continue;
            }

            if (memmap[i]->length < PAGE_SIZE)
            {
                continue;
            }

            if (HighestPfn < memmap[i]->base + memmap[i]->length)
            {
                HighestPfn = memmap[i]->base + memmap[i]->length;
            }

            MmPfnCount += memmap[i]->length / PAGE_SIZE;
        }

        HighestPfn /= PAGE_SIZE;

        DatabaseLength = ALIGN_UP(HighestPfn * sizeof(MMPFN), PAGE_SIZE);

        ULONG64 CurrentBestSize = 0xffffffffffffffff;

        for (ULONG64 i = 0; i < memmap_entries; i++)
        {
            if (memmap[i]->type != LIMINE_MEMMAP_USABLE)
            {
                continue;
            }

            if (memmap[i]->length < DatabaseLength)
            {
                continue;
            }

            if (memmap[i]->length < CurrentBestSize)
            {
                CurrentBestSize = memmap[i]->length;
                DatabasePhysical = memmap[i]->base;
            }
        }

        MmPfnDatabase = (PMMPFN)(DatabasePhysical + MmPhysicalOffset);
        Rtl::ZeroMemory(MmPfnDatabase, DatabaseLength);

        MmFreePageListHead.Total = 0;
        MmFreePageListHead.Type = MmTypeFree;

        MmZeroedPageListHead.Total = 0;
        MmZeroedPageListHead.Type = MmTypeZeroed;

        for (ULONG64 i = 0; i < memmap_entries; i++)
        {
            if (memmap[i]->type != LIMINE_MEMMAP_USABLE)
            {
                continue;
            }

            if (memmap[i]->length < PAGE_SIZE)
            {
                continue;
            }

            ULONG64 Base = memmap[i]->base;
            ULONG64 Length = memmap[i]->length;

            if (Base == DatabasePhysical)
            {
                Base += DatabaseLength;
                Length -= DatabaseLength;
            }

            Base = ALIGN_UP(Base, PAGE_SIZE);

            ULONG64 End = Base + Length;

            for (ULONG64 Page = Base; Page < End; Page += PAGE_SIZE)
            {
                MmPfnDatabase[Page / PAGE_SIZE].Long = 0;
                MmPfnDatabase[Page / PAGE_SIZE].PageFrameNumber = Page / PAGE_SIZE;
                MmPfnDatabase[Page / PAGE_SIZE].VaType = MmTypeFree;
                InsertPfnListEntry(&MmFreePageListHead, &MmPfnDatabase[Page / PAGE_SIZE]);
            }
        }

        ULONG64 NewDirectoryPhysical = AllocatePhysical(MmTypePageTable);
        MiKernelAddressSpace = (PPMLE)(NewDirectoryPhysical + MmPhysicalOffset);
        Rtl::ZeroMemory(MiKernelAddressSpace, PAGE_SIZE);

        for (ULONG64 i = 256; i < 512; i++)
        {
            ASSERT(
                Mi::PageTableToEntry(MiKernelAddressSpace, i, TRUE)
            );
        }

        for (ULONG64 Page = 0x1000; Page < 4096ULL * 1024 * 1024; Page += 0x200000)
        {
            ASSERT(
                Mi::MapLargePage(MiKernelAddressSpace,
                                 Page,
                                 Page,
                                 TRUE,
                                 FALSE,
                                 FALSE)
            );

            ASSERT(
                Mi::MapLargePage(MiKernelAddressSpace,
                                 Page + MmPhysicalOffset,
                                 Page,
                                 TRUE,
                                 FALSE,
                                 FALSE)
            );
        }

        for (ULONG64 i = 0; i < memmap_entries; i++)
        {
            ULONG64 Base   = memmap[i]->base;
            ULONG64 Length = memmap[i]->length;
            ULONG64 Top    = Base + Length;

            if (Base < 0x100000000)
            {
                Base = 0x100000000;
            }

            if (Base > Top)
            {
                continue;
            }

            ULONG64 AlignedBase   = ALIGN_DOWN(Base, 0x200000);
            ULONG64 AlignedTop    = ALIGN_UP(Top, 0x200000);
            ULONG64 AlignedLength = AlignedTop - AlignedBase;

            for (ULONG64 Page = 0; Page < AlignedLength; Page += 0x200000)
            {
                ASSERT(
                    Mi::MapLargePage(MiKernelAddressSpace,
                                     Page + AlignedBase,
                                     Page + AlignedBase,
                                     TRUE,
                                     FALSE,
                                     FALSE)
                );

                ASSERT(
                    Mi::MapLargePage(MiKernelAddressSpace,
                                     Page + MmPhysicalOffset + AlignedBase,
                                     Page + AlignedBase,
                                     TRUE,
                                     FALSE,
                                     FALSE)
                );
            }
        }

        ULONG64 KernelBaseVirtual  = 0xffffffff80000000;
        ULONG64 KernelBasePhysical = 0;

        for (ULONG64 i = 0; i < memmap_entries; i++)
        {
            if (memmap[i]->type == LIMINE_MEMMAP_EXECUTABLE_AND_MODULES)
            {
                KernelBasePhysical = memmap[i]->base;
                break;
            }
        }

        MmKernelBase = KernelBaseVirtual;
        MmKernelEnd = MmKernelBase + (64 * 1024 * 1024);

        for (ULONG64 Page = 0; Page < (64 * 1024 * 1024); Page += PAGE_SIZE)
        {
            ASSERT(
                Mi::MapPage(MiKernelAddressSpace,
                            KernelBaseVirtual + Page,
                            KernelBasePhysical + Page,
                            TRUE,
                            FALSE,
                            TRUE)
            );
        }

        for (ULONG64 Page = 0; Page < ALIGN_UP(DatabaseLength, PAGE_SIZE); Page += PAGE_SIZE)
        {
            ASSERT(
                Mi::MapPage(MiKernelAddressSpace,
                            MI_PFN_DATABASE + Page,
                            DatabasePhysical + Page,
                            TRUE,
                            FALSE,
                            FALSE)
            );
        }

        ULONG64 WorkingSet = AllocatePhysical(MmTypeWorkingSetList);
        Rtl::ZeroMemory(reinterpret_cast<PVOID>(WorkingSet + MmPhysicalOffset), PAGE_SIZE);

        ASSERT(
            Mi::MapPage(MiKernelAddressSpace,
                        MI_WORKING_SET_LIST,
                        WorkingSet,
                        TRUE,
                        FALSE,
                        FALSE)
        );

        MiSetAddressSpace(NewDirectoryPhysical);

        MmPfnDatabase = reinterpret_cast<PMMPFN>(MI_PFN_DATABASE);

        KdKernelBase = KernelBaseVirtual;
        KdKernelSize = (128 * 1024 * 1024);
    }

    VOID
    FreeInitCode()
    {
        ULONG64 InitStart = reinterpret_cast<ULONG64>(__InitStart);
        ULONG64 InitEnd   = reinterpret_cast<ULONG64>(__InitEnd);

        InitStart = ALIGN_DOWN(InitStart, PAGE_SIZE);
        InitEnd   = ALIGN_UP(InitEnd, PAGE_SIZE);

        ULONG64 RawStart = reinterpret_cast<ULONG64>(__InitStart);
        ULONG64 RawEnd   = reinterpret_cast<ULONG64>(__InitEnd);
        ULONG64 RawSize  = RawEnd - RawStart;

        Rtl::KdPrint(L"RECLAIMING SECTION .init: %lu bytes", RawSize);

        for (ULONG64 Va = InitStart; Va < InitEnd; Va += PAGE_SIZE)
        {
            if (!IsAddressValid(Va))
            {
                continue;
            }

            ULONG64 PhysicalAddress = Mi::GetPhysicalForVirtual(
                (PPMLE)MiGetAddressSpace(), Va
            );

            if (PhysicalAddress != 0)
            {
                Mi::UnmapPage(
                    (PPMLE)MiGetAddressSpace(),Va
                );

                FlushAddress(Va);

                FreePhysical(PhysicalAddress);
            }
        }
    }
} // namespace Mm