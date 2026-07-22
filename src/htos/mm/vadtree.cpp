/*++

MODULE: Virtual address descriptors

AUTHOR: Trollycat

ABSTRACT: Manages virtual address descriptor trees,
          tracks ranges of virtual memory,
          and helps allocate or free mapped regions

--*/
#include "ke/amd64/amd64.hpp"
#include "ke/spinlock.hpp"
#include "mm/mi.hpp"
#include "mm/mm.hpp"

#include "htbase.hpp"

namespace Mi
{
    
    namespace
    {
        VOID
        RotateVadLeft(PKPROCESS Process, PMM_VAD Vad)
        {
            PMM_VAD N  = Vad->Right;
            Vad->Right = N->Left;

            if (N->Left != NULL)
            {
                N->Left->Parent = Vad;
            }

            N->Parent = Vad->Parent;

            if (Vad->Parent == NULL)
            {
                Process->Vads = N;
            }
            else if (Vad == Vad->Parent->Left)
            {
                Vad->Parent->Left = N;
            }
            else
            {
                Vad->Parent->Right = N;
            }

            N->Left     = Vad;
            Vad->Parent = N;
        }

        VOID
        RotateVadRight(PKPROCESS Process, PMM_VAD Vad)
        {
            PMM_VAD N = Vad->Left;
            Vad->Left = N->Right;

            if (N->Right != NULL) {
                N->Right->Parent = Vad;
            }

            N->Parent = Vad->Parent;

            if (Vad->Parent == NULL) {
                Process->Vads = N;
            }
            else if (Vad == Vad->Parent->Right) {
                Vad->Parent->Right = N;
            }
            else {
                Vad->Parent->Left = N;
            }

            N->Right = Vad;
            Vad->Parent = N;

        }

        VOID
        FixupVadTreeAfterInsertion(PKPROCESS Process,
                                   PMM_VAD   Vad)
        {
            while (Vad->Parent != NULL &&
                   Vad->Parent->Rb == VAD_RED)
            {
                if (Vad->Parent == Vad->Parent->Parent->Left)
                {
                    PMM_VAD RelativeTo = Vad->Parent->Parent->Right;
                    if (MiIsVadRed(RelativeTo))
                    {
                        Vad->Parent->Rb         = VAD_BLACK;
                        RelativeTo->Rb          = VAD_BLACK;
                        Vad->Parent->Parent->Rb = VAD_RED;
                        Vad                     = Vad->Parent->Parent;
                    }
                    else
                    {
                        if (Vad == Vad->Parent->Right)
                        {
                            Vad = Vad->Parent;
                            RotateVadLeft(Process,Vad);
                        }

                        Vad->Parent->Rb         = VAD_BLACK;
                        Vad->Parent->Parent->Rb = VAD_RED;
                        RotateVadRight(Process, Vad->Parent->Parent);
                    }
                }
                else
                {
                    PMM_VAD Relative = Vad->Parent->Parent->Left;

                    if (MiIsVadRed(Relative))
                    {
                        Vad->Parent->Rb = VAD_BLACK;
                        Relative->Rb = VAD_BLACK;
                        Vad->Parent->Parent->Rb = VAD_RED;
                        Vad = Vad->Parent->Parent;
                    }
                    else
                    {
                        if (Vad == Vad->Parent->Left)
                        {
                            Vad = Vad->Parent;
                            RotateVadRight(Process, Vad);
                        }
                        
                        Vad->Parent->Rb = VAD_BLACK;
                        Vad->Parent->Parent->Rb = VAD_RED;
                        RotateVadLeft(Process, Vad->Parent->Parent);

                    }
                }
            }
            Process->Vads->Rb = VAD_BLACK;
        }

        VOID
        TransplantVad(PKPROCESS Process,
                      PMM_VAD   U,
                      PMM_VAD   V)
        {
            if (U->Parent == NULL)
            {
                Process->Vads = V;
            }
            else if (U == U->Parent->Left)
            {
                U->Parent->Left = V;
            }
            else
            {
                U->Parent->Right = V;
            }

            if (V != NULL)
            {
                V->Parent = U->Parent;
            }
        }

        NODISCARD
        PMM_VAD
        VadMinimum(PMM_VAD U)
        {
            while (U->Left != NULL)
            {
                U = U->Left;
            }

            return U;
        }

