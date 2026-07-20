/*++

MODULE: Architecture definitions

AUTHOR: Trollycat

ABSTRACT: X86_64 only definitions

--*/
#pragma once

#include "htbase.hpp"

//
// CR0 REGISTER
//
#define CR0_MP (1ULL << 1)
#define CR0_EM (1ULL << 2)

//
// CR4 REGISTER
//
#define CR4_OSFXSR       (1ULL << 9)
#define CR4_OSXMMEXCPT   (1ULL << 10)

//
// ACCESS MASKS
//
#define D_ACC_VALID   (1U << 7)
#define D_ACC_RING0   (0U << 5)
#define D_ACC_RING3   (3U << 5)
#define D_ACC_CODE    (1U << 4)
#define D_ACC_EXEC    (1U << 3)
#define D_ACC_WRITE   (1U << 1)

#define D_GRAN_LONG   (1U << 5)

//
// GDT ACCESS MASKS
//
#define GDT_KERNEL_CODE_ACCESS  (D_ACC_VALID | D_ACC_RING0 | D_ACC_CODE | D_ACC_EXEC | D_ACC_WRITE)
#define GDT_KERNEL_DATA_ACCESS  (D_ACC_VALID | D_ACC_RING0 | D_ACC_CODE | D_ACC_WRITE)
#define GDT_USER_CODE_ACCESS    (D_ACC_VALID | D_ACC_RING3 | D_ACC_CODE | D_ACC_EXEC | D_ACC_WRITE)
#define GDT_USER_DATA_ACCESS    (D_ACC_VALID | D_ACC_RING3 | D_ACC_CODE | D_ACC_WRITE)

//
// GDT CONSTANTS 
//
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE   0x20
#define GDT_USER_DATA   0x18

//
// TSS AND CALL GATES
//
#define SEGMENT_TYPE_TSS       0b1001
#define SEGMENT_TYPE_CALL_GATE 0b1100
#define TSS_FLAGS0_PRESENT     (1U << 7)

//
// GDT INDEXES
//
#define INDEX_GDT_KERNEL_CODE (GDT_KERNEL_CODE >> 3)
#define INDEX_GDT_KERNEL_DATA (GDT_KERNEL_DATA >> 3)
#define INDEX_GDT_USER_DATA   (GDT_USER_DATA >> 3)
#define INDEX_GDT_USER_CODE   (GDT_USER_CODE >> 3)

typedef struct PACKED _KDESCRIPTOR_TABLE_PTR
{
    USHORT  Limit;
    ULONG64 Base;
} KDESCRIPTOR_TABLE_PTR, *PKDESCRIPTOR_TABLE_PTR;

typedef struct PACKED _KDESCRIPTOR_TABLE_DESCRIPTOR
{
    USHORT Limit;
    USHORT BaseLow;
    UCHAR  BaseMid;
    UCHAR  Access;
    UCHAR  Granularity;
    UCHAR  BaseHigh;
} KDESCRIPTOR_TABLE_DESCRIPTOR, *PKDESCRIPTOR_TABLE_DESCRIPTOR;

typedef struct PACKED _KTASK_DESCRIPTOR_TABLE_DESCRIPTOR {
    USHORT Length;
    USHORT BaseLow;
    UCHAR  BaseMid;
    UCHAR  Flags0;
    UCHAR  Flags1;
    UCHAR  BaseHigh;
    ULONG  BaseUpper32;
    ULONG  Reserved;
} KTASK_DESCRIPTOR_TABLE_DESCRIPTOR, * PKTASK_DESCRIPTOR_TABLE_DESCRIPTOR;

typedef struct PACKED _KTASK_STATE
{
    ULONG32 Reserved0;
    ULONG64 Rsp0;
    ULONG64 Rsp1;
    ULONG64 Rsp2;
    ULONG64 Reserved1;
    ULONG64 Ist[7];
    ULONG64 Reserved2;
    USHORT  Reserved3;
    USHORT  IopbOffset;
} KTASK_STATE, *PKTASK_STATE;

typedef struct PACKED _KDESCRIPTOR_TABLE
{
    KDESCRIPTOR_TABLE_DESCRIPTOR Entries[5];
    KTASK_DESCRIPTOR_TABLE_DESCRIPTOR Tss;
} KDESCRIPTOR_TABLE, *PKDESCRIPTOR_TABLE;

/*++

ROUTINE: Ke386SetTr

DESCRIPTION: Load task register with a selector

ARGUMENTS: Selector - Segment index of TSS

RETURNS: VOID

--*/

INLINE
VOID
Ke386SetTr(USHORT Selector)
{
    __asm__ volatile("ltr %0" : : "r"(Selector));
}

/*++

ROUTINE: Ke386GetTr

DESCRIPTION: Get the current task register selector

ARGUMENTS: N/A

RETURNS: USHORT

--*/

NODISCARD
INLINE
USHORT
Ke386GetTr()
{
    USHORT Selector;
    __asm__ volatile("str %0" : "=r"(Selector));
    return Selector;
}

/*++

ROUTINE: Ke386ReadCr0

DESCRIPTION: Read the CR0 register

ARGUMENTS: N/A

RETURNS: ULONG64

--*/
NODISCARD
INLINE
ULONG64
Ke386ReadCr0()
{
    ULONG64 Value;
    __asm__ volatile("mov %%cr0, %0" : "=r"(Value));
    return Value;
}

/*++

ROUTINE: Ke386WriteCr0

DESCRIPTION: Write a value to CR0 register

ARGUMENTS: Value - The new CR0 mask

RETURNS: VOID

--*/
INLINE
VOID
Ke386WriteCr0(ULONG64 Value)
{
    __asm__ volatile("mov %0, %%cr0" : : "r"(Value));
}

/*++

ROUTINE: Ke386ReadCr4

DESCRIPTION: Read the current CR4 register

ARGUMENTS: N/A

RETURNS: ULONG64

--*/
NODISCARD
INLINE
ULONG64
Ke386ReadCr4(VOID)
{
    ULONG64 Value;
    __asm__ volatile("mov %%cr4, %0" : "=r"(Value));
    return Value;
}

/*++

ROUTINE: Ke386WriteCr4

DESCRIPTION: Write a value to CR4

ARGUMENTS: Value - The new CR4 mask

RETURNS: N/A

--*/
INLINE
VOID
Ke386WriteCr4(ULONG64 Value)
{
    __asm__ volatile("mov %0, %%cr4" : : "r"(Value));
}

/*++

ROUTINE: Ke386Lgdt

DESCRIPTION: Loads the GDT register

ARGUMENTS: TablePtr - Pointer to KDESCRIPTOR_TABLE_PTR

RETURNS: N/A

--*/
INLINE
VOID
Ke386Lgdt(PKDESCRIPTOR_TABLE_PTR TablePtr)
{
    __asm__ volatile("lgdt (%0)" : : "r"(TablePtr) : "memory");
}

/*++

ROUTINE: KiFlushGdt

DESCRIPTION: Assembly GDT reset

ARGUMENTS: N/A

RETURNS: VOID

--*/
EXTERN_C
VOID
KiFlushGdt();

namespace Ki
{
    /*++

    MODULE: InitializeGdt

    AUTHOR: Trollycat

    ABSTRACT: Initialize the amd64 (x86_64) GDT

    --*/
    VOID
    InitializeGdt();
} // namespace Ki