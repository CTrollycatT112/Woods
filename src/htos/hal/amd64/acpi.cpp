/*++

MODULE: Advanced Configuration and Power Interface

AUTHOR: Trollycat

ABSTRACT: Hardware power management

--*/

#include "ke/amd64/amd64.hpp"
#include "hal/hal.hpp"
#include "rtl/rtl.hpp"

#include "htbase.hpp"
#include "limine.h"

STATIC
VOLATILE
struct limine_rsdp_request rsdp_request =
{
    .id       = LIMINE_RSDP_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

EXTERN limine_hhdm_request hhdm_request;

PACPI_RSDT     HalAcpiRsdt      = NULL;
STATIC BOOLEAN UsingXsdt        = FALSE;

namespace Hal
{
    namespace
    {
        UCHAR
        AcpiCalculateChecksum(PVOID Ptr,
                              ULONG64 Size)
        {
            UCHAR  Sum  = 0;
            PUCHAR Ptr1 = reinterpret_cast<PUCHAR>(Ptr);

            for (ULONG64 I = 0; I < Size; I++)
            {
                Sum += Ptr1[I];
            }

            return Sum;
        }
    } // namespace

    CODESEG(".init")
    VOID 
    InitializeAcpi()
    {
        ACPI_RSDP* Rsdp = reinterpret_cast<PACPI_RSDP>(rsdp_request.response->address);
        Rtl::Print("ACPI REVISION: %u\r\n", Rsdp->Revision);
        
        if (Rsdp->Revision >= 2 && Rsdp->Xsdt)
        {
            UsingXsdt   = TRUE;
            HalAcpiRsdt = reinterpret_cast<PACPI_RSDT>(Rsdp->Xsdt + MmPhysicalOffset);
            Rtl::Print("XSDT AT: %p\r\n", HalAcpiRsdt);
        }
        else
        {
            HalAcpiRsdt = reinterpret_cast<PACPI_RSDT>(static_cast<ULONG64>(Rsdp->Rsdt) + MmPhysicalOffset);
            UsingXsdt   = FALSE;
            Rtl::Print("RSDT AT: %p\r\n", HalAcpiRsdt);
        }

        Hal::InitializeHpet();
        Hal::InitializeApic();
    }

    PVOID
    AcpiFindSdt(LPCSTR Signature)
    {
        CONST ULONG64 Entries = (reinterpret_cast<PACPI_HEADER>(HalAcpiRsdt)->Length - sizeof(ACPI_HEADER)) / (UsingXsdt ? 8 : 4);

        for (ULONG64 I = 0; I < Entries; I++)
        {
            PACPI_HEADER Ptr = NULL;

            if (UsingXsdt)
            {
                PULONG64 Tables = reinterpret_cast<PULONG64>(reinterpret_cast<PBYTE>(HalAcpiRsdt) + sizeof(ACPI_HEADER));
                Ptr = reinterpret_cast<PACPI_HEADER>(Tables[I]);
            }
            else
            {
                PULONG32 Tables = reinterpret_cast<PULONG32>(reinterpret_cast<PBYTE>(HalAcpiRsdt) + sizeof(ACPI_HEADER));
                Ptr = reinterpret_cast<PACPI_HEADER>(static_cast<ULONG64>(Tables[I]));
            }

            Ptr = reinterpret_cast<PACPI_HEADER>(reinterpret_cast<ULONG64>(Ptr) + MmPhysicalOffset);

            if (Rtl::CompareMemory(Ptr->Signature, Signature, 4) != 4)
            {
                continue;
            }

            if (AcpiCalculateChecksum(Ptr, Ptr->Length) != 0)
            {
                continue;
            }

            return Ptr;
        }

        return NULL;
    }
} // namespace Hal