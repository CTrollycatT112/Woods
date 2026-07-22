/*++

MODULE: Shared memory management header

AUTHOR: Trollycat

ABSTRACT: Stores common memory management helpers and types

--*/
#pragma once

#include "htbase.hpp"

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

typedef enum _POOL_TYPE
{
    UnusedPool,
    NonPagedPool,
    NonPagedPoolExecute,
    NonPagedPoolZeroed,
    NonPagedPoolZeroedExecute,
    MaximumPool
} POOL_TYPE;

namespace Mm
{
    /*++

    ROUTINE: IsAddressValid

    DESCRIPTION: Checks if a virtual address is safe to use

    ARGUMENTS: Virtual - The address to check

    RETURNS: BOOLEAN

    --*/

    HTAPI
    BOOLEAN
    IsAddressValid(ULONG64 Virtual);

    /*++

    ROUTINE: AllocatePool

    DESCRIPTION: Allocates a block of memory from the kernel heap,
                 Every allocation is asigned a 4 byte tag,
                 This saves me from a nightmare debugging session,
                 Although it will still use tlsf

    ARGUMENTS: Type   - Pool options,
               Length - Bytes to allocate,
               Tag    - Allocation tag (4 bytes)

    RETURNS: PVOID

    --*/
    
    NODISCARD
    HTAPI
    PVOID
    AllocatePoolWithTag(POOL_TYPE Type, 
                        ULONG64   Length,
                        ULONG32   Tag);

    /*++

    ROUTINE: FreePool

    DESCRIPTION: Free an allocated block of memory from the kernel heap,
                 The block of memory will have a tag,
                 You must know the tag to free the allocation

    ARGUMENTS: Pool - Pointer to the block to free,
               Tag  - Allocation tag (4 bytes)

    RETURNS: VOID

    --*/

    HTAPI
    VOID
    FreePool(PVOID   Pool,
             ULONG32 Tag);

    /*++

    ROUTINE: MapIoSpace

    DESCRIPTION: Maps a hardware physical address into virtual memory,
                 Used for drivers

    ARGUMENTS: Address - Hardware physical address,
               Length  - Bytes to map

    RETURNS: PVOID

    --*/

    HTAPI
    PVOID
    MapIoSpace(ULONG64 Address,
               ULONG64 Length);

    /*++

    ROUTINE: UnmapIoSpace

    DESCRIPTION: Unmaps a mapped hardware address

    ARGUMENTS: Address - The (virtual) address to unmap

    RETURNS: VOID

    --*/

    HTAPI
    VOID
    UnmapIoSpace(ULONG64 Address);

    /*++

    ROUTINE: AllocateForHardwareDma

    DESCRIPTION: Finds a range of physical RAM,
                 Allocates it,
                 Purely used for Hardware Devices

    ARGUMENTS: N/A

    RETURNS: ULONG64

    --*/

    HTAPI
    ULONG64
    AllocateForHardwareDma();

    /*++

    ROUTINE: FreeDmaAllocation

    DESCRIPTION: Release a range of physical RAM,
                 Previously allocated for hardware devices

    ARGUMENTS: Address - The address to unmap

    RETURNS: VOID

    --*/

    HTAPI
    VOID
    FreeDmaAllocation(ULONG64 Address);
} // namespace Mm