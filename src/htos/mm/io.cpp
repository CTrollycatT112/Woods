/*++

MODULE: I/O Memory Management

AUTHOR: Trollycat

ABSTRACT: Manages I/O memory mappings,
          tracks DMA regions and buffer,
          and supports device memory

--*/
#include "mm/mi.hpp"
#include "mm/mm.hpp"

namespace Mm
{
    NODISCARD
    PVOID
    MapIoSpace(ULONG64 Address, ULONG64 Length)
    {
        // HUGE WARNING
        // THIS IS REALLY STUPID
        // LAZINESS
        // TODO:
        UNREFERENCED_PARAMETER(Length);
        return reinterpret_cast<PVOID>(Address + MmPhysicalOffset);
    }

    VOID
    UnmapIoSpace(ULONG64 Address)
    {
        UNREFERENCED_PARAMETER(Address);
    }
} // namespace Mm