/*++

MODULE: Interrupt Dispatcher

AUTHOR: Trollycat

ABSTRACT: Handles dispatching any interrupts (Or even DPC)

--*/
#include "htbase.hpp"
#include "bugcodes.hpp"

#include "ke/amd64/amd64.hpp"
#include "ke/bug.hpp"
#include "ke/irql.hpp"
#include "ke/processor.hpp"
#include "ke/dpcqueue.hpp"
#include "ke/intrdispatch.hpp"

#include "rtl/rtl.hpp"

namespace Ki
{
    VOID
    InitializePrcb(PKPRCB Processor)
    {
        Processor->Self = Processor;
        Processor->DpcQueueHead.Flink = &Processor->DpcQueueHead;
        Processor->DpcQueueHead.Blink = &Processor->DpcQueueHead;
        
        Processor->DpcQueueDepth         = 0;
        Processor->DpcInterruptRequested = FALSE;
        Processor->InterruptCount        = 0;

        Processor->ProcessorId = 0;
        Processor->LocalApicId = 0;

        Processor->ProcessorNumber  = 0;
        Processor->TaskState.Rsp[0] = 0;

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
        Ke::RaiseIrql(DISPATCH_LEVEL,&PreviousIrql);

        PKPRCB Processor = Ke::QueryCurrentProcessor();
        
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
        Ke::LowerIrql(PreviousIrql);
    }

    VOID
    InitializeDpc(PKDPC Dpc,
                  PKDEFFERED_ROUTINE Routine,
                  PVOID Context)
    {
        Dpc->ProcessorNumber = reinterpret_cast<ULONG64>(Ke::QueryCurrentProcessor());
        Dpc->Completed       = FALSE;
        Dpc->DefferedContext = Context;
        Dpc->DefferedRoutine = Routine;
    }

    VOID
    InsertQueueDpc(PKDPC Dpc)
    {
        PKPRCB Processor   = Ke::QueryCurrentProcessor();
        Dpc->Completed = FALSE;

        KeInsertTailList(&Processor->DpcQueueHead,&Dpc->DpcQueue);
        Processor->DpcQueueDepth++;
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
        ULONG64 Cr2Value   = 0;
        ULONG BugCheckCode = KMODE_EXCEPTION_NOT_HANDLED;

        if (TrapFrame->Interrupt == 0x03)
        {
            BugCheckCode = KMODE_EXCEPTION_NOT_HANDLED;
        }
        else if (TrapFrame->Interrupt == 0x0E)
        {
            __asm__ volatile("mov %%cr2, %0" : "=r"(Cr2Value));
            BugCheckCode = PAGE_FAULT_IN_NONPAGED_AREA;
        }

        Ke::BugCheckWithIf(BugCheckCode,
                           TrapFrame->Interrupt,
                            TrapFrame->Error,
                            TrapFrame->Rip,
                        Cr2Value,
                                            TrapFrame);
    }
} // namespace Ki