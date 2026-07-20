/*++

MODULE: Kernel initialization

AUTHOR: Trollycat

ABSTRACT: Kernel entry point

--*/
#include "htbase.hpp"
#include "limine.h"
#include "htversion.hpp"

#include "ke/amd64/amd64.hpp"

#include "rtl/rtl.hpp"
#include "inbv/inbv.hpp"
#include "hal/kdcom.hpp"

VOLATILE
LIMINE_REQUEST 
QWORD
BaseRevision[] = LIMINE_BASE_REVISION(3);

VOLATILE
LIMINE_REQUEST
struct limine_memmap_request memmap_request =
{
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = NULL
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
    NORETURN
    CODESEG(".text.startup")
    VOID SystemStartup()
    {
        ASSERT(LIMINE_BASE_REVISION_SUPPORTED(BaseRevision) == TRUE);

        Ki::InitializeGdt();

        Inbv::Initialize();

        Hal::Kd::Configure(SERIAL_COM1_BASE,
                          SERIAL_BAUD_RATE_115200);
        Hal::Kd::Write(SERIAL_COM1_BASE, 
                    (PCHAR)CLEAR_HOST_TERMINAL,
                        7);

        Rtl::Print("%s", OS_VERSION_STRING);
        Rtl::Print("GDT Init... OK");
        
        for (;;);
    } 
} // namespace Ki
