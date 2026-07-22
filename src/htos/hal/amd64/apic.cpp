/*++

MODULE: Advanced Programmable Interrupt Controller

AUTHOR: Trollycat

ABSTRACT: Hardware interrupt routing

--*/
#include "hal/hal.hpp"
#include "hal/ioport.hpp"

#include "ke/amd64/amd64.hpp"
#include "ke/bug.hpp"
#include "ke/processor.hpp"

#include "limine.h"
#include "bugcodes.hpp"

STATIC PMADT HalMadt = NULL;

STATIC PMADT_PROCESSOR_LOCAL_APIC HalMadtLocalApics[256];
STATIC PMADT_IO_APIC              HalMadtIoApics[256];
STATIC PMADT_INTERRUPT_SOURCE_OVERRIDE_APIC HalMadtIsoApics[256];
STATIC PMADT_NON_MASKABLE_INTERRUPT_APIC HalMadtNmiApics[256];

ULONG64 HalMadtLocalApicCount       = 0;
STATIC ULONG64 HalMadtIoApicCount   = 0;
STATIC ULONG64 HalMadtIsoApicCount  = 0;
STATIC ULONG64 HalMadtNmiApicCount  = 0;

STATIC BOOLEAN UsingX2Apic = FALSE;

ULONG64 HalBspLocalApicId = 0;

STATIC ULONG64 HalLocalApic = 0;

EXTERN limine_rsdp_request rsdp_request;

