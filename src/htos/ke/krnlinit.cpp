/*++

MODULE: Kernel initialization

AUTHOR: Trollycat

ABSTRACT: Kernel entry point

--*/
#include "hal/hal.hpp"
#include "htbase.hpp"
#include "limine.h"
#include "htversion.hpp"

#include "ke/amd64/amd64.hpp"
#include "ke/processor.hpp"

#include "rtl/rtl.hpp"
#include "inbv/inbv.hpp"
#include "hal/kdcom.hpp"

VOLATILE
LIMINE_REQUEST 
QWORD
BaseRevision[] = LIMINE_BASE_REVISION(0);

VOLATILE
struct limine_stack_size_request stack_size_request =
{
    .id         = LIMINE_STACK_SIZE_REQUEST_ID,
    .revision   = 0,
    .response   = NULL,
    .stack_size = 4080 * 8
};

VOLATILE
struct limine_module_request module_request =
{
    .id                    = LIMINE_MODULE_REQUEST_ID,
    .revision              = 0,
    .response              = NULL,
    .internal_module_count = 0,
    .internal_modules      = NULL
};

VOLATILE
LIMINE_REQUEST
struct limine_memmap_request memmap_request =
{
    .id       = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

VOLATILE
LIMINE_REQUEST
struct limine_hhdm_request hhdm_request =
{
    .id       = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

STATIC KPRCB BootPrcb;

ULONG64 MmPhysicalOffset = 0;

namespace Ki
{
    /*++

    ROUTINE: SystemStartup

    DESCRIPTION: Kernel entry point

    ARGUMENTS: N/A

    RETURNS: NORETURN

    --*/
    EXTERN_C
    NORETURN
    CODESEG(".text.startup")
    VOID SystemStartup()
    {
        ASSERT(LIMINE_BASE_REVISION_SUPPORTED(BaseRevision) == TRUE);
        MmPhysicalOffset = hhdm_request.response->offset;

        Inbv::Initialize();

        Hal::Kd::Configure(SERIAL_COM1_BASE,
                          SERIAL_BAUD_RATE_115200);
        Hal::Kd::Write(SERIAL_COM1_BASE, 
                    (PCHAR)CLEAR_HOST_TERMINAL,
                        7);

        Rtl::Print("%s", OS_VERSION_STRING);

        Rtl::Print("   ");
        Rtl::Print("    ");


        Rtl::Print("GDT Init... OK");
        Ki::InitializeGdt();

        Rtl::Print("IDT Init.. Ok");
        Hal::InitializeIdt();

        Rtl::Print("PRCB Init... Ok");
        InitializePrcb(&BootPrcb);

        Rtl::Print("ACPI Init... Ok");
        Hal::InitializeAcpi();

        Rtl::Print("    ");

        for (;;);
    } 
} // namespace Ki