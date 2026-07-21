/*++

MODULE: Interrupt Dispatcher

AUTHOR: Trollycat

ABSTRACT: Handles dispatching any interrupts (Or even DPC)

--*/
#include "htdef.hpp"
#include "ke/amd64/amd64.hpp"

#include "rtl/rtl.hpp"

namespace Ki
{
    VOID
    InitializePrcb(PKPRCB Processor)
    {
        Processor->DpcQueueHead.Flink = &Processor->DpcQueueHead;
        Processor->DpcQueueHead.Blink = &Processor->DpcQueueHead;
        
        Processor->DpcQueueDepth = 0;
        Processor->DpcInterruptRequested = FALSE;
        Processor->Self = Processor;

        ULONG64 Base = reinterpret_cast<ULONG64>(Processor);
        __asm__ volatile(
            "wrmsr"
            :
            : "a"(static_cast<ULONG32>(Base)),
            "d"(static_cast<ULONG32>(Base >> 32)),
            "c"(0xC0000101)
        );
    }
    VOID
    ProcessDpcQueue()
    {
        KIRQL PreviousIrql = 0;
        KeRaiseIrql(DISPATCH_LEVEL,&PreviousIrql);

        PKPRCB Processor = KeQueryCurrentProcessor();
        
        while (!KeEmptyList(&Processor->DpcQueueHead))
        {
            PLIST_ENTRY Entry = KeRemoveHeadList(&Processor->DpcQueueHead);
            PKDPC Dpc         = CONTAINING_RECORD(Entry,KDPC,DpcQueue);

            if (Processor->DpcQueueDepth > 0 )
            {
                Processor->DpcQueueDepth--;
            }

            if (!Dpc->Completed)
            {
                Dpc->Completed = TRUE;
                if (Dpc->DefferedRoutine != NULL)
                {
                    Dpc->DefferedRoutine(Dpc, Dpc->DefferedContext);
                }
            }
        }

        Processor->DpcInterruptRequested = FALSE;
        KeLowerIrql(PreviousIrql);
    }

    EXTERN_C
    VOID
    DispatchHardware(PKTRAP_FRAME TrapFrame)
    {
        Rtl::Print("UNEXPECTED HARDWARE INTERRUPT VECTOR: %llu", TrapFrame->Interrupt);
        ASSERTMSG("NO DRIVER HANDLER EXISTS YET, FREEZING", TRUE != TRUE);
    }

    EXTERN_C
    VOID
    DispatchException(PKTRAP_FRAME TrapFrame)
    {
        Rtl::Print("UNHANDLED EXCEPTION VECTOR: %llu ERROR: %llu RIP: %llx CR2: %llx",
                TrapFrame->Interrupt, TrapFrame->Error, TrapFrame->Rip);

        ULONG64 Cr2;
        __asm__ volatile("mov %%cr2, %0" : "=r"(Cr2));
        Rtl::Print("FAULT ADDRESS: %llx", Cr2);

        for (;;)
        {
            __asm__ volatile("cli; hlt");
        }
    }
} // namespace Ki

namespace Ke
{
    VOID
    InitializeDpc(PKDPC Dpc,
                  PKDEFFERED_ROUTINE Routine,
                  PVOID Context)
    {
        Dpc->ProcessorNumber = reinterpret_cast<ULONG64>(KeQueryCurrentProcessor());
        Dpc->Completed       = FALSE;
        Dpc->DefferedContext = Context;
        Dpc->DefferedRoutine = Routine;
    }

    VOID
    InsertQueueDpc(PKDPC Dpc)
    {
        PKPRCB Processor   = KeQueryCurrentProcessor();
        Dpc->Completed = FALSE;

        KeInsertTailList(&Processor->DpcQueueHead,&Dpc->DpcQueue);
        Processor->DpcQueueDepth++;
    }
} // namespace Ke