        VOID
        FixupVadTreeAfterDeletion(PKPROCESS Process,
                                  PMM_VAD   X,
                                  PMM_VAD   ParentX)
        {
            while (X != Process->Vads && MiIsVadBlack(X)) 
            {
                if (X == ParentX->Left) 
                {
                    PMM_VAD W = ParentX->Right;

                    if (MiIsVadRed(W))
                    {
                        W->Rb       = VAD_BLACK;
                        ParentX->Rb = VAD_RED;

                        RotateVadLeft(Process, ParentX);
                        W = ParentX->Right;
                    }

                    if (MiIsVadBlack(W->Left) && 
                        MiIsVadBlack(W->Right))
                    {
                        W->Rb   = VAD_RED;
                        X       = ParentX;
                        ParentX = X->Parent;
                    }
                    
                    else 
                    {

                        if (MiIsVadBlack(W->Right)) 
                        {
                            if (W->Left) 
                            { 
                                W->Left->Rb = VAD_BLACK; 
                            }

                            W->Rb = VAD_RED;

                            RotateVadRight(Process, W);
                            W = ParentX->Right;
                        }

                        W->Rb       = ParentX->Rb;
                        ParentX->Rb = VAD_BLACK;

                        if (W->Right) 
                        { 
                            W->Right->Rb = VAD_BLACK; 
                        }

                        RotateVadLeft(Process, ParentX);
                        X       = Process->Vads;
                        ParentX = NULL;
                    }
                }

                else 
                {
                    PMM_VAD W = ParentX->Left;

                    if (MiIsVadRed(W)) 
                    {
                        W->Rb       = VAD_BLACK;
                        ParentX->Rb = VAD_RED;
                        
                        RotateVadRight(Process, ParentX);
                        W = ParentX->Left;
                    }

                    if (MiIsVadBlack(W->Right) && 
                        MiIsVadBlack(W->Left)) 
                    {
                        W->Rb   = VAD_RED;
                        X       = ParentX;
                        ParentX = X->Parent;
                    }

                    else 
                    {
                        if (MiIsVadBlack(W->Left)) 
                        {
                            if (W->Right)
                            {
                                 W->Right->Rb = VAD_BLACK; 
                            }

                            W->Rb = VAD_RED;
                            RotateVadLeft(Process, W);
                            W = ParentX->Left;
                        }

                        W->Rb       = ParentX->Rb;
                        ParentX->Rb = VAD_BLACK;
                        
                        if (W->Left) 
                        { 
                            W->Left->Rb = VAD_BLACK; 
                        }

                        RotateVadRight(Process, ParentX);
                        X = Process->Vads;
                        ParentX = NULL;
                    }
                }
            }

            if (X != NULL) 
            {
                X->Rb = VAD_BLACK;
            }
        }

        VOID
        DeleteVadNode(PKPROCESS Process,
                      PMM_VAD   Z)
        {
            PMM_VAD Y            = Z;
            UCHAR YOriginalColor = Y->Rb;
            PMM_VAD X            = NULL;
            PMM_VAD ParentX      = NULL;

            if (Z->Left == NULL) 
            {
                X       = Z->Right;
                ParentX = Z->Parent;
                TransplantVad(Process, Z, Z->Right);
            }

            else if (Z->Right == NULL) 
            {
                X       = Z->Left;
                ParentX = Z->Parent;
                TransplantVad(Process, Z, Z->Left);
            }

            else 
            {
                Y              = VadMinimum(Z->Right);
                YOriginalColor = Y->Rb;
                X              = Y->Right;

                if (Y->Parent == Z) 
                {
                    ParentX = Y;
                }

                else 
                {
                    ParentX = Y->Parent;
                    TransplantVad(Process, Y, Y->Right);
                    Y->Right         = Z->Right;
                    Y->Right->Parent = Y;
                }

                TransplantVad(Process, Z, Y);
                Y->Left         = Z->Left;
                Y->Left->Parent = Y;
                Y->Rb           = Z->Rb;
            }

            if (YOriginalColor == VAD_BLACK) 
            {
                FixupVadTreeAfterDeletion(Process, X, ParentX);
            }
        }

