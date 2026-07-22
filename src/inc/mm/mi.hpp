/*++

MODULE: Internal memory management header

AUTHOR: Trollycat

ABSTRACT: Stores internal memory management structures,
          and functions

--*/
#pragma once

#include "htbase.hpp"

#include "rtl/rtl.hpp"

#include "ke/amd64/amd64.hpp"
#include "ke/spinlock.hpp"
#include "ke/asm386.hpp"

typedef union _CR3
{
    struct
    {
        ULONG64 Reserved1       : 3;
        ULONG64 WriteThrough    : 1;
        ULONG64 CacheDisable    : 1;
        ULONG64 Reserved2       : 7;
        ULONG64 PageTable       : 52;
    } Pcid0;

    struct
    {
        ULONG64 Pcid        : 12;
        ULONG64 PageTable   : 52;
    } Pcid1;

    ULONG64     Long;
} CR3, *PCR3;

typedef union _PMLE
{
    struct
    {
        ULONG64 Present         : 1;
        ULONG64 Write           : 1;
        ULONG64 User            : 1;
        ULONG64 WriteThough     : 1;
        ULONG64 CacheDisable    : 1;
        ULONG64 Accessed        : 1;
        ULONG64 Dirty           : 1;
        ULONG64 LargePage       : 1;
        ULONG64 Global          : 1;
        ULONG64 Avail1          : 3;
        ULONG64 PageFrameNumber : 36;
        ULONG64 Reserved1       : 4;
        ULONG64 Avail2          : 7;
        ULONG64 ProtectionKey   : 4;
        ULONG64 ExecuteDisable  : 1;
    };

    ULONG64     Long;
} PMLE, *PPMLE;

typedef struct _MMPFN
{
    union
    {
        struct
        {
            ULONG64 PageFrameNumber     : 37;
            ULONG64 VaType              : 8;
            ULONG64 ReferenceCount      : 16;
        };

        ULONG64     Long;
    };

    LIST_ENTRY Link;
} MMPFN, *PMMPFN;

typedef enum _VA_TYPE
{
    MmTypeFree,
    MmTypeZeroed,
    MmTypePageTable,
    MmTypeDma,
    MmTypeWorkingSetList,
    MmTypeProcessPrivate,
    MmTypeNonPagedPool,
    MmTypeSectionObject,
    MmTypeMaximum
} VA_TYPE;

typedef UCHAR MM_VA_TYPE;

typedef struct _MMPFN_LIST
{
    MM_VA_TYPE  Type;
    ULONG64     Total;
    KSPIN_LOCK  Lock;
    PLIST_ENTRY Head;
} MMPFN_LIST, *PMMPFN_LIST;

//
// VAD PERMISSIONS
//
#define MM_VAD_READ    (1 << 0)
#define MM_VAD_WRITE   (1 << 1)
#define MM_VAD_EXECUTE (1 << 2)
#define MM_VAD_COMMIT  (1 << 3)
#define MM_VAD_SECTION (1 << 4)

//
// VAD HELEPRS
//
#define VAD_RED    1
#define VAD_BLACK  0

#define MiIsVadRed(n)   ((n) != NULL && (n)->Rb    == VAD_RED)
#define MiIsVadBlack(n) ((n) == NULL || (n)->Rb    == VAD_BLACK)

//
// SECTION PERMISSIONS
//
#define SECTION_MAP_WRITE   1 << 1
#define SECTION_MAP_READ    1 << 0
#define SECTION_MAP_EXECUTE 1 << 2

typedef struct _MM_SECTION_PAGES
{
    ULONG64     PageCount;
    PMMPFN      Pages[];
} MM_SECTION_PAGES, *PMM_SECTION_PAGES;

typedef struct _MM_SECTION
{
    ULONG64 Length;
    ULONG   Flags;
    ULONG   Protection;

    union {
        PMM_SECTION_PAGES Pages;
        PVOID CachedFile;
    };

} MM_SECTION, *PMM_SECTION;

typedef struct _MM_VAD
{
    ULONG64 Start;
    ULONG64 End;

    union
    {
        struct
        {
            UCHAR Rb    : 1;
            UCHAR Type  : 7;
        };

        UCHAR TypeChar;
    };

    ULONG64     SelectionBase;
    PMM_SECTION SectionObject;

    struct _MM_VAD* Parent;
    struct _MM_VAD* Left;
    struct _MM_VAD* Right;
} MM_VAD, *PMM_VAD;

