// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Kernel initialization

AUTHOR: Trollycat

ABSTRACT: Kernel entry point

--*/
#include "htbase.hpp"
#include "limine.h"

STATIC 
VOLATILE 
LIMINE_REQUEST 
QWORD BaseRevision[] = LIMINE_BASE_REVISION(3);

STATIC
VOLATILE
LIMINE_REQUEST
struct limine_framebuffer_request framebuffer_request = 
{
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

STATIC
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
        if (LIMINE_BASE_REVISION_SUPPORTED(BaseRevision) == false)
        {
            for (;;);
        }

        if (framebuffer_request.response == NULL
            || framebuffer_request.response->framebuffer_count < 1)
        {
            for (;;);   
        }

        for (;;);
    }
} // namespce Ki