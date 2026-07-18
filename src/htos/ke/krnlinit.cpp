// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Kernel initialization

AUTHOR: Trollycat

ABSTRACT: Kernel entry point

--*/
#include "htbase.hpp"
#include "limine.h"

#include "inbv/inbv.hpp"

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
        Inbv::WriteString("GDT init... Ok\r\n");

        for (;;);
    } 
} // namespace Ki
