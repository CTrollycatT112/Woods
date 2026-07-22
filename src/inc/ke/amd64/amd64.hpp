/*++

MODULE: Architecture definitions

AUTHOR: Trollycat

ABSTRACT: X86_64 only definitions

--*/
#pragma once

#include "htbase.hpp"
#include "httypes.hpp"

typedef struct _MM_VAD MM_VAD, *PMM_VAD;

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
// TSS AND SEGMENTS
//
#define SEGMENT_TYPE_TSS                0b1001
#define SEGMENT_TYPE_CALL_GATE          0b1100
#define SEGMENT_TYPE_INTERRUPT_GATE     0b1110
#define SEGMENT_TYPE_TRAP_GATE          0b1111
#define TSS_FLAGS0_PRESENT             (1U << 7)

//
// GDT INDEXES
//
#define INDEX_GDT_KERNEL_CODE (GDT_KERNEL_CODE >> 3)
#define INDEX_GDT_KERNEL_DATA (GDT_KERNEL_DATA >> 3)
#define INDEX_GDT_USER_DATA   (GDT_USER_DATA >> 3)
#define INDEX_GDT_USER_CODE   (GDT_USER_CODE >> 3)

//
// ACPI
//
#define ACPI_MADT_TYPE_PROCESSOR_LOCAL_APIC             0
#define ACPI_MADT_TYPE_IO_APIC                          1
#define ACPI_MADT_TYPE_INTERRUPT_SOURCE_OVERRIDE_APIC   2
#define ACPI_MADT_TYPE_NON_MASKABLE_INTERRUPT_APIC      4
#define ACPI_MADT_TYPE_LOCAL_APIC_ADDRESS_OVERRIDE_APIC 5

//
// MADT
//
#define ACPI_MADT_TYPE_PROCESSOR_LOCAL_APIC             0
#define ACPI_MADT_TYPE_IO_APIC                          1
#define ACPI_MADT_TYPE_INTERRUPT_SOURCE_OVERRIDE_APIC   2
#define ACPI_MADT_TYPE_NON_MASKABLE_INTERRUPT_APIC      4
#define ACPI_MADT_TYPE_LOCAL_APIC_ADDRESS_OVERRIDE_APIC 5

//
// IRQLs
//
#define PASSIVE_LEVEL   0
#define APC_LEVEL       1
#define DISPATCH_LEVEL  2
#define HIGH_LEVEL      3
#define INPUT_LEVEL     6
#define DISK_LEVEL      7

typedef struct _KPRCB KPRCB, *PKPRCB;
typedef struct _KDPC KDPC, *PKDPC;
typedef struct _KPROCESS KPROCESS, *PKPROCESS;

typedef ULONG64 KIRQL, *PKIRQL;
typedef VOLATILE ULONG64 KSPIN_LOCK, *PKSPIN_LOCK;

typedef VOID(*PKDEFFERED_ROUTINE)(PKDPC Dpc, PVOID DefferedContext);

typedef struct PACKED _KDPC
{
    ULONG64            ProcessorNumber;
    PKDEFFERED_ROUTINE DefferedRoutine;
    PVOID              DefferedContext;
    BOOLEAN            Completed;
    LIST_ENTRY         DpcQueue;
} KDPC, *PKDPC;

typedef struct _KPRCB
{
    PVOID       Self;
    LIST_ENTRY  DpcQueueHead;
    KSPIN_LOCK  DpcLock;
    ULONG64     DpcQueueDepth;
    ULONG64     InterruptCount;
    ULONG64     ProcessorId;
    ULONG64     LocalApicId;

    ULONG       ProcessorNumber;
    BOOLEAN     DpcInterruptRequested;
    UCHAR       Padding[3];
    
    struct
    {
        ULONG64 Rsp[1];
    } TaskState;
} KPRCB, *PKPRCB;

typedef struct _KPROCESS
{
    ULONG64    ProcessId;
    ULONG64    ProcessStatus;
    LIST_ENTRY ProcessLinks;

    ULONG64    DirectoryBase;
    KSPIN_LOCK VadLock;
    PMM_VAD    Vads;
    KSPIN_LOCK WorkingSetLock;
    ULONG64    UserRegionHint;

    ULONG64     ThreadCount;
    PLIST_ENTRY ThreadList;
    KSPIN_LOCK  ThreadListLock;

    ULONG64 DllRegionhint;

    WCHAR Name[128];
} KPROCESS, *PKPROCESS;

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

