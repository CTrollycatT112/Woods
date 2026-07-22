/*++

MODULE: Kernel initialization

AUTHOR: Trollycat

ABSTRACT: Kernel entry point

--*/
#include "htbase.hpp"
#include "htversion.hpp"
#include "limine.h"

#include "ke/irql.hpp"
#include "ke/amd64/amd64.hpp"
#include "ke/processor.hpp"

#include "hal/hal.hpp"
#include "hal/kdcom.hpp"

#include "mm/mi.hpp"

#include "rtl/rtl.hpp"

#include "inbv/inbv.hpp"

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
    .stack_size = PAGE_SIZE * 8
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

STATIC KPRCB BootPrcb;

//
// TEMP HACK
//
KPROCESS KpKernelProcessInstance = {
    .ProcessId          = 0,
    .ProcessStatus      = 0,
    .ProcessLinks       = { NULL,NULL},
    .DirectoryBase      = 0,
    .VadLock            = 0,
    .Vads               = NULL,
    .WorkingSetLock     = 0,
    .UserRegionHint     = 0,
    .ThreadCount        = 0,
    .ThreadList         = NULL,
    .ThreadListLock     = 0,
    .DllRegionhint      = 0,
    .Name               = { 0 }
};


namespace Ki
{
    /*++

    ROUTINE: SystemStartup

    DESCRIPTION: Kernel entry point

    ARGUMENTS: N/A

    RETURNS: NORETURN

    --*/
    EXTERN_C
    CODESEG(".text")

    NORETURN
    VOID SystemStartup()
    {
        ASSERT(LIMINE_BASE_REVISION_SUPPORTED(BaseRevision) == TRUE);

        Hal::Kd::Configure(SERIAL_COM1_BASE, SERIAL_BAUD_RATE_115200);
        Hal::Kd::Write(SERIAL_COM1_BASE, (PCHAR)CLEAR_HOST_TERMINAL, 7);

        Mm::InitializeMemoryManager();
        Inbv::Initialize();

        Rtl::Print("%s\n", OS_VERSION_STRING);
        Rtl::Print("-------------------------------------\n");

        Rtl::Print("GDT Init... OK\n");
        Ki::InitializeGdt();
        Rtl::Print("IDT Init... OK\n");
        Hal::InitializeIdt();

        InitializePrcb(&BootPrcb);

        Hal::InitializeAcpi();

        Ke::LowerIrql(PASSIVE_LEVEL);

        Mm::FreeInitCode();

        while (TRUE)
        {
            __asm__ volatile("hlt");
        }
    }
} // namespace Ki