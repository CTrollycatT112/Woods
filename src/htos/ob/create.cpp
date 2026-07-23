/*++

MODULE: Object Manager

AUTHOR: Trollycat

ABSTRACT: This module implements the creation/deletion routines

--*/
#include "ob/ob.hpp"

#include "mm/mm.hpp"

#include "rtl/rtl.hpp"

EXTERN POBJECT_TYPE ObDirectoryObject;

namespace Ob
{
    NODISCARD
    HTSTATUS
    CreateObject(POBJECT_TYPE       Type, 
                 POBJECT_ATTRIBUTES Attributes, 
                 ULONG              Length, 
                 PVOID              *Object)
    {
        //
        // ATTEMPT TO ALLOCATE OUR HEADER
        //
        POBJECT_HEADER ObjectHeader = (POBJECT_HEADER)
        Mm::AllocatePoolWithTag
        (
            NonPagedPoolZeroed,
            sizeof(OBJECT_HEADER) + Length,
            Type->Key
        );

        //
        // The POOL failed to allocate our header
        // Likely means' we lack the memory to allocate it
        // Instead of crashing, we can say 'INSUFFICIENT_RESOURCES'
        // Maybe HT will be good at handling 0 memory left
        //
        if (ObjectHeader == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        //
        // The POOL allocated our header
        //
        ObjectHeader->Type         = Type;
        ObjectHeader->PointerCount = 1;
        ObjectHeader->Length       = Length;
        ObjectHeader->Attributes   = *Attributes;

        if (Attributes->Name.Buffer != NULL)
        {
            ObjectHeader->Attributes.Name.Buffer = (PWCH)Rtl::CloneString(
                ObjectHeader->Attributes.Name.Buffer
            );
        }

        if (Type->ObjectList == NULL)
        {
            KeInitializeHeadList(&ObjectHeader->ObjectLinks);
            Type->ObjectList = &ObjectHeader->ObjectLinks;
        }
        else
        {
            KeInsertTailList(Type->ObjectList, &ObjectHeader->ObjectLinks);
        }

        Type->ObjectCount++;

        if (Attributes->Parent != NULL)
        {
            if (ObGetHeaderFromObject(Attributes->Parent)->Type ==
                ObDirectoryObject)
            {
                POBJECT_DIRECTORY ParentDirecotry = (POBJECT_DIRECTORY)
                Attributes->Parent;

                if (ParentDirecotry->ObjectCount == 0)
                {
                    KeInitializeHeadList(&ObjectHeader->ObjectDirectoryLinks);
                    ParentDirecotry->ObjectLinks = ObjectHeader;
                }
                else
                {
                    KeInsertTailList(&ParentDirecotry->ObjectLinks->ObjectDirectoryLinks,
                                     &ObjectHeader->ObjectDirectoryLinks);
                }

                ParentDirecotry->ObjectCount++;
            }
        }

        *Object = ObGetObjectFromHeader(ObjectHeader);
        return STATUS_SUCCESS;
    }

    NODISCARD
    HTSTATUS
    DestroyObject(PVOID Object)
    {
        //
        // GRAB THE OBJECT FIRST
        //
        POBJECT_HEADER ObjectHeader = ObGetHeaderFromObject(Object);
        POBJECT_TYPE   Type         = ObjectHeader->Type;

        //
        // IF THE OBJECT HAS A DELETE CALLBACK,
        // WE DIRECTLY CALL IT HERE..
        //
        if (Type->ObjectInitializer.DeleteMethod)
        {
            Type->ObjectInitializer.DeleteMethod(Object);
        }

        Type->ObjectCount--;
        if (Type->ObjectCount == 0)
        {
            Type->ObjectList = NULL;
        }
        else
        {
            KeRemoveList(&ObjectHeader->ObjectLinks);
        }

        if (ObjectHeader->Attributes.Parent != NULL)
        {
            if (ObGetHeaderFromObject(ObjectHeader->Attributes.Parent)->Type ==
                ObDirectoryObject)
            {
                POBJECT_DIRECTORY ParentDirectory = (POBJECT_DIRECTORY)
                ObjectHeader->Attributes.Parent;

                ParentDirectory->ObjectCount--;

                if (ParentDirectory->ObjectCount == 0)
                {
                    ParentDirectory->ObjectLinks = NULL;
                }
                else
                {
                    KeRemoveList(&ObjectHeader->ObjectDirectoryLinks);
                }
            }
        }

        if (ObjectHeader->Attributes.Name.Buffer)
        {
            Mm::FreePool(ObjectHeader->Attributes.Name.Buffer,
                         MAKE_TAG('R', 't', 'l', ' '));
        }

        Mm::FreePool(ObjectHeader,Type->Key);

        return STATUS_SUCCESS;
    }
} // namespace Ob