namespace Mi
{
    /*++

    ROUTINE: ProtToVadType

    DESCRIPTION: Converts protection flags,
                 Into a singl byte value for VAD tree

    ARGUMENTS: Read     - Enable or disable read permissions,
               Write    - Enable or disable write permissions,
               Execute  - Enable or disable execute permissions,
               Commit   - Mark the region as committed page space,
               Section  - Mark if the region is backed by a section

    RETURNS: UCHAR

    --*/
    INLINE
    UCHAR
    ProtToVadType(BOOLEAN Read,
                  BOOLEAN Write,
                  BOOLEAN Execute,
                  BOOLEAN Commit,
                  BOOLEAN Section)
    {
        return (Section << 4) |
               (Commit  << 3) |
               (Execute << 2) |
               (Write   << 1) |
               (Read    << 0);
    }
} // namespace Mi

//
// VAD PERMISSION MACRO'S
//
#define MiIsVadReadable(x)   (((x) & MM_VAD_READ)    == MM_VAD_READ)
#define MiIsVadWriteable(x)  (((x) & MM_VAD_WRITE)   == MM_VAD_WRITE)
#define MiIsVadExecutable(x) (((x) & MM_VAD_EXECUTE) == MM_VAD_EXECUTE)
#define MiIsVadCommit(x)     (((x) & MM_VAD_COMMIT)  == MM_VAD_COMMIT)
#define MiIsVadSection(x)    (((x) & MM_VAD_SECTION) == MM_VAD_SECTION)

namespace Mm
{
    /*++

    ROUTINE: InitializePfnListHead

    DESCRIPTION: Sets up a page frame number head with its first entry

    ARGUMENTS: 
        - Head: Pointer to the page list to init
        - Pfn:  Pointer to the first page block to insert

    RETURNS: VOID

    --*/
    INLINE
    VOID
    InitializePfnListHead(PMMPFN_LIST Head,
                          PMMPFN      Pfn)
    {
        KIRQL PreviousIrql = 0;
        Ke::AcquireSpinLock(&Head->Lock,
                        &PreviousIrql);
        
        Head->Head = &Pfn->Link;
        Pfn->Link.Flink = &Pfn->Link;
        Pfn->Link.Blink = &Pfn->Link;
        Head->Total++;

        Ke::ReleaseSpinLock(&Head->Lock,
                            PreviousIrql);
    }

    /*++

    ROUTINE: RemovePfnListEntry

    DESCRIPTION: Removes a page tracking entry from list

    ARGUMENTS: 
        - Head: Pointer to the page list that owns the node
        - Pfn:  Pointer to the page block to remove

    RETURNS: VOID

    --*/
    INLINE
    VOID
    RemovePfnListEntry(PMMPFN_LIST Head,
                       PMMPFN      Pfn)
    {
        KIRQL PreviousIrql = 0;
        Ke::AcquireSpinLock(&Head->Lock,
                        &PreviousIrql);
        
        PLIST_ENTRY Flink = Pfn->Link.Flink;
        PLIST_ENTRY Blink = Pfn->Link.Blink;

        Blink->Flink = Flink;
        Flink->Blink = Blink;

        if (Head->Head == &Pfn->Link)
        {
            Head->Head = Flink;
        }

        Head->Total--;
        Ke::ReleaseSpinLock(&Head->Lock, PreviousIrql);
    }

    /*++

    ROUTINE: InsertPfnListEntry

    DESCRIPTION: Inserts a page entry onto the end of a page list

    ARGUMENTS: 
        - Head: Pointer to the page list to insert onto 
        - Pfn:  Pointer to the page block to insert

    RETURNS: VOID

    --*/
    INLINE
    VOID
    InsertPfnListEntry(PMMPFN_LIST Head,
                       PMMPFN      Pfn)
    {
        KIRQL PreviousIrql = 0;
        Ke::AcquireSpinLock(&Head->Lock,&PreviousIrql);

        if (Head->Total == 0)
        {
            Ke::ReleaseSpinLock(&Head->Lock,PreviousIrql);
            InitializePfnListHead(Head,Pfn);
            return;
        }

        KeInsertTailList(Head->Head,&Pfn->Link);
        Head->Total++;
        Ke::ReleaseSpinLock(&Head->Lock,PreviousIrql);
    }


