/*++

MODULE: Object manager

AUTHOR: Trollycat

ABSTRACT: This module implements
            Reference/Dereference routines

--*/
#include "ob/ob.hpp"

#include "rtl/rtl.hpp"

EXTERN POBJECT_TYPE ObDirectoryObject;
EXTERN PKPROCESS    KpKernelProcessInstance;

namespace Ob
{
    VOID
    ReferenceObject(PVOID Object)
    {
        POBJECT_HEADER ObjectHeader = ObGetHeaderFromObject(Object);
        __atomic_fetch_add(&ObjectHeader->PointerCount, 1, __ATOMIC_SEQ_CST);
    }

    NODISCARD
    HTSTATUS
    ReferenceObjectByName(POBJECT_ATTRIBUTES ObjectAttribute,
                          POBJECT_TYPE       Type,
                          KPROCESSOR_MODE    AccessMode,
                          PVOID*             Object)
    {
        POBJECT_DIRECTORY ParentDirectory = NULL;

        if (ObjectAttribute->Parent != NULL)
        {
            if (ObGetHeaderFromObject(ObjectAttribute->Parent)->
                Type == ObDirectoryObject)
            {
                ParentDirectory = (POBJECT_DIRECTORY)ObjectAttribute->Parent;
            }
        }

        HTSTATUS Status = GetObjectByPath(&ObjectAttribute->Name,
                                          Object,
                                          ParentDirectory);
        if (Status != STATUS_SUCCESS)
        {
            return Status;
        }

        if (Type != NULL && ObGetHeaderFromObject(*Object)->Type != Type)
        {
            return STATUS_OBJECT_TYPE_MISMATCH;
        }

        if (AccessMode == UserMode && (ObGetHeaderFromObject(*Object)->Attributes.Attributes & OBJ_KERNEL) != 0)
        {
            return STATUS_ACCESS_DENIED;
        }

        ReferenceObject(*Object);
        return STATUS_SUCCESS;
    }

    VOID 
    DereferenceObject(PVOID Object) {
        POBJECT_HEADER ObjectHeader = ObGetHeaderFromObject(Object);
        __atomic_fetch_sub(&ObjectHeader->PointerCount, 1, __ATOMIC_SEQ_CST);

        if (ObjectHeader->PointerCount   == 0
            && ObjectHeader->HandleCount == 0
            && (ObjectHeader->Attributes.Attributes & OBJ_PERMANENT) == 0) 
        {
            HTSTATUS Status =DestroyObject(Object);
            ASSERT(Status == STATUS_SUCCESS);
        }
    }

    NODISCARD
    HTSTATUS
    ReferenceObjectByHandle(HANDLE          Handle, 
                            POBJECT_TYPE    Type, 
                            ACCESS_MASK     Access, 
                            KPROCESSOR_MODE AccessMode, 
                            PVOID*          Object)
    {
        PVOID Object_ = NULL;

        PKPROCESS           CallingProcess        = KpKernelProcessInstance;
        PHANDLE_TABLE_ENTRY HandleEntry           = (PHANDLE_TABLE_ENTRY)Handle;

        if (Handle == KpKernelProcessInstance) 
        {
            Object_ = KpKernelProcessInstance;
        }
        else 
        {
            if (HandleEntry == NULL)
            {
                return STATUS_INVALID_HANDLE;
            }

            if (HandleEntry->Owner != CallingProcess) 
            {
                return STATUS_INVALID_HANDLE;
            }
            
            if ((HandleEntry->Access & Access) != Access) 
            {
                return STATUS_ACCESS_DENIED;
            }

            Object_ = HandleEntry->Object;
        }

        POBJECT_HEADER ObjectHeader = ObGetHeaderFromObject(Object_);

        if (Type != NULL && ObjectHeader->Type != Type) 
        {
            return STATUS_OBJECT_TYPE_MISMATCH;
        }

        if (((ObjectHeader->Attributes.Attributes & OBJ_KERNEL) == OBJ_KERNEL) && AccessMode != KernelMode)
        {
            return STATUS_ACCESS_DENIED;
        }

        if (ObjectHeader->Type->ObjectInitializer.OpenMethod != NULL) 
        {
            PVOID TargetProcess      = KpKernelProcessInstance;
            PVOID RawObject          = Object_;
            DWORD AccessMask         = Access;
            DWORD ProcessHandleCount = ObjectHeader->HandleCount;

            ObjectHeader->Type->ObjectInitializer.OpenMethod(
                ObOpenHandle,
                TargetProcess,
                RawObject,
                AccessMask,
                ProcessHandleCount
            );
        }

        __atomic_fetch_add(&ObjectHeader->PointerCount, 1, __ATOMIC_SEQ_CST);

        *Object = Object_;
        return STATUS_SUCCESS;
    }

} // namespace Ob