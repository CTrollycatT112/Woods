/*++

MODULE: Object manager

AUTHOR: Trollycat

ABSTRACT: This module implements the initialization routines

--*/
#include "htdef.hpp"
#include "ob/ob.hpp"

#include "mm/mm.hpp"
#include "rtl/rtl.hpp"

PLIST_ENTRY         ObjectTypeList        = NULL;
POBJECT_TYPE        ObDirectoryObject     = NULL;
STATIC POBJECT_TYPE ObTypeObject          = NULL;
POBJECT_TYPE        ObLinkObject          = NULL;

POBJECT_DIRECTORY ObRootDirectory         = NULL;
POBJECT_DIRECTORY ObTypesDirectory        = NULL;
POBJECT_DIRECTORY ObGlobalRootDirectory   = NULL;

namespace Ob
{
    namespace
    {
        NODISCARD
        HTSTATUS
        RecursiveDirectoryDump(POBJECT_DIRECTORY Directory,
                            INT               Level,
                            OB_DUMP_TARGET    Target)
        {
            if (Directory->ObjectCount == 0)
            {
                if (Target & ObDumpToKdcom)
                {
                    for (INT I = 0; I < Level; I++)
                    {
                        Rtl::KdPrint(L"\t");
                    }
                    Rtl::KdPrint(L"%S HAS NOTHING\r\n",
                                ObGetHeaderFromObject(Directory)->Attributes.Name.Buffer);
                }

                if (Target & ObDumpToInbv)
                {
                    for (INT I = 0; I < Level; I++)
                    {
                        Rtl::Print("\t");
                    }
                    Rtl::Print("%S HAS NOTHING\r\n",
                            ObGetHeaderFromObject(Directory)->Attributes.Name.Buffer);
                }
                            
                return STATUS_SUCCESS;
            }

            PLIST_ENTRY Flink = Directory->ObjectLinks->ObjectDirectoryLinks.Flink;

            do
            {
                POBJECT_HEADER CurrentObject = CONTAINING_RECORD(
                    Flink,
                    OBJECT_HEADER,
                    ObjectDirectoryLinks
                );

                Flink = Flink->Flink;

                if (Target & ObDumpToKdcom)
                {
                    for (INT I = 0; I < Level; I++)
                    {
                        Rtl::KdPrint(L"\t");
                    }
                    Rtl::KdPrint(L"%S OF TYPE %S\r\n",
                                CurrentObject->Attributes.Name.Buffer,
                                CurrentObject->Type->Name.Buffer);
                }

                if (Target & ObDumpToInbv)
                {
                    for (INT I = 0; I < Level; I++)
                    {
                        Rtl::Print("\t");
                    }
                    Rtl::Print("%S OF TYPE %S\r\n",
                            CurrentObject->Attributes.Name.Buffer,
                            CurrentObject->Type->Name.Buffer);
                }

                if (CurrentObject->Type == ObDirectoryObject)
                {
                    HTSTATUS Status = RecursiveDirectoryDump(
                        (POBJECT_DIRECTORY)ObGetObjectFromHeader(CurrentObject),
                        Level + 1,
                        Target
                    );

                    if (Status != STATUS_SUCCESS)
                    {
                        return Status;
                    }
                }
            } while (Flink != Directory->ObjectLinks->ObjectDirectoryLinks.Flink);

            return STATUS_SUCCESS;
        }

    } // namespace