    /*++

    ROUTINE: PopPfnListEntry

    DESCRIPTION: Removes and gets the first page entry from list

    ARGUMENTS: 
        - Head: Pointer to the physical page list to pull from.

    RETURNS: PMMPFN

    --*/
    INLINE
    PMMPFN
    PopPfnListEntry(PMMPFN_LIST Head)
    {
        KIRQL PreviousIrql = 0;
        Ke::AcquireSpinLock(&Head->Lock,&PreviousIrql);

        if (Head->Total == 0)
        {
            Ke::ReleaseSpinLock(&Head->Lock,PreviousIrql);
            return NULL;
        }

        PLIST_ENTRY Entry = Head->Head;
        PMMPFN Pfn        = CONTAINING_RECORD(Entry, 
                                              MMPFN,
                                              Link);

        PLIST_ENTRY Flink = Pfn->Link.Flink;
        PLIST_ENTRY Blink = Pfn->Link.Blink;

        Blink->Flink = Flink;
        Flink->Blink = Blink;

        if (Head->Head == &Pfn->Link)
        {
            Head->Head = Flink;
        }

        Head->Total--;

        Ke::ReleaseSpinLock(&Head->Lock,PreviousIrql);
        return Pfn;
    }
} // namespace Mm

typedef UCHAR MM_POOL_TYPE;

typedef struct _POOL_HEADER
{
    ULONG32         Tag;
    ULONG64         Length;
    MM_POOL_TYPE    Type;
    PVOID           Reserved;
} POOL_HEADER, *PPOOL_HEADER;

#define MAX_WORKING_SET_LIST_SIZE 255

typedef enum _WSLE_USE
{
    MmMappedUnused,
    MmMappedPhysical,
    MmMappedViewOfSection,
    MmMappedMaximum
} WSLE_USE;

typedef struct _MM_WSLE
{
    union
    {
        struct
        {
            ULONG64 Usage               : 8;
            ULONG64 PageFrameNumber     : 55;
            ULONG64 Reserved            : 1;
            ULONG64 VirtualAddress;
        };

        struct
        {
            ULONG64 Reserved1 : 8;
            ULONG64 Pfn       : 55;
            ULONG64 Copied    : 1;
            ULONG64 Reserved2;
        };

        struct
        {
            ULONG64 Upper;
            ULONG64 Lower;
        };
    };
} MM_WSLE, *PMM_WSLE;

typedef struct _MM_WSL
{
    ULONG64 WrokingSetListCDount;
    MM_WSLE WorkingSetList[MAX_WORKING_SET_LIST_SIZE];
} MM_WSL, *PMM_WSL;

#define MiGetAddressSpace()  Ke::A386ReadCr3()
#define MiSetAddressSpace(A) Ke::A386WriteCr3(A)
#define MiIndexLevel4(address) (((ULONG64)(address) & (0x1FFULL << 39ULL)) >> 39ULL)
#define MiIndexLevel3(address) (((ULONG64)(address) & (0x1FFULL << 30ULL)) >> 30ULL)
#define MiIndexLevel2(address) (((ULONG64)(address) & (0x1FFULL << 21ULL)) >> 21ULL)
#define MiIndexLevel1(address) (((ULONG64)(address) & (0x1FFULL << 12ULL)) >> 12ULL)

#define MI_PFN_DATABASE     0xFFFFFA8000000000ULL
#define MI_NON_PAGED_POOL   (MI_PFN_DATABASE + (1ULL << 40ULL))
#define MI_WORKING_SET_LIST (MI_PFN_DATABASE - (1ULL << 42ULL))

EXTERN PMMPFN     MmPfnDatabase;
EXTERN ULONG64    MmPfnCount;
EXTERN ULONG64    MmPhysicalOffset;
EXTERN MMPFN_LIST MmZeroedPageListHead;
EXTERN MMPFN_LIST MmFreePageListHead;
EXTERN PPMLE      MiKernelAddressSpace;
EXTERN ULONG64    MmKernelBase;
EXTERN ULONG64    MmKernelEnd;