typedef struct PACKED _KINTERRUPT_DESCRIPTOR
{
    struct
    {
        ULONG64 OffsetLow           : 16;
        ULONG64 SegmentSelector     : 16;
        ULONG64 Ist                 : 3;
        ULONG64 Reserved            : 5;
        ULONG64 Type                : 4;
        ULONG64 Reserved1           : 1;
        ULONG64 PrivilegeLevel      : 2;
        ULONG64 Present             : 1;
        ULONG64 OffsetMid           : 16;
        ULONG64 OffsetHigh          : 32;
        ULONG64 Reserved2           : 32;
    };
} KINTERRUPT_DESCRIPTOR, *PKINTERRUPT_DESCRIPTOR;

typedef struct PACKED _KINTERRUPT_DESCRIPTOR_PTR
{
    USHORT  Size;
    ULONG64 Address;
} KINTERRUPT_DESCRIPTOR_PTR, *PKINTERRUPT_DESCRIPTOR_PTR;

typedef UCHAR KX86_REG[10];
typedef UCHAR KXMM_REG[16];

typedef union PACKED _KMMX_REG
{
    struct
    {
        KX86_REG St;
        UCHAR    StReserved[6];
    };

    struct
    {
        UCHAR   MmValue[8];
        UCHAR   MmReserved[8];
    };
} KMMX_REG, *PKMMX_REG;

typedef struct PACKED _KFXSAVE64
{
    USHORT   Control;
    USHORT   Status;
    UCHAR    Tag;
    UCHAR    Reserved1;
    USHORT   Opcode;
    ULONG64  Ip64;
    ULONG64  Dp64;
    ULONG32  Mxcsr;
    ULONG32  MxcsrMask;
    KMMX_REG MmxReg[8];
    KXMM_REG XmmReg[16];
    UCHAR    Reserved2[48];
    UCHAR    Available[48];
} KFXSAVE64, *PKFXSAVE64;

static_assert(sizeof(KFXSAVE64) == 512, "KFXSAVE64 must match the real FXSAVE area size");

typedef struct PACKED _KTRAP_FRAME
{
    KFXSAVE64 Fxsave64;
    ULONG64   Align2;
    ULONG64   Align1;
    ULONG64   Align0;
    ULONG64   Dr7;
    ULONG64   Dr6;
    ULONG64   Dr3;
    ULONG64   Dr2;
    ULONG64   Dr1;
    ULONG64   Dr0;
    ULONG64   Cr3;
    ULONG64   SegGs;
    ULONG64   SegFs;
    ULONG64   SegEs;
    ULONG64   SegDs;
    ULONG64   R15;
    ULONG64   R14;
    ULONG64   R13;
    ULONG64   R12;
    ULONG64   R11;
    ULONG64   R10;
    ULONG64   R9;
    ULONG64   R8;
    ULONG64   Rdi;
    ULONG64   Rsi;
    ULONG64   Rbp;
    ULONG64   Rdx;
    ULONG64   Rcx;
    ULONG64   Rbx;
    ULONG64   Rax;
    ULONG64   Interrupt;
    ULONG64   Error;
    ULONG64   Rip;
    ULONG64   SegCs;
    ULONG64   EFlags;
    ULONG64   Rsp;
    ULONG64   SegSs;
} KTRAP_FRAME, *PKTRAP_FRAME;

typedef struct PACKED ACPI_HEADER_ {
    CHAR    Signature[4];
    ULONG32 Length;
    UCHAR   Revision;
    UCHAR   Checksum;
    CHAR    Oem[6];
    CHAR    OemTable[8];
    ULONG32 OemRevision;
    ULONG32 CreatorId;
    ULONG32 CreatorRevision;
} ACPI_HEADER, *PACPI_HEADER;

typedef struct PACKED _ACPI_RSDT {
    ACPI_HEADER Header;
    UCHAR       Tables[];
} ACPI_RSDT, *PACPI_RSDT;

