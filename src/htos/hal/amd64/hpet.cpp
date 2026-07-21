/*++

MODULE: High Precision Event Timer

AUTHOR: Trollycat

ABSTRACT: HPET TIMER for timing

--*/
#include "hal/hal.hpp"
#include "rtl/rtl.hpp"

#include "limine.h"

#include "ke/amd64/amd64.hpp"

//
// CONSTANTS
//
#define FEMTOSECONDS_PER_SECOND       1000000000000000ULL
#define FEMTOSECONDS_PER_MICROSECOND  1000000000ULL
#define HPET_PERIOD_SHIFT             32
#define HPET_CFG_OFF                  0ULL
#define HPET_CFG_ENABLE               1ULL

//
// STUPID HACK ALERT!!!!
// TODO:
//
//
//
// SO WE DONT HAVE MM YET
// SO WE'RE HARD-CODING HPET INTO LIMINES HHDM
// THIS IS REALLY DUMB
// ALSO THIS FUNCTION SHOULDN'T BE HERE
// FIX AS SOON AS MM IS A THING..
//
EXTERN
VOLATILE
struct limine_hhdm_request hhdm_request;

EXTERN_C
PVOID
MmMapIoSpace(QWORD PhysAddress, QWORD Size)
{
    UNREFERENCED_PARAMETER(Size);
    if (!hhdm_request.response) {
        return NULL; 
    }

    QWORD VirtualAddress = hhdm_request.response->offset + PhysAddress;
    return reinterpret_cast<PVOID>(VirtualAddress);
}
// END OF SECTION
//
//

STATIC PACPI_HPET_TABLE HpetTable = NULL;
STATIC PACPI_HPET Hpet            = NULL;

namespace Hal
{
    VOID
    InitializeHpet()
    {
        HpetTable = reinterpret_cast
        <PACPI_HPET_TABLE>
        (AcpiFindSdt("HPET"));

        ASSERTMSG("FAILED TO INITIALIZE HPET", 
                  HpetTable != NULL);

        ASSERTMSG("FAILED TO INITIALIZE HPET",
                HpetTable->AddressSpaceId == 0 &&
                HpetTable->Address        != 0);

        Hpet = reinterpret_cast<PACPI_HPET>
        (MmMapIoSpace(HpetTable->Address,
                            sizeof(ACPI_HPET)));

        ASSERTMSG("FAILED TO INITIALIZE HPET", Hpet != NULL);

        Hpet->GeneralConfiguration = HPET_CFG_OFF;
        Hpet->CounterValue         = 0;
        Hpet->GeneralConfiguration = HPET_CFG_ENABLE;
    }

    VOID
    HpetStall(ULONG64 Microseconds)
    {
        ULONG64 PeriodFems  = Hpet->GeneralCapabilities >> HPET_PERIOD_SHIFT;
        ULONG64 TicksToWait = (Microseconds * FEMTOSECONDS_PER_MICROSECOND) / PeriodFems;
        ULONG64 StartCount  = Hpet->CounterValue;

        while ((Hpet->CounterValue - StartCount) < TicksToWait)
        {
            __asm__ volatile("pause");
        }
    }

    ULONG64
    GetTimerFrequency()
    {
        ULONG64 PeriodFems = Hpet->GeneralCapabilities >> HPET_PERIOD_SHIFT;
        return FEMTOSECONDS_PER_SECOND / PeriodFems;
    }

    ULONG64
    GetTimerCount()
    {
        return Hpet->CounterValue;
    }
} // namespace Hal