        VOID
        DumpVadNode(PMM_VAD Vad)
        {
            if (Vad == NULL)
            {
                return;
            }

            DumpVadNode(Vad->Left);

            Rtl::Print("%p - %p type %x %s",
                        Vad->Start,
                        Vad->End,
                        Vad->Type,
                        Vad->Rb == VAD_RED ? "(r)" : "(b)");

            DumpVadNode(Vad->Right);
        }
    } // namespace


    NODISCARD
    PMM_VAD
    AllocateVad(ULONG64 Start,
                ULONG64 End,
                UCHAR   Type)
    {
        PMM_VAD Vad = (PMM_VAD)Mm::AllocatePoolWithTag(NonPagedPoolZeroed,
                                              sizeof(MM_VAD),
                                              MAKE_TAG('D', ' ', 'A', 'V'));

        if (!Vad)
        {
            return NULL;
        }

        Vad->Start  = Start;
        Vad->End    = End;
        Vad->Type   = Type;
        Vad->Left   = NULL;
        Vad->Right  = NULL;
        Vad->Parent = NULL;
        Vad->Rb     = VAD_RED;

        return Vad;
    }

    VOID
    FreeVad(PMM_VAD Vad)
    {
        Mm::FreePool(Vad, MAKE_TAG('D', ' ', 'A', 'V'));
    }

    VOID
    InsertVad(PKPROCESS Process,
              PMM_VAD   Vad)
    {
        KIRQL PreviousIrql = 0;
        Ke::AcquireSpinLock(&Process->VadLock,&PreviousIrql);

        PMM_VAD Find    = NULL;
        PMM_VAD Search  = Process->Vads;

        while (Search != NULL)
        {
            Find = Search;
            if (Vad->Start <
                Search->Start)
            {
                Search = Search->Left;
            }
            else
            {
                Search = Search->Right;
            }
        }

        Vad->Parent = Find;

        if (Find == NULL)
        {
            Process->Vads = Vad;
        }
        else if (Vad->Start < Find->Start)
        {
            Find->Left = Vad;
        }
        else
        {
            Find->Right = Vad;
        }

        FixupVadTreeAfterInsertion(Process,Vad);
        Ke::ReleaseSpinLock(&Process->VadLock,PreviousIrql);
    }

    NODISCARD
    PMM_VAD
    FindVadByAddress(PKPROCESS Process,
                     ULONG64   Address)
    {
        KIRQL PreviousIrql = 0;
        Ke::AcquireSpinLock(&Process->VadLock,
                            &PreviousIrql);

        PMM_VAD Search = Process->Vads;
        while (Search != NULL) 
        {
            if (Address >= Search->Start && Address < Search->End) 
            {
                break;
            }
            else if (Address < Search->Start) 
            {
                Search = Search->Left;
            }
            else 
            {
                Search = Search->Right;
            }
        }

        Ke::ReleaseSpinLock(&Process->VadLock, PreviousIrql);
        return Search;
    }

    VOID
    RemoveVadByAddress(PKPROCESS Process,
                       ULONG64   Address)
    {
        KIRQL PreviousIrql = 0;
        Ke::AcquireSpinLock(&Process->VadLock,&PreviousIrql);

        PMM_VAD Vad = Process->Vads;

        while (Vad != NULL)
        {
            if (Address >= Vad->Start &&
                Address <  Vad->End)
            {
                break;
            }
            else if (Address < Vad->Start)
            {
                Vad = Vad->Left;
            }
            else
            {
                Vad = Vad->Right;
            }
        }

        if (Vad != NULL)
        {
            DeleteVadNode(Process,Vad);
            FreeVad(Vad);
        }

        Ke::ReleaseSpinLock(&Process->VadLock,PreviousIrql);
    }

    VOID
    DumpVads(PKPROCESS Process)
    {
        KIRQL PreviousIrql = 0;

        Ke::AcquireSpinLock(&Process->VadLock,
                            &PreviousIrql);

        if (Process->Vads == NULL)
        {
            Rtl::Print("DUMP: NO VADS FOUND...");
        }
        else
        {
            DumpVadNode(Process->Vads);
        }

        Ke::ReleaseSpinLock(&Process->VadLock,PreviousIrql);
    }
} // namespace Mi