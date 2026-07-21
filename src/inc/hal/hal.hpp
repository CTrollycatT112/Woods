/*++

MODULE: Hardware abstraction layer

AUTHOR: Trollycat

ABSTRACT: AMD64 HAL ONLY

--*/
#pragma once

#include "htbase.hpp"

// NOTE:
// IM VERY TIRED
// I REALLY DON'T FEEL LIKE RIGHTING DOC
// SO COMMENTS ARE STUPID..

namespace Hal
{
    /*++

    ROUTINE: LoadIdt

    DESCRIPTION: Simple, load's the idt using inline assembly

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/

    VOID
    LoadIdt();

    /*++

    ROUTINE: InitializeIdt

    DESCRIPTION: Fill and initialize the IDT

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/

    VOID
    InitializeIdt();

    /*++

    ROUTINE: InitializeHpet

    DESCRIPTION: Initialize the event timer clock

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/

    VOID 
    InitializeHpet();

    /*++

    ROUTINE: InitializeApic

    DESCRIPTION: Initialize the Advanced Programmable Interrupt Controller

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/

    VOID 
    InitializeApic();

    /*++

    ROUTINE: InitializeLocalApic

    DESCRIPTION: Initialize the APIC per-core

    ARGUMENTS: LocalApicId - CPU Core to configure

    RETURNS: VOID

    --*/

    VOID
    InitializeLocalApic(UCHAR LocalApicId);

    /*++

    ROUTINE: HpetStall

    DESCRIPTION: STALL the timer by given (Microseconds)

    ARGUMENTS:

    RETURNS:

    --*/

    VOID
    HpetStall(ULONG64 Microseconds);

    /*++

    ROUTINE: InitializeAcpi

    DESCRIPTION: Initialize the advanced controllable power interface

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/

    VOID 
    InitializeAcpi();

    /*++

    ROUTINE: AcpiFindSdt

    DESCRIPTION: Returns pointer to ACPI Description Table

    ARGUMENTS: Signature - Target table(4 char)

    RETURNS: PVOID

    --*/

    HTAPI
    PVOID
    AcpiFindSdt(LPCSTR Signature);

    /*++

    ROUTINE: GetTimerFrequency

    DESCRIPTION: Get HPET current Frequency

    ARGUMENTS: N/A

    RETURNS: ULONG64

    --*/

    HTAPI
    ULONG64
    GetTimerFrequency();

    /*++

    ROUTINE: GetTimerCount

    DESCRIPTION: Get the current main counter value from the HPET

    ARGUMENTS: N/A

    RETURNS: ULONG64

    --*/

    HTAPI
    ULONG64
    GetTimerCount();

    /*++

    ROUTINE: LocalApicRead

    DESCRIPTION: Read from a local APIC register

    ARGUMENTS: Register - The register to read from

    RETURNS: ULONG32

    --*/

    ULONG32
    LocalApicRead(ULONG32 Register);

    /*++

    ROUTINE: LocalApicWrite

    DESCRIPTION: Write  to a local APIC register

    ARGUMENTS: Register - The register
               Value - The data to write

    RETURNS: VOID

    --*/

    VOID
    LocalApicWrite(ULONG32 Register,
                   ULONG32 Value);

    /*++

    ROUTINE: Eoi

    DESCRIPTION: Send an End of Interrupt signal to the local APIC

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/
    EXTERN_C
    VOID
    Eoi();

    /*++

    ROUTINE: GetLocalApicId

    DESCRIPTION: Get the local APIC ID of the current CPU core

    ARGUMENTS: N/A

    RETURNS: UCHAR

    --*/

    HTAPI
    UCHAR
    GetLocalApicId();

    /*++

    ROUTINE: IoApicRead

    DESCRIPTION: Read a register from I/O APIC base address

    ARGUMENTS: Address - I/O APIC base,
               Reg     - Register index

    RETURNS: ULONG32

    --*/

    ULONG32
    IoApicRead(ULONG64 Address,
               ULONG64 Reg);

    /*++

    ROUTINE: IoApicWrite

    DESCRIPTION: Write to an I/O APIC register

    ARGUMENTS: Address - I/O APIC base,
               Reg     - Register index,
               Val     - Data to write

    RETURNS: VOID

    --*/

    VOID
    IoApicWrite(ULONG64 Address,
                ULONG64 Reg,
                ULONG32 Val);

    /*++

    ROUTINE: IoApicRedirectGsi

    DESCRIPTION: Route a Global System Interrupt to a IDT vector

    ARGUMENTS: Gsi - Interrupt number,
               Vector - IDT target,
               Flags - Trigger flags

    RETURNS: VOID

    --*/

    VOID IoApicRedirectGsi(ULONG32 Gsi,
                           UCHAR   Vector,
                           USHORT  Flags);
    
    /*++

    ROUTINE: IoApicRedirectIrq

    DESCRIPTION: Map a standard ISA IRQ line to an IDT vector

    ARGUMENTS: Irq - ISA interrupt line,
               Vector - IDT target vector

    RETURNS: VOID

    --*/

    HTAPI
    VOID
    IoApicRedirectIrq(ULONG32 Irq,
                      UCHAR   Vector);

} // namespace Hal