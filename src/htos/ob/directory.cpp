/*++

MODULE: Object Manager

AUTHOR: Trollycat

ABSTRACT: This module implements the directory/path routines

--*/
#include "ob/ob.hpp"

EXTERN POBJECT_TYPE      ObDirectoryObject;
EXTERN POBJECT_DIRECTORY ObRootDirectory;

namespace Ob
{
    NODISCARD
    HTSTATUS
    GetDirectoryFromPath(PUNICODE_STRING    Path,
                         POBJECT_DIRECTORY *Directory)
    {
        PVOID Object = NULL;

        HTSTATUS Status = GetObjectByPath(Path,&Object, NULL);
    
        if (Status != STATUS_SUCCESS)
        {
            return Status;
        }

        POBJECT_HEADER ObjectHeader = ObGetHeaderFromObject(Object);
        if (ObjectHeader->Type != ObDirectoryObject)
        {
            return STATUS_OBJECT_TYPE_MISMATCH;
        }

        *Directory = (POBJECT_DIRECTORY)Object;
        return STATUS_SUCCESS;
    }

    NODISCARD
    HTSTATUS
    CreateDirectoryObject(POBJECT_DIRECTORY *Directory, 
                          POBJECT_ATTRIBUTES Attributes)
    {
        HTSTATUS Status = CreateObject(ObDirectoryObject,
                                       Attributes,
                                       sizeof(OBJECT_DIRECTORY),
                                       (PVOID*)Directory);

        if (Status != STATUS_SUCCESS)
        {
            return Status;
        }

        return STATUS_SUCCESS;
    }

    NODISCARD
    POBJECT_DIRECTORY
    GetRootDirectory()
    {
        return ObRootDirectory;
    }
} // namespace Ob