#define PAGE_SIZE 4096

namespace Mm
{
    /*++

    ROUTINE: FlushAddress

    DESCRIPTION: Removes an address from the CPU cache buffer

    ARGUMENTS:
        - Address: The memory address to clear

    RETURNS: VOID

    --*/
    VOID
    FlushAddress(ULONG64 Address);

    /*++

    ROUTINE: ChangePfnVaType

    DESCRIPTION: Updates the type on a page node

    ARGUMENTS: 
        - Pfn:  Pointer to the page block
        - Type: The new virtual memory usage to add

    RETURNS: VOID

    --*/
    VOID
    ChangePfnVaType(PMMPFN     Pfn,
                    MM_VA_TYPE Type);

    /*++

    ROUTINE: ChangePfnRangeVaType

    DESCRIPTION: Updates the type on a range of pages

    ARGUMENTS: 
        - Address: The starting address of the range
        - Length:  The byte size of the range
        - Type:    The new virtual memory usage to add

    RETURNS: VOID

    --*/
    VOID
    ChangePfnRangeVaType(ULONG64    Address,
                         ULONG64    Length,
                         MM_VA_TYPE Type);

    /*++

    ROUTINE: ReferencePfnByAddress

    DESCRIPTION: Finds the page block for a address

    ARGUMENTS: 
        - Address: The address to look up

    RETURNS: PMMPFN

    --*/
    NODISCARD
    PMMPFN
    ReferencePfnByAddress(ULONG64 Address);

    /*++

    ROUTINE: ReferencePfnByVaType

    DESCRIPTION: Finds the first page block that matches usage label

    ARGUMENTS: 
        - Type: The virtual memory usage label to search for

    RETURNS: PMMPFN

    --*/
    NODISCARD
    PMMPFN
    ReferencePfnByVaType(MM_VA_TYPE Type);

    /*++

    ROUTINE: AllocatePhysicalPfn

    DESCRIPTION: Grabs a page from the list

    ARGUMENTS: 
        - Reason: The memory usage label for allocation
        - Pfn:    Pointer to receive the page block address

    RETURNS: ULONG64

    --*/
    NODISCARD
    ULONG64
    AllocatePhysicalPfn(MM_VA_TYPE Reason,
                        ULONG64    MinimumPhysAddress,
                        PMMPFN*    Pfn);

    /*++

    ROUTINE: AllocatePhysical

    DESCRIPTION: Grabs page from the list (address return only)

    ARGUMENTS: 
        - Reason: The memory usage label for the allocation

    RETURNS: ULONG64

    --*/
    NODISCARD
    ULONG64
    AllocatePhysical(MM_VA_TYPE Reason,
                     ULONG64    MinimumPhysAddress = 0);

    /*++

    ROUTINE: AllocatePhysicalZeroedPfn

    DESCRIPTION: Grabs a zeroed page from the list and returns its PFN

    ARGUMENTS: 
        - Reason: The memory usage label for allocation
        - Pfn:    Pointer to the page block

    RETURNS: ULONG64

    --*/
    NODISCARD
    ULONG64
    AllocatePhysicalZeroedPfn(MM_VA_TYPE Reason,
                              PMMPFN*    Pfn);

    /*++

    ROUTINE: AllocatePhysicalZeroed

    DESCRIPTION: Grabs a zeroed page from the list (address return only)

    ARGUMENTS: 
        - Reason: The memory usage label for the allocation

    RETURNS: ULONG64

    --*/
    NODISCARD
    ULONG64
    AllocatePhysicalZeroed(MM_VA_TYPE Reason);

    /*++

    ROUTINE: FreePhysical

    DESCRIPTION: Releases allocated page back to the list

    ARGUMENTS: 
        - Address: The address of the page to release

    RETURNS: VOID

    --*/
    VOID
    FreePhysical(ULONG64 Address);