typedef struct PACKED _ACPI_RSDP {
    CHAR Signature[8];
    UCHAR Checksum;
    CHAR Oem[6];
    UCHAR Revision;
    ULONG32 Rsdt;
    ULONG32 Length;
    ULONG64 Xsdt;
    UCHAR ChecksumEx;
} ACPI_RSDP, *PACPI_RSDP;


typedef struct PACKED ACPI_HPET_TABLE_ {
    ACPI_HEADER Header;
    UCHAR   HardwareRevision;
    UCHAR   ComparatorCount    : 5;
    UCHAR   CounterSize        : 1;
    UCHAR   Reserved           : 1;
    UCHAR   LegacyReplacement  : 1;
    USHORT  PciVendorId;
    UCHAR   AddressSpaceId;
    UCHAR   RegisterBitWidth;
    UCHAR   RegisterBitOffset;
    UCHAR   Reserved0;
    ULONG64 Address;
    UCHAR   HpetNumber;
    USHORT  MinimumTick;
    UCHAR   PageProtection;
} ACPI_HPET_TABLE, *PACPI_HPET_TABLE;

typedef struct PACKED _ACPI_HPET {
    VOLATILE ULONG64          GeneralCapabilities;
    ULONG64                   Reserved1;
    VOLATILE ULONG64          GeneralConfiguration;
    ULONG64                   Reserved2;
    VOLATILE ULONG64          GeneralInterruptStatus;
    ULONG64                   Reserved3;
    ULONG64                   Reserved4[24];
    VOLATILE ULONG64          CounterValue;
    ULONG64          Reserved5;
} ACPI_HPET, *PACPI_HPET;

typedef struct PACKED _MADT_HEADER {
    UCHAR       EntryType;
    UCHAR       RecordLength;
} MADT_HEADER, *PMADT_HEADER;

typedef struct PACKED _MADT {
    ACPI_HEADER  Header;
    ULONG        LocalApicAddress;
    ULONG        Flags;
    UCHAR        Entry0;
} MADT, *PMADT;

typedef struct PACKED _MADT_PROCESSOR_LOCAL_APIC {
    MADT_HEADER Header;
    UCHAR       AcpiProcessorId;
    UCHAR       ApicId;
    ULONG       Flags;
} MADT_PROCESSOR_LOCAL_APIC, *PMADT_PROCESSOR_LOCAL_APIC;

typedef struct PACKED _MADT_IO_APIC {
    MADT_HEADER Header;
    UCHAR       IoApicId;
    UCHAR       Reserved;
    ULONG       IoApicAddress;
    ULONG       GlobalSystemInterruptBase;
} MADT_IO_APIC, *PMADT_IO_APIC;

typedef struct PACKED _MADT_INTERRUPT_SOURCE_OVERRIDE_APIC {
    MADT_HEADER Header;
    UCHAR       BusSource;
    UCHAR       IrqSource;
    ULONG       GlobalSystemInterrupt;
    USHORT      Flags;
} MADT_INTERRUPT_SOURCE_OVERRIDE_APIC, *PMADT_INTERRUPT_SOURCE_OVERRIDE_APIC;

typedef struct PACKED _MADT_NON_MASKABLE_INTERRUPT_APIC {
    MADT_HEADER Header;
    UCHAR       AcpiProcessorId;
    USHORT      Flags;
    UCHAR       LocalInterrupt;
} MADT_NON_MASKABLE_INTERRUPT_APIC, *PMADT_NON_MASKABLE_INTERRUPT_APIC;

typedef struct _MADT_LOCAL_APIC_ADDRESS_OVERRIDE {
    MADT_HEADER Header;
    USHORT      Reserved;
    ULONG64     LocalApicAddress;
} MADT_LOCAL_APIC_ADDRESS_OVERRIDE, * PMADT_LOCAL_APIC_ADDRESS_OVERRIDE;

typedef struct _OBJECT_ATTRIBUTES
{
    UNICODE_STRING Name;
    PVOID          Parent;
    ULONG          Attributes;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _FILE_OBJECT
{
    PVOID          FsContext;
    PVOID          FsContext2;
    ULONG64        CurrentByteOffset;
    UNICODE_STRING FileName;
    ULONG          Flags;
} FILE_OBJECT, *PFILE_OBJECT;

EXTERN PACPI_RSDT HalAcpiRsdt;
EXTERN ULONG64    MmPhysicalOffset;

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