// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Kernel initialization

AUTHOR: Trollycat

ABSTRACT: Kernel entry point

--*/
#include "htbase.hpp"
#include "limine.h"

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
        if (LIMINE_BASE_REVISION_SUPPORTED(BaseRevision) == FALSE)
        {
            for (;;);
        }

        Inbv::Initialize();
        Hal::Kd::Configure(SERIAL_COM1_BASE,
                          SERIAL_BAUD_RATE_115200);
          
        //
        // STUPID TUNG TUNG HACK
        // MANUALLY CLEAR HOST CONSOLE
        // SERIAL OUTPUT GET'S STUCK WITHIN OVMF OUTPUT
        // I DON'T KNOW HOW TO STOP OVMF OUTPUT
        // SO MANUALLY CLEAR CONSOLE
        //
        Hal::Kd::Write(SERIAL_COM1_BASE, 
                         (PCHAR)CLEAR_HOST_TERMINAL,
                             7);

        for (INT I = 0; I < 51; I++)
        {
            Rtl::Print("I: %d", I);
        }

        for (;;);
    } 
} // namespace Ki
