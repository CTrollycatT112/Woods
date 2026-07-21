/*++

MODULE: Spinlock

AUTHOR: Trollycat

ABSTRACT:
    A synchronization lock,
    causes a thread waiting to acquire it to remain active in a loop,
    while checking if the lock becomes available

--*/
#pragma once

#include "ke/amd64/amd64.hpp"
#include "ke/irql.hpp"

#include "htbase.hpp"

namespace Ke
{
    
    /*++

    ROUTINE: AcquireSpinLock

    DESCRIPTION: Disable thread switching,
                 lock the resource

    ARGUMENTS: SpinLock     - Pointer to the lock you want to grab,
               PreviousIrql - Variable to store old IRQL 

    RETURNS: VOID

    --*/

    INLINE
    VOID
    AcquireSpinLock(PKSPIN_LOCK SpinLock, 
                    PKIRQL PreviousIrql)
    {
        Ke::RaiseIrql(DISPATCH_LEVEL,PreviousIrql);

        while (__sync_val_compare_and_swap(SpinLock, 0, 1) != 0)
        {
            __builtin_ia32_pause();
        }
    }

    /*++

    ROUTINE: ReleaseSpinLock

    DESCRIPTION: Unlocks the resource,
                 Allow thread switching again

    ARGUMENTS: SpinLock     - Pointer to the lock you want to release
               PreviousIrql - Old IRQL that you stored during lock

    RETURNS: VOID

    --*/

    INLINE
    VOID
    ReleaseSpinLock(PKSPIN_LOCK SpinLock, 
                    KIRQL PreviousIrql)
    {
        __sync_lock_release(SpinLock);
        Ke::LowerIrql(PreviousIrql);
    }

    /*++

    ROUTINE: AcquireSpinLockAtDpcLevel

    DESCRIPTION: Fast lock,
                 skips changing the CPU priority level,
                 Thread switching is already disabled

    ARGUMENTS: SpinLock - Pointer to the lock you want to grab

    RETURNS: VOID

    --*/

    INLINE
    VOID
    AcquireSpinLockAtDpcLevel(PKSPIN_LOCK SpinLock)
    {
        while (__sync_val_compare_and_swap(SpinLock, 0, 1) != 0)
        {
            __builtin_ia32_pause();
        }
    }
    
    /*++

    ROUTINE: ReleaseSpinLockAtDpcLevel

    DESCRIPTION: Fast unlock,
                 Unlock the resource,
                 Skip changing CPU priority level

    ARGUMENTS: SpinLock - Pointer to the lock you want to release

    RETURNS: VOID

    --*/

    INLINE
    VOID
    ReleaseSpinLockAtDpcLevel(PKSPIN_LOCK SpinLock)
    {
        __atomic_exchange_n(SpinLock, 0, __ATOMIC_SEQ_CST);
    }

    /*++

    ROUTINE: QuerySpinLock

    DESCRIPTION: Check if a resource is currently locked,
                 DOES NOT ATTEMPT TO GRAB IT...

    ARGUMENTS: SpinLock - Pointer to the lock you want to check

    RETURNS: BOOL

    --*/

    INLINE
    BOOL
    QuerySpinLock(PKSPIN_LOCK SpinLock)
    {
        __asm__ volatile("" ::: "memory");
        return (*SpinLock != 0);
    }

} // namespace Ke