    /*++

    ROUTINE: InitializeMemoryManager

    DESCRIPTION: Early phase one setup for page tracking

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/
    VOID
    InitializeMemoryManager();

    /*++

    ROUTINE: InitializeMemoryManagerPart2

    DESCRIPTION: Final phase the kernel heap pool

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/
    VOID
    InitializeMemoryManagerPart2();
} // namespace Mm

namespace Mi
{
    /*++

    ROUTINE: IsAddressPresent

    DESCRIPTION: Checks if a virtual address is mapped

    ARGUMENTS: 
        - AddressSpace: The page table base
        - Virtual:      The address to check

    RETURNS: BOOLEAN

    --*/
    NODISCARD
    BOOLEAN
    IsAddressPresent(PPMLE   AddressSpace,
                     ULONG64 Virtual);

    /*++

    ROUTINE: GetPhysicalForVirtual

    DESCRIPTION: Gets the physical address of a virtual address

    ARGUMENTS: 
        - AddressSpace: The page table base
        - Virtual:      The virtual address to read

    RETURNS: ULONG64

    --*/
    NODISCARD
    ULONG64
    GetPhysicalForVirtual(PPMLE   AddressSpace,
                          ULONG64 Virtual);

    /*++

    ROUTINE: UnmapPage

    DESCRIPTION: Unmap a page from the page tables

    ARGUMENTS: 
        - AddressSpace: The page table base
        - Virtual:      The virtual address to unmap

    RETURNS: VOID

    --*/
    VOID
    UnmapPage(PPMLE   AddressSpace,
              ULONG64 Virtual);

    /*++

    ROUTINE: MapLargePage

    DESCRIPTION: Maps a 2MB large page

    ARGUMENTS: 
        - AddressSpace: The page table base
        - Virtual:      The destination virtual address
        - Physical:     The source physical address
        - Write:        Enable write permissions
        - User:         Enable user access
        - Executable:   Enable code execution

    RETURNS: BOOLEAN

    --*/
    NODISCARD
    BOOLEAN
    MapLargePage(PPMLE   AddressSpace,
                 ULONG64 Virtual,
                 ULONG64 Physical,
                 BOOLEAN Write,
                 BOOLEAN User,
                 BOOLEAN Executable);
            
    /*++

    ROUTINE: MapPage

    DESCRIPTION: Maps a standard 4KB page

    ARGUMENTS: 
        - AddressSpace: The page table base
        - Virtual:      The destination virtual address
        - Physical:     The source physical address
        - Write:        Enable write permissions
        - User:         Enable user access
        - Executable:   Enable code execution

    RETURNS: BOOLEAN

    --*/
    NODISCARD
    BOOLEAN
    MapPage(PPMLE   AddressSpace,
            ULONG64 Virtual,
            ULONG64 Physical,
            BOOLEAN Write,
            BOOLEAN User,
            BOOLEAN Executable);

    /*++

    ROUTINE: FlagPage

    DESCRIPTION: Updates permissions on a mapped page

    ARGUMENTS: 
        - AddressSpace: The page table base
        - Virtual:      The destination virtual address
        - Read:         Enable read permissions
        - Write:        Enable write permissions
        - User:         Enable user access
        - Executable:   Enable code execution

    RETURNS: BOOLEAN

    --*/
    NODISCARD
    BOOLEAN
    FlagPage(PPMLE   AddressSpace,
             ULONG64 Virtual,
             BOOLEAN Read,
             BOOLEAN Write,
             BOOLEAN User,
             BOOLEAN Executable);

    /*++

    ROUTINE: PageTableToEntry

    DESCRIPTION: Moves down a page table layer and allocates if missing

    ARGUMENTS: 
        - Top:   The current layer table pointer
        - Index: The entry index to look up
        - Alloc: Create a new sub-table if missing

    RETURNS: PPMLE

    --*/
    NODISCARD
    INLINE
    PPMLE
    PageTableToEntry(PPMLE   Top,
                     ULONG64 Index,
                     BOOLEAN Alloc)
    {
        if ((ULONG64)Top < MmPhysicalOffset)
        {
            Top = (PPMLE)((ULONG64)Top + MmPhysicalOffset);
        }

        if (Top[Index].Present)
        {
            return reinterpret_cast<PPMLE>((Top[Index].PageFrameNumber * PAGE_SIZE) +
                                           MmPhysicalOffset);
        }

        if (!Alloc)
        {
            return NULL;
        }

        ULONG64 NextLevel = Mm::AllocatePhysical(MmTypePageTable);
        Rtl::ZeroMemory((PVOID)(NextLevel + MmPhysicalOffset), PAGE_SIZE);

        Top[Index].PageFrameNumber = NextLevel / PAGE_SIZE;
        Top[Index].Present = 1;
        Top[Index].Write   = 1;
        Top[Index].User    = 1;

        return reinterpret_cast<PPMLE>((NextLevel + MmPhysicalOffset));
    }

