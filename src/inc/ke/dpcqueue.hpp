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