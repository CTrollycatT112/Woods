/*++

MODULE: Memory utils

AUTHOR: Trollycat

ABSTRACT: Bare bones memory functions so flanterm will work

--*/
#include "htbase.hpp"

EXTERN_C {
    
    PVOID 
    memset(PVOID Dest, 
           INT Ch,
           QWORD Count) 
    {
        PUCHAR Ptr = static_cast<PUCHAR>(Dest);

        for (QWORD I = 0; I < Count; I++) {
            Ptr[I] = static_cast<UCHAR>(Ch);
        }

        return Dest;
    }

    PVOID 
    memcpy(PVOID Dest,
           PCVOID Src,
           QWORD Count)
    {
        if (Count >= 512)
        {
            PVOID Temp = Dest;
            __asm__ volatile( "rep movsb" : "+D"(Dest), "+S"(Src), "+c"(Count) : : "memory");
            return Temp;
        }

        PBYTE  PDest = (PBYTE)Dest;
        PCBYTE PSrc  = (PCBYTE)Src;

        if (ALIGNOFF(PDest, sizeof(QWORD)) == ALIGNOFF(PSrc, sizeof(QWORD)))
        {
            while (!IS_ALIGNED(PDest, sizeof(QWORD)) && Count > 0)
            {
                *PDest++ = *PSrc++;
                Count--;
            }

            PQWORD  WDest = (PQWORD)PDest;
            PCQWORD WSrc  = (PCQWORD)PSrc;

            while (Count >= sizeof(QWORD))
            {
                *WDest++ = *WSrc++;
                Count -= sizeof(QWORD);
            }

            PDest = (PBYTE)WDest;
            PSrc  = (PCBYTE)WSrc;
        }

        for (QWORD I = 0; I < Count; I++)
        {
            PDest[I] = PSrc[I];
        }

        return Dest;
    }

    PVOID 
    memmove(PVOID Dest,
            PCVOID Src,
            QWORD Count)
    {
        PUCHAR DstPtr  = static_cast<PUCHAR>(Dest);
        PCUCHAR SrcPtr = static_cast<PCUCHAR>(Src);

        if (DstPtr < SrcPtr || DstPtr >= (SrcPtr + Count)) {
            for (QWORD I = 0; I < Count; I++) {
                DstPtr[I] = SrcPtr[I];
            }
        }
        else {
            for (QWORD I = Count; I > 0; I--) {
                DstPtr[I - 1] = SrcPtr[I - 1];
            }
        }

        return Dest;
    }

    INT 
    memcmp(PCVOID Lhs,
           PCVOID Rhs,
           QWORD Count)
    {
        PCUCHAR LhsPtr = static_cast<PCUCHAR>(Lhs);
        PCUCHAR RhsPtr = static_cast<PCUCHAR>(Rhs);

        for (QWORD I = 0; I < Count; I++) {
            if (LhsPtr[I] != RhsPtr[I]) {
                return (LhsPtr[I] < RhsPtr[I]) ? -1 : 1;
            }
        }

        return 0;
    }
}
