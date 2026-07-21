/*++

MODULE: Bug check codes

AUTHOR: Trollycat

ABSTRACT: Panic/BugCheck failure codes

--*/
#pragma once

#define STATUS_BUGCHECK_BASE                            0x00000000

#define APC_INDEX_MISMATCH                              0x00000001
#define DEVICE_QUEUE_NOT_BUSY                           0x00000002
#define INVALID_AFFINITY_SET                            0x00000003
#define INVALID_DATA_ACCESS_TRAP                        0x00000004
#define INVALID_PROCESS_ATTACH_ATTEMPT                  0x00000005
#define INVALID_PROCESS_DETACH_ATTEMPT                  0x00000006
#define INVALID_SOFTWARE_INTERRUPT                      0x00000007
#define IRQL_NOT_DISPATCH_LEVEL                         0x00000008
#define IRQL_NOT_GREATER_OR_EQUAL                       0x00000009
#define IRQL_NOT_LESS_OR_EQUAL                          0x0000000A
#define MAXIMUM_WAIT_OBJECTS_EXCEEDED                   0x0000000C
#define MUTEX_ALREADY_OWNED                             0x0000000D
#define SPIN_LOCK_ALREADY_OWNED                         0x0000000E
#define SPIN_LOCK_NOT_OWNED                             0x0000000F
#define TRAP_CAUSE_UNKNOWN                              0x00000012
#define EMPTY_THREAD_REAP_LIST                          0x00000013
#define CREATE_DELETE_LOCK_NOT_LOCKED                   0x00000014
#define LAST_CHANCE_CALLOUT_FAILED                      0x00000015
#define CID_HANDLE_CREATION                             0x00000016
#define CID_HANDLE_DELETION                             0x00000017
#define REFERENCE_BY_POINTER                            0x00000018
#define BAD_POOL_HEADER                                 0x00000019
#define SYSTEM_EXIT_OWNED_MUTEX                         0x0000001A
#define AUDIO_DRIVER_INTERNAL                           0x0000001B
#define MUST_SUCCEED_POOL_EMPTY                         0x0000001D
#define KMODE_EXCEPTION_NOT_HANDLED                     0x0000001E
#define KSHARED_USER_DATA_HEADER_CORRUPT                0x0000001F
#define KERNEL_APC_PENDING_DURING_EXIT                  0x00000020
#define QUOTA_UNDERFLOW                                 0x00000021
#define FILE_SYSTEM                                     0x00000022
#define FAT_FILE_SYSTEM                                 0x00000023
#define NPFS_FILE_SYSTEM                                0x00000025
#define MSFS_FILE_SYSTEM                                0x00000026
#define DATA_BUS_ERROR                                  0x0000002E
#define INSTRUCTION_BUS_ERROR                           0x0000002F
#define SET_OF_INVALID_CONTEXT                          0x00000030
#define PHASE0_INITIALIZATION_FAILED                    0x00000031
#define PHASE1_INITIALIZATION_FAILED                    0x00000032
#define UNEXPECTED_INITIALIZATION_CALL                  0x00000033
#define CACHE_MANAGER                                   0x00000034
#define NO_MORE_IRP_STACK_LOCATIONS                     0x00000035
#define DEVICE_REFERENCE_COUNT_NOT_ZERO                 0x00000036
#define FLOPPY_INTERNAL_ERROR                           0x00000037
#define SERIAL_DRIVER_INTERNAL                          0x00000038
#define SYSTEM_BOARD_HARDWARE_FAILURE                   0x00000039
#define MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED      0x0000003E
#define PAGE_FAULT_WITH_INTERRUPTS_OFF                  0x00000049
#define PAGE_FAULT_IN_NONPAGED_AREA                     0x00000050
#define REGISTRY_ERROR                                  0x00000051
#define MAILSLOT_FILE_SYSTEM                            0x00000052
#define UNEXPECTED_KERNEL_MODE_TRAP                     0x0000007F
#define SYSTEM_THREAD_EXCEPTION_NOT_HANDLED             0x0000007E
#define ACPI_BIOS_ERROR                                 0x000000A5
#define CRITICAL_INITIALIZATION_FAILURE                 0x0000013D
#define HAL_INITIALIZATION_FAILED                       0x0000005C