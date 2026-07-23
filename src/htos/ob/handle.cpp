/*++

MODULE: Object manager

AUTHOR: Trollycat

ABSTRACT: This module handles
          Opening/closing handles to an object

--*/
#include "httypes.hpp"
#include "ob/ob.hpp"
#include "mm/mm.hpp"

#include "ke/amd64/amd64.hpp"

EXTERN PKPROCESS KpKernelProcessInstance;

namespace Ob
{
    NODISCARD
    HTSTATUS
    OpenObjectByPointer(PVOID           Object, 
                        ACCESS_MASK     DesiredAccess, 
                        POBJECT_TYPE    ObjectType, 
                        KPROCESSOR_MODE AccessMode, 
                        PHANDLE         Handle)
    {
        POBJECT_HEADER ObjectHeader = ObGetHeaderFromObject(Object);

        //
        // CHECK FOR TYPE MISMATCH
        //
        if (ObjectType && ObjectHeader->Type != ObjectType)
        {
            return STATUS_OBJECT_TYPE_MISMATCH;
        }
        
        //
        // SECURITY CHECK:
        //      CHECK IF THIS IS KERNEL MODE
        //      IF NOT, ACCESS WILL BE DENIED
        //
        if (((ObjectHeader->Attributes.Attributes & OBJ_KERNEL) == 
            OBJ_KERNEL) &&
            AccessMode != KernelMode)
        {
            return STATUS_ACCESS_DENIED;
        }

        //
        // ATTEMPT TO ALLOCATE HANDLE ENTRY
        //
        PHANDLE_TABLE_ENTRY Entry = (PHANDLE_TABLE_ENTRY)
        Mm::AllocatePoolWithTag(
            NonPagedPool,
            sizeof(HANDLE_TABLE_ENTRY),
            MAKE_TAG('H', 'n', 'd', 'l')
        );

        //
        // POOL COULD NOT ALLOCATE
        //
        if (Entry == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        PKPROCESS OwnerProcess = KpKernelProcessInstance;

        // TODO:
        // NSG650 DID AN INTERESTING HACK HERE
        // INSTEAD OF A GLOBAL ARRAY, THE HANDLE_TABLE IS A LINKED LIST
        // THIS IS A HUGE SECURITY RISK
        // CASTING THIS TO A HANDLE IS A REALLY BAD IDEA
        // FIX THIS AFTER THIS IS VERY IMPORTANT
        // OTHERWISE ANY USER APP COULD TAKE OVER THE SYSTEM
        Entry->Access   = DesiredAccess;
        Entry->Owner    = OwnerProcess;
        Entry->Handle   = (HANDLE)Entry;
        Entry->Object   = Object;

        __atomic_fetch_add(&ObjectHeader->HandleCount, 1, __ATOMIC_SEQ_CST);
        ReferenceObject(Object);

        if (OwnerProcess->HandleTable.HandleCount == 0)
        {
            KeInitializeHeadList(&Entry->HandleLinks);
            OwnerProcess->HandleTable.HandleList = &Entry->HandleLinks;
        }
        else
        {
            KeInsertTailList(OwnerProcess->HandleTable.HandleList, &Entry->HandleLinks);
        }

        OwnerProcess->HandleTable.HandleCount++;

        if (ObjectHeader->Type->ObjectInitializer.OpenMethod)
        {
            OB_OPEN_REASON Reason           = ObCreateHandle;
            PVOID          TargetProcess    = OwnerProcess;
            PVOID          RawObject        = Object;
            DWORD          AccessMask       = DesiredAccess;
            DWORD          CurrentCount     = ObjectHeader->HandleCount;

            ObjectHeader->Type->ObjectInitializer.OpenMethod(
                Reason,
                TargetProcess,
                RawObject,
                AccessMask,
                CurrentCount
            );
        }

        *Handle = Entry->Handle;
        return STATUS_SUCCESS;
    }

    NODISCARD
    HTSTATUS
    CloseHandle(HANDLE Handle)
    {
        PVOID               Object                = NULL;
        PKPROCESS           CallingProcess        = KpKernelProcessInstance;
        PHANDLE_TABLE_ENTRY HandleEntry           = (PHANDLE_TABLE_ENTRY)Handle;

        if (Handle == KpKernelProcessInstance)
        {
            Object = KpKernelProcessInstance;
        }
        else
        {
            if (HandleEntry->Owner != CallingProcess)
            {
                return STATUS_INVALID_HANDLE;
            }

            Object = HandleEntry->Object;
        }

        POBJECT_HEADER ObjectHeader = ObGetHeaderFromObject(Object);

        if (ObjectHeader->Type->ObjectInitializer.CloseMethod)
        {
            PVOID TargetProcess             = HandleEntry->Owner;
            PVOID RawObject                 = HandleEntry->Object;
            DWORD AccessMask                = HandleEntry->Access;

            DWORD ProcessHandleCount        = ObjectHeader->HandleCount;
            DWORD SystemHandleCount         = ObjectHeader->HandleCount;

            ObjectHeader->Type->ObjectInitializer.CloseMethod(
                TargetProcess,
                RawObject,
                AccessMask,
                ProcessHandleCount,
                SystemHandleCount
            );
        }

        if (Handle != KpKernelProcessInstance)
        {
            KeRemoveList(&HandleEntry->HandleLinks);
            CallingProcess->HandleTable.HandleCount--;
        }

        __atomic_fetch_sub(&ObjectHeader->HandleCount, 1, __ATOMIC_SEQ_CST);
        DereferenceObject(Object);

        Mm::FreePool(HandleEntry,MAKE_TAG('H', 'n', 'd', 'l'));

        return STATUS_SUCCESS;
    }

    VOID
    CloseAllHandles(PKPROCESS Process)
    {
        if (Process->HandleTable.HandleList && 
            Process->HandleTable.HandleList->Flink != Process->HandleTable.HandleList)
        {
            PLIST_ENTRY ListHead = Process->HandleTable.HandleList;
            PLIST_ENTRY Current  = ListHead->Flink;

            while (Current != ListHead)
            {
                PHANDLE_TABLE_ENTRY HandleEntry = CONTAINING_RECORD(
                    Current, HANDLE_TABLE_ENTRY, HandleLinks
                );

                PLIST_ENTRY Next = Current->Flink;

                PVOID          Object       = HandleEntry->Object;
                POBJECT_HEADER ObjectHeader = ObGetHeaderFromObject(Object);

                if (ObjectHeader->Type->ObjectInitializer.CloseMethod) 
                {
                    PVOID TargetProcess      = HandleEntry->Owner;
                    PVOID RawObject          = HandleEntry->Object;
                    DWORD AccessMask         = HandleEntry->Access;
                    DWORD ProcessHandleCount = ObjectHeader->HandleCount;
                    DWORD SystemHandleCount  = ObjectHeader->HandleCount;

                    ObjectHeader->Type->ObjectInitializer.CloseMethod(
                        TargetProcess,
                        RawObject,
                        AccessMask,
                        ProcessHandleCount,
                        SystemHandleCount
                    );
                }

                __atomic_fetch_sub(&ObjectHeader->HandleCount, 1, __ATOMIC_SEQ_CST);

                KeRemoveList(&HandleEntry->HandleLinks);

                DereferenceObject(Object);

                Mm::FreePool(HandleEntry, MAKE_TAG('H', 'n', 'd', 'l'));

                Current = Next;
            }

            Process->HandleTable.HandleCount = 0;
        }
    }
} // namespace Ob