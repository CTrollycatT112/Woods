/*++

MODULE: Page Table

AUTHOR: Trollycat

ABSTRACT: Handles page-based memory management.
          Virtual-to-physical with (MMU),
          Page table entries (PTE)

--*/
#include "mm/mi.hpp"

typedef struct _PAGE_WALK_RESULT
{
    PPMLE Entry;
    ULONG TargetLevel;
} PAGE_WALK_RESULT, *PPAGGE_WALK_RESULT;

namespace Mm
{
    BOOLEAN
    IsAddressValid(ULONG64 Virtual)
    {
        return Mi::IsAddressPresent((PPMLE)MiGetAddressSpace(),
                                    Virtual);
    }

    VOID
    FlushAddress(ULONG64 Address)
    {
        __asm__ volatile("invlpg (%0)" : : "r"(Address) : "memory");
    }
} // namespace Mm

namespace Mi
{
    namespace
    {
        NODISCARD
        INLINE
        PAGE_WALK_RESULT
        WalkToLevel(PPMLE   AddressSpace, 
                    ULONG64 Virtual, 
                    ULONG   StopLevel, 
                    BOOLEAN Alloc)
        {
            const ULONG64 Levels[5] = {
                0,
                MiIndexLevel1(Virtual),
                MiIndexLevel2(Virtual),
                MiIndexLevel3(Virtual),
                MiIndexLevel4(Virtual)
            };

            PPMLE CurrentTable = AddressSpace;

            for (ULONG CurrentLevel = 4; CurrentLevel > StopLevel; --CurrentLevel)
            {
                PPMLE NextTable = Mi::PageTableToEntry(CurrentTable, Levels[CurrentLevel], Alloc);
                if (NextTable == nullptr)
                {
                    return { nullptr, CurrentLevel };
                }

                if (CurrentLevel == 2 && CurrentTable[Levels[2]].LargePage)
                {
                    return { &CurrentTable[Levels[2]], 2 };
                }

                CurrentTable = NextTable;
            }

            return { &CurrentTable[Levels[StopLevel]], StopLevel };
        }
    } // namespace

    VOID
    UnmapPage(PPMLE AddressSpace,
              ULONG64 Virtual)
    {
        PAGE_WALK_RESULT Result = WalkToLevel(AddressSpace, 
                                              Virtual,
                                               1,
                                                 FALSE);
        if (Result.Entry != NULL)
        {
            Result.Entry->Long = 0;
        }
    }

    NODISCARD
    ULONG64
    GetPhysicalForVirtual(PPMLE AddressSpace,
                          ULONG64 Virtual)
    {
        PAGE_WALK_RESULT Result = WalkToLevel(AddressSpace,
                                              Virtual,
                                              1,
                                              FALSE);
        
        if (Result.Entry == NULL || Result.TargetLevel == 2)
        {
            return 0;
        }
        
        return Result.Entry->PageFrameNumber * PAGE_SIZE;
    }

    NODISCARD
    BOOLEAN
    IsAddressPresent(PPMLE   AddressSpace,
                     ULONG64 Virtual)
    {
        PAGE_WALK_RESULT Result = WalkToLevel(AddressSpace,
                                              Virtual,
                                              1,
                                              FALSE);
            
        return (Result.Entry != NULL && Result.Entry->Present);
    }

    NODISCARD
    BOOLEAN
    MapLargePage(PPMLE   AddressSpace,
                 ULONG64 Virtual,
                 ULONG64 Physical, 
                 BOOLEAN Write, 
                 BOOLEAN User, 
                 BOOLEAN Executable)
    {
        PAGE_WALK_RESULT Result = WalkToLevel(AddressSpace, 
                                              Virtual,
                                             2, 
                                             TRUE);
            
        if (Result.Entry == NULL)
        {
            return FALSE;
        }

        Result.Entry->PageFrameNumber = Physical / PAGE_SIZE;
        Result.Entry->Present         = 1;
        Result.Entry->Write           = Write;
        Result.Entry->User            = User;
        Result.Entry->ExecuteDisable  = !Executable;
        Result.Entry->LargePage       = 1;
        return TRUE;
    }

    NODISCARD
    BOOLEAN
    MapPage(PPMLE   AddressSpace, 
            ULONG64 Virtual, 
            ULONG64 Physical, 
            BOOLEAN Write, 
            BOOLEAN User, 
            BOOLEAN Executable)
    {
        PAGE_WALK_RESULT Result = WalkToLevel(AddressSpace, 
                                              Virtual,
                                             1, 
                                             TRUE);

        if (Result.Entry == NULL)
        {
            return FALSE;
        }

        Result.Entry->PageFrameNumber = Physical / PAGE_SIZE;
        Result.Entry->Present         = 1;
        Result.Entry->Write           = Write;
        Result.Entry->User            = User;
        Result.Entry->ExecuteDisable  = !Executable;
        return TRUE;
    }

    NODISCARD
    BOOLEAN
    FlagPage(PPMLE   AddressSpace, 
             ULONG64 Virtual,
             BOOLEAN Read, 
             BOOLEAN Write, 
             BOOLEAN User, 
             BOOLEAN Executable)
    {
        PAGE_WALK_RESULT Result = WalkToLevel(AddressSpace, 
                                              Virtual,
                                        1, 
                                        FALSE);

        if (Result.Entry == NULL)
        {
            return FALSE;
        }

        Result.Entry->Present        = Read;
        Result.Entry->Write          = Write;
        Result.Entry->User           = User;
        Result.Entry->ExecuteDisable = !Executable;
        return TRUE;
    }
} // namespace Mi