    /*++

    ROUTINE: AllocateVad

    DESCRIPTION: Creates a new tracking area block

    ARGUMENTS: 
        - Start: The starting virtual address
        - End:   The ending virtual address
        - Type:  The protection flag byte type

    RETURNS: PMM_VAD

    --*/
    NODISCARD
    PMM_VAD
    AllocateVad(ULONG64 Start,
                ULONG64 End,
                UCHAR   Type);

    /*++

    ROUTINE: FreeVad

    DESCRIPTION: Deletes a tracking area block

    ARGUMENTS: 
        - Vad: The tracking area block to delete

    RETURNS: VOID

    --*/
    VOID 
    FreeVad(PMM_VAD Vad);

    /*++

    ROUTINE: InsertVad

    DESCRIPTION: Adds a tracking area block to a process list

    ARGUMENTS: 
        - Process: The target process structure
        - Vad:     The tracking area block to add

    RETURNS: VOID

    --*/
    VOID
    InsertVad(PKPROCESS Process,
              PMM_VAD   Vad);

    /*++

    ROUTINE: FindVadByAddress

    DESCRIPTION: Finds a tracking area block by address

    ARGUMENTS: 
        - Process: The target process structure
        - Address: The virtual address to find

    RETURNS: PMM_VAD

    --*/
    NODISCARD
    PMM_VAD
    FindVadByAddress(PKPROCESS Process,
                     ULONG64   Address);
                
    /*++

    ROUTINE: RemoveVadByAddress

    DESCRIPTION: Deletes a tracking area block matching an address

    ARGUMENTS: 
        - Process: The target process structure
        - Address: The virtual address to remove

    RETURNS: VOID

    --*/
    VOID
    RemoveVadByAddress(PKPROCESS Process,
                       ULONG64   Address);

    /*++

    ROUTINE: DumpVads

    DESCRIPTION: Prints all tracking area blocks for a process

    ARGUMENTS: 
        - Process: The target process structure

    RETURNS: VOID

    --*/
    VOID
    DumpVads(PKPROCESS Process);

    /*++

    ROUTINE: CreateAddressSpace

    DESCRIPTION: Sets up a new top-level page table directory

    ARGUMENTS: N/A

    RETURNS: ULONG64

    --*/
    NODISCARD
    ULONG64
    CreateAddressSpace();

    /*++

    ROUTINE: VirtualAllocate

    DESCRIPTION: Allocates a virtual address range for a process

    ARGUMENTS: 
        - Process:    The target process structure
        - Base:       Gets or set the allocation address
        - Length:     The total allocation byte size
        - Write:      Enable write permissions
        - User:       Enable user access
        - Executable: Enable code execution
        - Commit:     Allocate physical pages immediately

    RETURNS: HTSTATUS

    --*/
    NODISCARD
    HTSTATUS
    VirtualAllocate(PKPROCESS Process,
                    PVOID*    Base,
                    ULONG64   Length,
                    BOOLEAN   Write,
                    BOOLEAN   User,
                    BOOLEAN   Executable,
                    BOOLEAN   Commit);

    /*++

    ROUTINE: VirtualFree

    DESCRIPTION: Releases a virtual address range for a process

    ARGUMENTS: 
        - Process: The target process
        - Base:    The starting virtual address to clear
        - Length:  The total byte size to clear

    RETURNS: HTSTATUS

    --*/
    NODISCARD
    HTSTATUS
    VirtualFree(PKPROCESS Process,
                PVOID     Base,
                ULONG64   Length);

    /*++

    ROUTINE: FindFreeUserRegion

    DESCRIPTION: Finds an empty memory range in user space

    ARGUMENTS: 
        - Process: The target process
        - Length:  The total byte size needed

    RETURNS: PVOID

    --*/
    NODISCARD
    PVOID
    FindFreeUserRegion(PKPROCESS Process,
                       ULONG     Length);

