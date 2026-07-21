// NOTE:
//      DPC FUNCTIONS ARE HANDLED IN 'dispatch.cpp'
//      JUST STORED IN A SEPERATE FILE (dpcqueue over intrdispatch)

/*++

MODULE: Deferred Procedure Call

AUTHOR: Trollycat

ABSTRACT: Manages Deferred Procedure Calls at DISPATCH_LEVEL,
          NT LARP.

--*/
#include "ke/amd64/amd64.hpp"

namespace Ki
{
        
    /*++

    ROUTINE: InitializeDpc

    DESCRIPTION: Configures a new DPC object

    ARGUMENTS: Dpc - Pointer to KDPC object,
               Routine - Function to run,
               Context - Data for the function

    RETURNS: N/A

    --*/
    VOID
    InitializeDpc(PKDPC Dpc,
                  PKDEFFERED_ROUTINE Routine,
                  PVOID Context);
    
    /*++

    ROUTINE: InsertQueueDpc

    DESCRIPTION: Adds a deferred function to list

    ARGUMENTS: Dpc - Pointer to KDPC object

    RETURNS: N/A

    --*/
    VOID
    InsertQueueDpc(PKDPC Dpc);

    /*++

    ROUTINE: ProcessDpcQueue

    DESCRIPTION: Drains and processes DPC queue

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/
    VOID
    ProcessDpcQueue();
    
} // namespace Ki