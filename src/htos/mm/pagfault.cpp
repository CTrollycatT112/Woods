/*++

MODULE: Page Fault Exception

AUTHOR: Trollycat

ABSTRACT: Handles page fault exceptions,
          determines fault causes and protections,
          and restores or terminates processes

--*/
#include "htstatus.hpp"
#include "mm/mi.hpp"

ULONG64 MiPageFaultCount = 0;

EXTERN KPROCESS KpKernelProcessInstance;

namespace Mi
{
    NODISCARD
    HTSTATUS
    HandlePageFault(PKTRAP_FRAME TrapFrame,
                    ULONG64      Address)
    {
        if (Ke::GetCurrentIrql() > APC_LEVEL)
        {
            return STATUS_UNSUCCESSFUL;
        }

        PKPROCESS Process = &KpKernelProcessInstance;

        PMM_VAD Vad = FindVadByAddress(Process, Address);

        if (Vad == NULL)
        {
            return STATUS_UNSUCCESSFUL;
        }

        if (!MiIsVadCommit(Vad->Type))
        {
            return STATUS_UNSUCCESSFUL;
        }

        ULONG64 FaultingPage = ALIGN_DOWN(Address, PAGE_SIZE);

        PPMLE VirtualAddressSpace = reinterpret_cast<PPMLE>(
            Process->DirectoryBase + MmPhysicalOffset
        );

        if (MiIsVadCommit(Vad->Type))
        {
            if ((TrapFrame->Error & 0x2) == 0x2 && !MiIsVadWriteable(Vad->Type))
            {
                return STATUS_UNSUCCESSFUL;
            }

            ULONG64 Page = Mm::AllocatePhysical(MmTypeProcessPrivate);
            if (Page == 0)
            {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            Rtl::ZeroMemory(
                reinterpret_cast<PVOID>(Page + MmPhysicalOffset),
                PAGE_SIZE
            );

            if (!Mi::MapPage(VirtualAddressSpace,
                             FaultingPage,
                             Page,
                             MiIsVadWriteable(Vad->Type),
                             Process != &KpKernelProcessInstance,
                             MiIsVadExecutable(Vad->Type)))
            {
                Mm::FreePhysical(Page);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        MiPageFaultCount++;
        return STATUS_SUCCESS;
    }
} // namespace Mi