namespace Hal
{
    VOID
    InitializeApic()
    {
        HalMadt = reinterpret_cast<PMADT>(AcpiFindSdt("APIC"));

        if (HalMadt == NULL)
        {
            Ke::BugCheckEx(HAL_INITIALIZATION_FAILED,
                            MAKE_TAG('C', 'I', 'P', 'A'),
                            0x0,
                            (ULONG64)HalMadt,
                            0x0);
        }

        for (ULONG64 MadtPtr = (ULONG64)&HalMadt->Entry0; MadtPtr < (ULONG64)(HalMadt)+HalMadt->Header.Length; MadtPtr += ((PMADT_HEADER)MadtPtr)->RecordLength)
        {
            PMADT_HEADER MadtPtr1 = (PMADT_HEADER)MadtPtr;
            switch (MadtPtr1->EntryType) {
                case ACPI_MADT_TYPE_PROCESSOR_LOCAL_APIC:
                    HalMadtLocalApics[HalMadtLocalApicCount++] = (PMADT_PROCESSOR_LOCAL_APIC)MadtPtr;
                    break;
                case ACPI_MADT_TYPE_IO_APIC:
                    HalMadtIoApics[HalMadtIoApicCount++] = (PMADT_IO_APIC)MadtPtr;
                    break;
                case ACPI_MADT_TYPE_INTERRUPT_SOURCE_OVERRIDE_APIC:
                    HalMadtIsoApics[HalMadtIsoApicCount++] = (PMADT_INTERRUPT_SOURCE_OVERRIDE_APIC)MadtPtr;
                    break;
                case ACPI_MADT_TYPE_NON_MASKABLE_INTERRUPT_APIC:
                    HalMadtNmiApics[HalMadtNmiApicCount++] = (PMADT_NON_MASKABLE_INTERRUPT_APIC)MadtPtr;
                    break;
                case ACPI_MADT_TYPE_LOCAL_APIC_ADDRESS_OVERRIDE_APIC:
                    HalLocalApic = *((PULONG64)(MadtPtr + 4));
                    break;
                default:
                    break;
            }
	    }

        if (HalLocalApic == 0)
        {
            HalLocalApic = HalMadt->LocalApicAddress;
        }

        HalLocalApic = (HalLocalApic + MmPhysicalOffset);

        ULONG32 eax = 1, ebx = 0, ecx = 0, edx = 0;
        __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));

        DWORD Low, High;
        __asm__ volatile("rdmsr" : "=a"(Low), "=d"(High) : "c"(0x1B));

        if (ecx & (1 << 21))
        {
            UsingX2Apic = TRUE;
            Low |= (1 << 11) | (1 << 10);
        }
        else
        {
            UsingX2Apic = FALSE;
            Low |= (1 << 11);
        }

        __asm__ volatile("wrmsr" :: "a"(Low), "d"(High), "c"(0x1B));

        if (HalMadt->Flags & 1)
        {
            WRITE_PORT_UCHAR(0x20,0x11);
            WRITE_PORT_UCHAR(0xA0,0x11);
            WRITE_PORT_UCHAR(0x21,0x20);
            WRITE_PORT_UCHAR(0xA1,0x28);
            WRITE_PORT_UCHAR(0x21,4);
            WRITE_PORT_UCHAR(0xA1, 2);
            WRITE_PORT_UCHAR(0x21, 1);
            WRITE_PORT_UCHAR(0xA1,1);
            WRITE_PORT_UCHAR(0x21,0);
            WRITE_PORT_UCHAR(0xA1,0);

            WRITE_PORT_UCHAR(0xA1,0xFF);
            WRITE_PORT_UCHAR(0x21, 0xFF);
        }
    }

    namespace
    {
        ULONG32
        RegToX2Apic(ULONG32 Reg)
        {
            ULONG64 X2ApicReg = 0;

            if (Reg == 0x310)
            {
                X2ApicReg = 0x30;
            }
            else
            {
                X2ApicReg = Reg >> 4;
            }

            return X2ApicReg + 0x800;
        }

        VOID
        LocalApicSetNmi(UCHAR Vector,
                        UCHAR CurrentProcessorId,
                        UCHAR ProcessorId,
                        USHORT Flags,
                        UCHAR Lint)
        {
            if (ProcessorId != 0xFF)
            {
                if (CurrentProcessorId != ProcessorId)
                {
                    return;
                }
            }

            ULONG32 nmi = 0x400 | Vector;

            if (Flags & 2)
            {
                nmi |= 1 << 13;
            }

            if (Flags & 8)
            {
                nmi |= 1 << 15;
            }

            if (Lint == 0)
            {
                LocalApicWrite(0x350,nmi);
            }
            else if (Lint == 1)
            {
                LocalApicWrite(0x360,nmi);
            }
        }

        ULONG32
        GetGsiCount(ULONG64 Address)
        {
            return (IoApicRead(Address,1) & 0xFF0000) >> 16;
        }

        PMADT_IO_APIC
        GetIoApicFromGsi(ULONG32 Gsi)
        {
            for (ULONG64 I = 0; I < HalMadtIoApicCount; I++)
            {
                PMADT_IO_APIC IoApic = HalMadtIoApics[I];
                if (IoApic->GlobalSystemInterruptBase <= Gsi &&
                    IoApic->GlobalSystemInterruptBase + GetGsiCount(IoApic->IoApicAddress) > Gsi)
                {
                    return IoApic;
                }
            }

            return NULL;
        }
    } // namespace

    ULONG32
    LocalApicRead(ULONG32 Register)
    {
        if (UsingX2Apic)
        {
            ULONG32 Low, High;
            ULONG32 Msr = RegToX2Apic(Register);

            __asm__ volatile("rdmsr" : "=a"(Low), "=d"(High) : "c"(Msr));
            return Low;
        }

        return *(volatile ULONG32*)(HalLocalApic + Register);
    }

    VOID
    LocalApicWrite(ULONG32 Register, ULONG32 Value)
    {
        if (UsingX2Apic)
        {
            ULONG32 Msr  = RegToX2Apic(Register);
            ULONG32 High = 0;

            if (Register == 0x300)
            {
                High = LocalApicRead(0x310);
            }

            __asm__ volatile("wrmsr" :: "a"(Value), "d"(High), "c"(Msr));
        }
        else
        {
            *(volatile ULONG32*)(HalLocalApic + Register) = Value;
        }
    }

    EXTERN_C
    VOID 
    Eoi()
    {
        LocalApicWrite(0x80, 0);
    }

    VOID
    InitializeLocalApic(UCHAR LocalApicId)
    {
        auto ReadMsr = [](uint32_t Msr) -> uint64_t {
            uint32_t Low, High;
            asm volatile("rdmsr" : "=a"(Low), "=d"(High) : "c"(Msr));
            return ((uint64_t)High << 32) | Low;
        };

        auto WriteMsr = [](uint32_t Msr, uint64_t Value) {
            uint32_t Low = (uint32_t)(Value & 0xFFFFFFFF);
            uint32_t High = (uint32_t)(Value >> 32);
            asm volatile("wrmsr" :: "a"(Low), "d"(High), "c"(Msr));
        };

        WriteMsr(0x1B, (1ULL << 11) | ReadMsr(0x1B));

        if (UsingX2Apic) {
            WriteMsr(0x1B, (1ULL << 10) | ReadMsr(0x1B));
        }

        LocalApicWrite(0x80, 0);
        LocalApicWrite(0xF0, LocalApicRead(0xF0) | 0x100);

        if (!UsingX2Apic) {
            LocalApicWrite(0xE0, 0xF0000000);
            LocalApicWrite(0xD0, LocalApicRead(0x20));
        }

        for (ULONG64 Index = 0; Index < HalMadtNmiApicCount; Index++) {
            PMADT_NON_MASKABLE_INTERRUPT_APIC NmiRecord = HalMadtNmiApics[Index];
            LocalApicSetNmi(2, LocalApicId, NmiRecord->AcpiProcessorId, NmiRecord->Flags, NmiRecord->LocalInterrupt);
        }

        asm volatile("mov %0, %%cr8" :: "r"((uint64_t)DISPATCH_LEVEL) : "memory");

        LocalApicWrite(0x3E0, 3);
        LocalApicWrite(0x380, 0xFFFFFFFF);

        HpetStall(10 * 1000);

        LocalApicWrite(0x320, 0x10000);

        ULONG64 TimerTicksIn10ms = 0xFFFFFFFF - LocalApicRead(0x390);

        LocalApicWrite(0x3E0, 3);
        LocalApicWrite(0x380, TimerTicksIn10ms / 10);
        LocalApicWrite(0x320, 32 | 0x20000);
    }

    ULONG32
    IoApicRead(ULONG64 Address,
               ULONG64 Reg)
    {
        VOLATILE PULONG R = (PULONG)(Address + MmPhysicalOffset);
        *R = Reg & 0xFF;
        R = (PULONG)(Address + MmPhysicalOffset + 16);
        return *R;
    }

    VOID
    IoApicWrite(ULONG64 Address,
                ULONG64 Reg,
                ULONG32 Val)
    {
        VOLATILE PULONG R = (PULONG)(Address + MmPhysicalOffset);
        *R = Reg & 0xFF;
        R  = (PULONG)(Address + MmPhysicalOffset + 16);
        *R = Val;
    }
    
    VOID
    IoApicRedirectGsi(ULONG32 Gsi,
                      UCHAR Vector,
                      USHORT Flags)
    {
        PMADT_IO_APIC IoApicRecord = GetIoApicFromGsi(Gsi);
        if (!IoApicRecord)
        {
            return;
        }

        ULONG64 IoApic = GetIoApicFromGsi(Gsi)->IoApicAddress;

        ULONG32 LowIndex  = 0x10 + (Gsi - GetIoApicFromGsi(Gsi)->GlobalSystemInterruptBase) * 2;
        ULONG32 HighIndex = LowIndex + 1;

        ULONG32 High = IoApicRead(IoApic,HighIndex);
        High &= ~0xFF000000;
        High |= (ULONG32)GetLocalApicId() << 24;
        IoApicWrite(IoApic, HighIndex, High);

        ULONG32 Low = IoApicRead(IoApic, LowIndex);

        Low &= ~(1 << 16);
        Low &= ~(1 << 11);
        Low &= ~0x700;
        Low &= ~0xFF;
        Low |= Vector;

        if (Flags & 2)
        {
            Low |= 1 << 13;
        }

        if (Flags & 8)
        {
            Low |= 1 << 15;
        }

        IoApicWrite(IoApic, LowIndex, Low);
    }

    HTAPI
    VOID
    IoApicRedirectIrq(ULONG32 Irq,
                      UCHAR Vector)
    {
        for (ULONG64 I = 0; I < HalMadtIsoApicCount; I++)
        {
            if (HalMadtIsoApics[I]->IrqSource == Irq)
            {
                IoApicRedirectGsi(HalMadtIsoApics[I]->GlobalSystemInterrupt,
                                  Vector,
                                  HalMadtIsoApics[I]->Flags);
                return;
            }
        }

        IoApicRedirectGsi(Irq, Vector, 0);
    }

    UCHAR 
    GetLocalApicId()
    {
        return Ke::QueryCurrentProcessor()->LocalApicId;
    }
} // namespace Hal