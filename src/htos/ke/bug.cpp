/*++

MODULE: Bug check crash handler

AUTHOR: Trollycat

ABSTRACT: BugCheck/Panic System, the operating system cannot recover

--*/
#include "htdef.hpp"
#include "bugcodes.hpp"

#include "ke/spinlock.hpp"
#include "rtl/rtl.hpp"

#include "ke/bug.hpp"
#include "ke/amd64/amd64.hpp"
#include "ke/processor.hpp"

#include "inbv/inbv.hpp"

KSPIN_LOCK KiBugCheckLock = { 0 };

namespace Ke
{

    NORETURN
    HTAPI
    VOID
    BugCheckWithIf(IN ULONG     BugCheckCode,
                   IN ULONG64   BugCheckParameter1,
                   IN ULONG64   BugCheckParameter2,
                   IN ULONG64   BugCheckParameter3,
                   IN ULONG64   BugCheckParameter4,
                   PKTRAP_FRAME TrapFrame)
    {
        __asm__ volatile("cli");

        Ke::AcquireSpinLockAtDpcLevel(&KiBugCheckLock);

        PULONG64 Rip = (PULONG64)__builtin_return_address(0);
        PULONG64 Rsp = (PULONG64)(ULONG64)__builtin_frame_address(0);

        if (TrapFrame != NULL)
        {
            Rip = (PULONG64)TrapFrame->Rip;
            
            if ((TrapFrame->SegCs & 3) == 0)
            {
                Rsp = (PULONG64)((ULONG64)TrapFrame + sizeof(KTRAP_FRAME) - 16);
            }
            else
            {
                Rsp = (PULONG64)TrapFrame->Rsp;
            }
        }

        Inbv::ResetDisplay();

        PCSTR BugCheckName = "UNKNOWN_BUGCHECK_SPECIFIER";

        switch (BugCheckCode)
        {
            case 0x00000001: BugCheckName = "APC_INDEX_MISMATCH"; break;
            case 0x00000002: BugCheckName = "DEVICE_QUEUE_NOT_BUSY"; break;
            case 0x00000003: BugCheckName = "INVALID_AFFINITY_SET"; break;
            case 0x00000004: BugCheckName = "INVALID_DATA_ACCESS_TRAP"; break;
            case 0x00000005: BugCheckName = "INVALID_PROCESS_ATTACH_ATTEMPT"; break;
            case 0x00000006: BugCheckName = "INVALID_PROCESS_DETACH_ATTEMPT"; break;
            case 0x00000007: BugCheckName = "INVALID_SOFTWARE_INTERRUPT"; break;
            case 0x00000008: BugCheckName = "IRQL_NOT_DISPATCH_LEVEL"; break;
            case 0x00000009: BugCheckName = "IRQL_NOT_GREATER_OR_EQUAL"; break;
            case 0x0000000A: BugCheckName = "IRQL_NOT_LESS_OR_EQUAL"; break;
            case 0x0000000C: BugCheckName = "MAXIMUM_WAIT_OBJECTS_EXCEEDED"; break;
            case 0x0000000D: BugCheckName = "MUTEX_ALREADY_OWNED"; break;
            case 0x0000000E: BugCheckName = "SPIN_LOCK_ALREADY_OWNED"; break;
            case 0x0000000F: BugCheckName = "SPIN_LOCK_NOT_OWNED"; break;
            case 0x00000012: BugCheckName = "TRAP_CAUSE_UNKNOWN"; break;
            case 0x00000013: BugCheckName = "EMPTY_THREAD_REAP_LIST"; break;
            case 0x00000014: BugCheckName = "CREATE_DELETE_LOCK_NOT_LOCKED"; break;
            case 0x00000015: BugCheckName = "LAST_CHANCE_CALLOUT_FAILED"; break;
            case 0x00000016: BugCheckName = "CID_HANDLE_CREATION"; break;
            case 0x00000017: BugCheckName = "CID_HANDLE_DELETION"; break;
            case 0x00000018: BugCheckName = "REFERENCE_BY_POINTER"; break;
            case 0x00000019: BugCheckName = "BAD_POOL_HEADER"; break;
            case 0x0000001A: BugCheckName = "SYSTEM_EXIT_OWNED_MUTEX"; break;
            case 0x0000001B: BugCheckName = "AUDIO_DRIVER_INTERNAL"; break;
            case 0x0000001D: BugCheckName = "MUST_SUCCEED_POOL_EMPTY"; break;
            case 0x0000001E: BugCheckName = "KMODE_EXCEPTION_NOT_HANDLED"; break;
            case 0x0000001F: BugCheckName = "KSHARED_USER_DATA_HEADER_CORRUPT"; break;
            case 0x00000020: BugCheckName = "KERNEL_APC_PENDING_DURING_EXIT"; break;
            case 0x00000022: BugCheckName = "FILE_SYSTEM"; break;
            case 0x00000023: BugCheckName = "FAT_FILE_SYSTEM"; break;
            case 0x00000024: BugCheckName = "NTFS_FILE_SYSTEM"; break;
            case 0x00000025: BugCheckName = "NPFS_FILE_SYSTEM"; break;
            case 0x00000026: BugCheckName = "MSFS_FILE_SYSTEM"; break;
            case 0x0000002E: BugCheckName = "DATA_BUS_ERROR"; break;
            case 0x0000002F: BugCheckName = "INSTRUCTION_BUS_ERROR"; break;
            case 0x00000030: BugCheckName = "SET_OF_INVALID_CONTEXT"; break;
            case 0x00000031: BugCheckName = "PHASE0_INITIALIZATION_FAILED"; break;
            case 0x00000032: BugCheckName = "PHASE1_INITIALIZATION_FAILED"; break;
            case 0x00000033: BugCheckName = "UNEXPECTED_INITIALIZATION_CALL"; break;
            case 0x00000034: BugCheckName = "CACHE_MANAGER"; break;
            case 0x00000035: BugCheckName = "NO_MORE_IRP_STACK_LOCATIONS"; break;
            case 0x00000036: BugCheckName = "DEVICE_REFERENCE_COUNT_NOT_ZERO"; break;
            case 0x00000037: BugCheckName = "FLOPPY_INTERNAL_ERROR"; break;
            case 0x00000038: BugCheckName = "SERIAL_DRIVER_INTERNAL"; break;
            case 0x00000039: BugCheckName = "SYSTEM_BOARD_HARDWARE_FAILURE"; break;
            case 0x0000003E: BugCheckName = "MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED"; break;
            case 0x00000049: BugCheckName = "PAGE_FAULT_WITH_INTERRUPTS_OFF"; break;
            case 0x00000050: BugCheckName = "PAGE_FAULT_IN_NONPAGED_AREA"; break;
            case 0x00000051: BugCheckName = "REGISTRY_ERROR"; break;
            case 0x00000052: BugCheckName = "MAILSLOT_FILE_SYSTEM"; break;
            case 0x0000007E: BugCheckName = "SYSTEM_THREAD_EXCEPTION_NOT_HANDLED"; break;
            case 0x0000007F: BugCheckName = "UNEXPECTED_KERNEL_MODE_TRAP"; break;
            case 0x000000A5: BugCheckName = "ACPI_BIOS_ERROR"; break;
            case 0x0000013D: BugCheckName = "CRITICAL_INITIALIZATION_FAILURE"; break;
        }

        Rtl::Print(L":(");
        Rtl::Print(L"Your PC ran into a problem and needs to restart.");
        Rtl::Print(L"Stop code: %s", BugCheckName);

        if (BugCheckCode == KMODE_EXCEPTION_NOT_HANDLED && BugCheckParameter3 > 0 && BugCheckParameter1 >= 0xFFFF800000000000)
        {
            Rtl::Print(L"");
            Rtl::Print(L"--- ASSERTION DETAILS ---");
            Rtl::Print(L"Expression: %s", reinterpret_cast<PCSTR>(BugCheckParameter1));
            Rtl::Print(L"File:       %s", reinterpret_cast<PCSTR>(BugCheckParameter2));
            Rtl::Print(L"Line:       %u", static_cast<ULONG>(BugCheckParameter3));
            
            if (BugCheckParameter4 >= 0xFFFF800000000000)
            {
                Rtl::Print(L"Message:    %s", reinterpret_cast<PCSTR>(BugCheckParameter4));
            }
        }

        Rtl::KdPrint(L"*** STOP: %s (%p, %p, %p, %p)",
                    BugCheckName,
                    (PVOID)BugCheckParameter1,
                    (PVOID)BugCheckParameter2,
                    (PVOID)BugCheckParameter3,
                    (PVOID)BugCheckParameter4);

        PKPRCB Prcb = Ke::QueryCurrentProcessor();
        if (Prcb != NULL)
        {
            Rtl::KdPrint(L"Bug check called on processor %p", (PVOID)(ULONG64)Prcb->ProcessorNumber);
        }

        if (TrapFrame != NULL)
        {
            ULONG64 Cr2Value;
            __asm__ volatile("mov %%cr2, %0" : "=r"(Cr2Value));

            ULONG64 Cr8Value;
            __asm__ volatile("mov %%cr8, %0" : "=r"(Cr8Value));

            Rtl::KdPrint(L"Rax: %p Rbx: %p Rcx: %p", (PVOID)TrapFrame->Rax, (PVOID)TrapFrame->Rbx, (PVOID)TrapFrame->Rcx);
            Rtl::KdPrint(L"Rdx: %p Rdi: %p Rsi: %p", (PVOID)TrapFrame->Rdx, (PVOID)TrapFrame->Rdi, (PVOID)TrapFrame->Rsi);
            Rtl::KdPrint(L"R8:  %p R9:  %p R10: %p", (PVOID)TrapFrame->R8,  (PVOID)TrapFrame->R9,  (PVOID)TrapFrame->R10);
            Rtl::KdPrint(L"R11: %p R12: %p R13: %p", (PVOID)TrapFrame->R11, (PVOID)TrapFrame->R12, (PVOID)TrapFrame->R13);
            Rtl::KdPrint(L"R14: %p R15: %p Rsp: %p", (PVOID)TrapFrame->R14, (PVOID)TrapFrame->R15, (PVOID)Rsp);
            Rtl::KdPrint(L"Rbp: %p Rip: %p EFlags: %p", (PVOID)TrapFrame->Rbp, (PVOID)Rip, (PVOID)TrapFrame->EFlags);
            Rtl::KdPrint(L"CS:  %p DS: %p SS: %p", (PVOID)TrapFrame->SegCs, (PVOID)TrapFrame->SegDs, (PVOID)TrapFrame->SegSs);
            Rtl::KdPrint(L"ES:  %p FS: %p GS: %p", (PVOID)TrapFrame->SegEs, (PVOID)TrapFrame->SegFs, (PVOID)TrapFrame->SegGs);
            Rtl::KdPrint(L"DR0: %p DR1: %p DR2: %p", (PVOID)TrapFrame->Dr0, (PVOID)TrapFrame->Dr1, (PVOID)TrapFrame->Dr2);
            Rtl::KdPrint(L"DR3: %p DR6: %p DR7: %p", (PVOID)TrapFrame->Dr3, (PVOID)TrapFrame->Dr6, (PVOID)TrapFrame->Dr7);
            Rtl::KdPrint(L"ERR: %p CR3: %p INT: %p", (PVOID)TrapFrame->Error, (PVOID)TrapFrame->Cr3, (PVOID)TrapFrame->Interrupt);
            Rtl::KdPrint(L"CR2: %p RSP0: %p IRQL: %p", (PVOID)Cr2Value, (PVOID)(Prcb ? Prcb->TaskState.Rsp : 0), (PVOID)Cr8Value);
        }

        while (TRUE)
        {
            __asm__ volatile("hlt");
        }
    }

    NORETURN
    HTAPI
    VOID
    BugCheckEx(IN ULONG     BugCheckCode,
               IN ULONG64   BugCheckParameter1,
               IN ULONG64   BugCheckParameter2,
               IN ULONG64   BugCheckParameter3,
               IN ULONG64   BugCheckParameter4)
    {
        BugCheckWithIf(BugCheckCode,
                       BugCheckParameter1,
                       BugCheckParameter2,
                       BugCheckParameter3,
                       BugCheckParameter4,
                       NULL);
    }

    NORETURN
    HTAPI
    VOID
    BugCheck(IN ULONG BugCheckCode)
    {
        BugCheckEx(BugCheckCode, 0, 0, 0, 0);
    }
} // namespace Ke