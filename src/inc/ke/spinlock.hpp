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

    INLINE
    VOID
    ReleaseSpinLock(PKSPIN_LOCK SpinLock, 
                    KIRQL PreviousIrql)
    {
        __sync_lock_release(SpinLock);
        Ke::LowerIrql(PreviousIrql);
    }


    INLINE
    VOID
    AcquireSpinLockAtDpcLevel(PKSPIN_LOCK SpinLock)
    {
        while (__sync_val_compare_and_swap(SpinLock, 0, 1) != 0)
        {
            __builtin_ia32_pause();
        }
    }

    INLINE
    VOID
    ReleaseSpinLockAtDpcLevel(PKSPIN_LOCK SpinLock)
    {
        __atomic_exchange_n(SpinLock, 0, __ATOMIC_SEQ_CST);
    }

    INLINE
    BOOL
    QuerySpinLock(PKSPIN_LOCK SpinLock)
    {
        __asm__ volatile("" ::: "memory");
        return (*SpinLock != 0);
    }

} // namespace Ke