    NODISCARD
    HTSTATUS
    CreateObjectType(PUNICODE_STRING          TypeName, 
                     POBJECT_TYPE_INITIALIZER ObjectTypeInit, 
                     POBJECT_TYPE             *ObjectType)
    {
        //
        // TRY TO ALLOCATE OUR TYPE
        //
        POBJECT_TYPE Type = (POBJECT_TYPE)Mm::AllocatePoolWithTag(
            NonPagedPoolZeroed,
            sizeof(OBJECT_TYPE),
            MAKE_TAG('O', 'b', ' ', ' ')
        );

        //
        // FAILED TO ALLOCATE OUR TYPE
        //
        if (Type == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        //
        // TYPE WAS ALLOCATED
        //
        Type->Name = *TypeName;

        if (ObjectTypeInit)
        {
            Type->ObjectInitializer = *ObjectTypeInit;
        }

        //
        // SET TAG TO 'Ob '
        //
        Type->Key         = MAKE_TAG('O', 'b', ' ', ' ');
        Type->ObjectCount = 0;
        
        //
        // Set ObjectTypeList
        // If NULL, HeadList
        // If not NULL, TailList
        //
        if (ObjectTypeList == NULL)
        {
            KeInitializeHeadList(&Type->ObjectTypeLinks);
            ObjectTypeList = &Type->ObjectTypeLinks;
        }
        else
        {
            KeInsertTailList(ObjectTypeList, &Type->ObjectTypeLinks);
        }

        if (ObTypesDirectory != NULL)
        {
            OBJECT_ATTRIBUTES Attr = {
                .Name       = *TypeName,
                .Parent     = ObTypesDirectory,
                .Attributes = 0
            };

            PVOID Object = NULL;

            HTSTATUS Status = CreateObject(
                ObTypeObject,
                &Attr,
                sizeof(OBJECT_TYPE),
                &Object
            );

            if (Status != STATUS_SUCCESS)
            {
                return Status;
            }
        }

        *ObjectType = Type;
        return STATUS_SUCCESS;
    }

    VOID
    SetObjectTypeTag(POBJECT_TYPE ObjectType,
                     ULONG        Tag)
    {
        ObjectType->Key = Tag;
    }

    NODISCARD
    HTSTATUS
    GetObjectByPath(PUNICODE_STRING   Path, 
                    PVOID            *Object, 
                    POBJECT_DIRECTORY ParentDirectory)
    {
        PWSTR* Tokens     = NULL;
        ULONG  TokenCount = 0;

        if (!Rtl::SplitString(Path->Buffer, L'\\', &Tokens, &TokenCount))
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        POBJECT_DIRECTORY CurrentDirectory =
        ParentDirectory ? ParentDirectory : ObRootDirectory;

        for (ULONG64 I = 0; I < TokenCount; I++)
        {
            POBJECT_HEADER CurrentFind = NULL;

            if (Rtl::StringLength(Tokens[I]) == 0)
            {
                continue;
            }


            PLIST_ENTRY Flink =
            CurrentDirectory->ObjectLinks->ObjectDirectoryLinks.Flink;

            do
            {
                POBJECT_HEADER CurrentObject =
                CONTAINING_RECORD(Flink,OBJECT_HEADER, ObjectDirectoryLinks);

                Flink = Flink->Flink;

                if (Rtl::CompareString(
                    CurrentObject->Attributes.Name.Buffer,
                    Tokens[I],
                    TRUE
                ) == 0)
                {
                    CurrentFind = CurrentObject;
                    break;
                }

            } while (Flink != CurrentDirectory->ObjectLinks->ObjectDirectoryLinks.Flink);

            if (CurrentFind == NULL)
            {
                Rtl::FreeSplitString((PWSTR**)Tokens, TokenCount);
                return STATUS_NOT_FOUND;
            }

            if (CurrentFind->Type == ObLinkObject)
            {
                POBJECT_SYMBOLIC_LINK Link = 
                (POBJECT_SYMBOLIC_LINK)
                ObGetObjectFromHeader(CurrentFind);

                CurrentFind = ObGetHeaderFromObject(Link->LinkTargetObject);
            }

            if (I == TokenCount - 1)
            {
                Rtl::FreeSplitString((PWSTR**)Tokens,TokenCount);
                *Object = ObGetObjectFromHeader(CurrentFind);
                return STATUS_SUCCESS;
            }

            if (CurrentFind->Type != ObDirectoryObject)
            {
                Rtl::FreeSplitString((PWSTR**)Tokens, TokenCount);
                return STATUS_NOT_FOUND;
            }

            CurrentDirectory = (POBJECT_DIRECTORY)
                ObGetObjectFromHeader(CurrentFind);
        }

        Rtl::FreeSplitString((PWSTR**)Tokens,TokenCount);
        return STATUS_OBJECT_PATH_INVALID;
    }

    VOID
    DumpDirectory(PVOID Object, OB_DUMP_TARGET Target)
    {
        if (Object == NULL)
        {
            return;
        }

        POBJECT_DIRECTORY Directory = (POBJECT_DIRECTORY)Object;

        if (Directory->ObjectCount == 0)
        {
            POBJECT_HEADER ObjectHeader = ObGetHeaderFromObject(Directory);

            if (Target & ObDumpToKdcom)
            {
                Rtl::KdPrint(L"%S HAS NOTHING\r\n", 
                            ObjectHeader->Attributes.Name.Buffer);
            }

            if (Target & ObDumpToInbv)
            {
                Rtl::Print("%S HAS NOTHING\r\n", 
                        ObjectHeader->Attributes.Name.Buffer);
            }
            return;
        }

        HTSTATUS Status = RecursiveDirectoryDump(Directory, 0, Target);

        if (Status != STATUS_SUCCESS)
        {
            (VOID)Status;
        }
    }

    CODESEG(".init")
    VOID
    InitializeObjectManager()
    {
        UNICODE_STRING ObjectDirectoryName = RTL_CONSTANT_STRING(L"Directory");

        OBJECT_TYPE_INITIALIZER ObjectDirectoryInitializer = {
            .OpenMethod   = NULL,
            .CloseMethod  = NULL,
            .DeleteMethod = NULL,
            .DumpMethod   = DumpDirectory
        };

        HTSTATUS Status = CreateObjectType(
            &ObjectDirectoryName,
            &ObjectDirectoryInitializer,
            &ObDirectoryObject
        );

        ASSERT(Status == STATUS_SUCCESS);

        ObDirectoryObject->Key = MAKE_TAG('D', 'i', 'r', ' ');

        UNICODE_STRING ObjectTypeName = RTL_CONSTANT_STRING(L"Type");

        Status = CreateObjectType(
            &ObjectTypeName,
            NULL,
            &ObTypeObject
        );

        ASSERT(Status == STATUS_SUCCESS);

        ObDirectoryObject->Key = MAKE_TAG('T', 'y', 'p', 'e');

        UNICODE_STRING ObjectLinkName = RTL_CONSTANT_STRING(L"SymbolicLink");
    
        Status = CreateObjectType(
            &ObjectLinkName,
            NULL,
            &ObLinkObject
        );

        ASSERT(Status == STATUS_SUCCESS);

        ObLinkObject->Key = MAKE_TAG('L', 'i', 'n', 'k');

        OBJECT_ATTRIBUTES ObRootDirectoryAttributes = {
            .Name       = RTL_CONSTANT_STRING(L"\\"),
            .Parent     = NULL,
            .Attributes = OBJ_PERMANENT
        };

        Status = CreateDirectoryObject(&ObRootDirectory,&
                                        ObRootDirectoryAttributes);
            
        ASSERT(Status == STATUS_SUCCESS);

        OBJECT_ATTRIBUTES ObTypesDirectoryAttributes = {
            .Name       = RTL_CONSTANT_STRING(L"ObjectTypes"),
            .Parent     = ObRootDirectory,
            .Attributes = OBJ_PERMANENT | OBJ_KERNEL
        };

        Status = CreateDirectoryObject(&ObTypesDirectory,
                                       &ObTypesDirectoryAttributes);

        ASSERT(Status == STATUS_SUCCESS);

        OBJECT_ATTRIBUTES ObGlobalRootDirectoryAttributes = {
            .Name       = RTL_CONSTANT_STRING(L"??"),
            .Parent     = ObRootDirectory,
            .Attributes = OBJ_PERMANENT
        };

        Status = CreateDirectoryObject(&ObGlobalRootDirectory,
                                       &ObGlobalRootDirectoryAttributes);

        ASSERT(Status == STATUS_SUCCESS);

        OBJECT_ATTRIBUTES Attr = {
            .Name       = ObjectDirectoryName,
            .Parent     = ObTypesDirectory,
            .Attributes = 0
        };

        PVOID Obj = NULL;

        Status = CreateObject(
            ObTypeObject, 
            &Attr, 
            sizeof(OBJECT_TYPE), 
            &Obj
        );

        Attr.Name = ObjectLinkName;

        Obj = NULL;

        Status = CreateObject(
            ObTypeObject,
            &Attr, 
            sizeof(OBJECT_TYPE), 
            &Obj
        );
    }
} // namespace Ob