    /*++

    ROUTINE: IsRegionFree

    DESCRIPTION: Checks if a memory region is free

    ARGUMENTS: 
        - Process: The target process
        - Base:    The starting virtual address
        - Length:  The total byte size to check

    RETURNS: BOOLEAN

    --*/
    NODISCARD
    BOOLEAN
    IsRegionFree(PKPROCESS Process,
                 ULONG64   Base,
                 ULONG     Length);

    /*++

    ROUTINE: DestroyAddressSpace

    DESCRIPTION: Deletes a process page table directory

    ARGUMENTS: 
        - Process: The target process

    RETURNS: HTSTATUS

    --*/
    NODISCARD
    HTSTATUS
    DestroyAddressSpace(PKPROCESS Process);

    /*++

    ROUTINE: HandlePageFault

    DESCRIPTION: Fixes missing or protected memory errors from the CPU

    ARGUMENTS: 
        - TrapFrame: The saved CPU register states
        - Address:   The memory address that caused the error

    RETURNS: HTSTATUS

    --*/
    NODISCARD
    HTSTATUS
    HandlePageFault(PKTRAP_FRAME TrapFrame,
                    ULONG64      Address);
} // namespace Mi

namespace Mm
{
    /*++

    ROUTINE: InsertWslEntry

    DESCRIPTION: Adds a working set list entry

    ARGUMENTS:
        - Entry: Pointer to the working set list entry

    RETURNS: VOID

    --*/
    VOID
    InsertWslEntry(PMM_WSLE Entry);

    /*++

    ROUTINE: ReferenceWslEntryByAddress

    DESCRIPTION: Finds a working set list entry by address

    ARGUMENTS:
        - Address: Virtual address to look up
        - Type:    Working set entry usage type

    RETURNS: PMM_WSLE

    --*/
    NODISCARD
    PMM_WSLE
    ReferenceWslEntryByAddress(ULONG64  Address,
                               WSLE_USE Type);

    /*++

    ROUTINE: RemoveWslEntry

    DESCRIPTION: Removes a working set list entry

    ARGUMENTS:
        - Entry: Pointer to the working set list entry

    RETURNS: VOID

    --*/
    VOID 
    RemoveWslEntry(PMM_WSLE Entry);

    /*++

    ROUTINE: CreateSection

    DESCRIPTION: Creates a section object

    ARGUMENTS:
        - SectionObject: Pointer to receive the section object
        - MaximumSize:   Maximum size of the section
        - Protection:    Section protection flags
        - Flags:         Section creation flags
        - Attributes:    Object attributes
        - FileObject:    Backing file object

    RETURNS: HTSTATUS

    --*/
    NODISCARD
    HTSTATUS
    CreateSection(PMM_SECTION*        SectionObject,
                  ULONG64             MaximumSize,
                  ULONG               Protection,
                  ULONG               Flags,
                  POBJECT_ATTRIBUTES  Attributes,
                  PFILE_OBJECT        FileObject);

    /*++

    ROUTINE: MapViewOfSection

    DESCRIPTION: Maps a section into a process view

    ARGUMENTS:
        - Section:       Section object to map
        - Process:       Target process
        - Base:          Pointer to receive the view base
        - SectionOffset: Offset into the section
        - ViewSize:      Size of the view
        - Protection:    Section protection for the view

    RETURNS: HTSTATUS

    --*/
    NODISCARD
    HTSTATUS
    MapViewOfSection(PMM_SECTION Section,
                     PKPROCESS   Process,
                     PVOID*      Base,
                     ULONG64     SectionOffset,
                     ULONG64     ViewSize,
                     ULONG       Protection);

    /*++

    ROUTINE: UnmapViewOfSection

    DESCRIPTION: Unmaps a section view from a process

    ARGUMENTS:
        - Process:     Target process
        - BaseAddress: Base address of the view

    RETURNS: HTSTATUS

    --*/
    NODISCARD
    HTSTATUS
    UnmapViewOfSection(PKPROCESS Process,
                       PVOID     BaseAddress);
} // namespace Mm