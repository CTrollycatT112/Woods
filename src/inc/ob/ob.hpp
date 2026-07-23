/*++

MODULE: Object manager

AUTHOR: Trollycat

ABSTRACT: There is 2 common approaches in operating systems,
          the unix approach, 'everything is a file',
          and the NT approach 'everything is an object',
          NT treats everything under a central 'object manager',
          a process, a thread, a file, it's all an 'object',
          this system allows for pure speed, and debugging,
          I like this approach a LOT more than the UNIX approach

--*/
#pragma once

#include "htbase.hpp"

#include "ke/amd64/amd64.hpp"

//
// OBJECT ATTRIBUTES
//
#define OBJ_PERMANENT 0x00000010
#define OBJ_KERNEL    0x00000000

//
// ACCESS MASKS
//
#define GENERIC_READ (1 << 29)
#define GENERIC_WRITE (1 << 30)
#define GENERIC_EXECUTE (1 << 31)
#define GENERIC_ALL (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE)

//
// HELPER MACROS
//
#define ObGetHeaderFromObject(x) ((POBJECT_HEADER)x - 1)
#define ObGetObjectFromHeader(x) ((POBJECT_HEADER)x + 1)

typedef enum _OB_OPEN_REASON
{
    ObCreateHandle,
    ObOpenHandle,
    ObDuplicateHandle,
    ObInheritHandle
} OB_OPEN_REASON, *POB_OPEN_REASON;

typedef enum _OB_DUMP_TARGET
{
    ObDumpToInbv        = 1,
    ObDumpToKdcom       = 2,
    ObDumpToBoth        = 3
} OB_DUMP_TARGET;

//
// OPEN
//
typedef VOID (*OB_OPEN_METHOD) 
(
    OB_OPEN_REASON Reason,
    PVOID          Process,
    PVOID          Object,
    DWORD          GrantedAccess,
    DWORD          HandleCount
);

//
// CLOSE
//
typedef VOID (*OB_CLOSE_METHOD) 
(
    PVOID       Process,
    PVOID       Object,
    DWORD       GrantedAccess,
    DWORD       ProcessHandleCount,
    DWORD       SystemHandleCount
);

//
// DUMP
//
typedef VOID (*OB_DUMP_METHOD) 
(
    PVOID          Object,
    OB_DUMP_TARGET Target
);

//
// DELETE
//
typedef VOID (*OB_DELETE_METHOD) (
    PVOID       Object
);

typedef struct _OBJECT_TYPE_INITIALIZER
{
    OB_OPEN_METHOD      OpenMethod;
    OB_CLOSE_METHOD     CloseMethod;
    OB_DELETE_METHOD    DeleteMethod;
    OB_DUMP_METHOD      DumpMethod;
} OBJECT_TYPE_INITIALIZER, *POBJECT_TYPE_INITIALIZER;

typedef struct _OBJECT_ATTRIBUTES
{
    UNICODE_STRING Name;
    PVOID          Parent;
    ULONG          Attributes;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _OBJECT_DIRECTORY_INFORMATION
{
    UNICODE_STRING      Name;
    UNICODE_STRING      TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

typedef struct _OBJECT_TYPE
{
    UNICODE_STRING              Name;
    ULONG32                     Key;
    ULONG64                     ObjectCount;
    OBJECT_TYPE_INITIALIZER     ObjectInitializer;
    LIST_ENTRY                  ObjectTypeLinks;
    PLIST_ENTRY                 ObjectList;
} OBJECT_TYPE, *POBJECT_TYPE;

typedef struct _OBJECT_DIRECTORY
{
    UNICODE_STRING Name;
    POBJECT_HEADER ObjectLinks;
    ULONG64        ObjectCount;
} OBJECT_DIRECTORY, *POBJECT_DIRECTORY;

typedef struct _OBJECT_HEADER
{
    ULONG64                 PointerCount;
    ULONG64                 HandleCount;
    OBJECT_ATTRIBUTES       Attributes;
    ULONG64                 Length;
    LIST_ENTRY              ObjectLinks;
    LIST_ENTRY              ObjectDirectoryLinks;
    POBJECT_TYPE            Type;
} OBJECT_HEADER, *POBJECT_HEADER;

typedef struct _OBJECT_SYMBOLIC_LINK
{
    UNICODE_STRING  LinkTargetPath;
    PVOID           LinkTargetObject;
} OBJECT_SYMBOLIC_LINK, *POBJECT_SYMBOLIC_LINK;

typedef struct _HANDLE_TABLE_ENTRY
{
    HANDLE      Handle;
    PVOID       Object;
    ACCESS_MASK Access;
    PKPROCESS   Owner;
    LIST_ENTRY  HandleLinks;
} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

namespace Ob
{
    /*++

    ROUTINE: InitializeObjectManager

    DESCRIPTION: Initialization routine for the object manager

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/
    VOID
    InitializeObjectManager();

    /*++

    ROUTINE: GetObjectByPath

    DESCRIPTION: Find an object from a path

    ARGUMENTS: Path             - The path to look up
               Object           - The object to be found
               ParentDirectory  - The parent directory

    RETURNS: HTSTATUS

    --*/

    NODISCARD
    HTSTATUS
    GetObjectByPath(PUNICODE_STRING   Path,
                    PVOID*            Object,
                    POBJECT_DIRECTORY ParentDirectory);

    /*++

    ROUTINE: CreateObject

    DESCRIPTION: Create a new object and store it in the manager

    ARGUMENTS: Type       - The object type
               Attributes - The object attributes
               Length     - The object length
               Object     - The new object

    RETURNS: HTSTATUS

    --*/

    NODISCARD
    HTAPI
    HTSTATUS
    CreateObject(POBJECT_TYPE       Type,
                 POBJECT_ATTRIBUTES Attributes,
                 ULONG              Length,
                 PVOID*             Object);

    /*++

    ROUTINE: DestroyObject

    DESCRIPTION: Remove an object from the manager

    ARGUMENTS: Object - The object to destroy

    RETURNS: HTSTATUS

    --*/

    NODISCARD
    HTSTATUS
    DestroyObject(PVOID Object);

    /*++

    ROUTINE: CreateObjectType

    DESCRIPTION: Register a new object type with its callbacks

    ARGUMENTS: TypeName       - The object type name
               ObjectTypeInit - The type initializer
               ObjectType     - The new object type

    RETURNS: HTSTATUS

    --*/

    NODISCARD
    HTAPI
    HTSTATUS
    CreateObjectType(PUNICODE_STRING            TypeName,
                     POBJECT_TYPE_INITIALIZER   ObjectTypeInit,
                     POBJECT_TYPE*              ObjectType);

    /*++

    ROUTINE: SetObjectTypeTag

    DESCRIPTION: Set the tag used for an object type

    ARGUMENTS: ObjectType - The object type
               Tag        - The tag to set

    RETURNS: VOID

    --*/

    HTAPI
    VOID
    SetObjectTypeTag(POBJECT_TYPE ObjectType,
                     ULONG        Tag);

    /*++

    ROUTINE: GetDirectoryFromPath

    DESCRIPTION: Find a directory object from a path

    ARGUMENTS: Path       - The path to inspect
               Directory  - The directory found

    RETURNS: HTSTATUS

    --*/

    NODISCARD
    HTSTATUS
    GetDirectoryFromPath(PUNICODE_STRING    Path,
                         POBJECT_DIRECTORY* Directory);


    /*++

    ROUTINE: CreateDirectoryObject

    DESCRIPTION: Create a new directory object

    ARGUMENTS: Directory  - The directory to create
               Attributes - The object attributes

    RETURNS: HTSTATUS

    --*/

    NODISCARD
    HTSTATUS
    CreateDirectoryObject(POBJECT_DIRECTORY* Directory,
                          POBJECT_ATTRIBUTES Attributes);


    /*++

    ROUTINE: GetRootDirectory

    DESCRIPTION: Return the root directory object

    ARGUMENTS: N/A

    RETURNS: POBJECT_DIRECTORY

    --*/

    HTAPI
    POBJECT_DIRECTORY
    GetRootDirectory();

    /*++

    ROUTINE: ReferenceObject

    DESCRIPTION: Increase the reference count of an object

    ARGUMENTS: Object - The object to reference

    RETURNS: VOID

    --*/

    VOID
    ReferenceObject(PVOID Object);

    /*++

    ROUTINE: ReferenceObjectByName

    DESCRIPTION: Find an object by its name and return it

    ARGUMENTS: ObjectAttributes - The object attributes
               Type             - The object type
               AccessMode       - The access mode
               Object           - The object found

    RETURNS: HTSTATUS

    --*/

    NODISCARD
    HTAPI
    HTSTATUS
    ReferenceObjectByName(POBJECT_ATTRIBUTES ObjectAttributes,
                          POBJECT_TYPE       Type,
                          KPROCESSOR_MODE    AccessMode,
                          PVOID*             Object);

    /*++

    ROUTINE: ReferenceObjectByHandle

    DESCRIPTION: Find an object by its handle and return it

    ARGUMENTS: Handle    - The handle to use
               Type      - The object type
               Access    - The access mask
               AcessMode - The access mode
               Object    - The object found

    RETURNS: HTSTATUS

    --*/

    NODISCARD
    HTAPI
    HTSTATUS
    ReferenceObjectByHandle(HANDLE          Handle,
                            POBJECT_TYPE    Type,
                            ACCESS_MASK     Access,
                            KPROCESSOR_MODE AcessMode,
                            PVOID*          Object);

    /*++

    ROUTINE: DereferenceObject

    DESCRIPTION: Lower the reference count of an object

    ARGUMENTS: Object - The object to dereference

    RETURNS: VOID

    --*/

    HTAPI
    VOID
    DereferenceObject(PVOID Object);

    /*++

    ROUTINE: OpenObjectByPointer

    DESCRIPTION: Create a handle for an object from a pointer

    ARGUMENTS: Object        - The object to open
               DesiredAccess - The access mask
               ObjectType    - The object type
               AccessMode    - The access mode
               Handle        - The new handle

    RETURNS: HTSTATUS

    --*/

    NODISCARD
    HTAPI
    HTSTATUS
    OpenObjectByPointer(PVOID          Object,
                       ACCESS_MASK     DesiredAccess,
                       POBJECT_TYPE    ObjectType,
                       KPROCESSOR_MODE AccessMode,
                       PHANDLE         Handle);

    /*++

    ROUTINE: CloseHandle

    DESCRIPTION: Close a handle that points to an object

    ARGUMENTS: Handle - The handle to close

    RETURNS: HTSTATUS

    --*/

    NODISCARD
    HTAPI
    HTSTATUS
    CloseHandle(HANDLE Handle);

    /*++

    ROUTINE: CloseAllHandles

    DESCRIPTION: Close all handles owned by a process

    ARGUMENTS: Process - The process to use

    RETURNS: VOID

    --*/

    VOID
    CloseAllHandles(PKPROCESS Process);

    /*++

    ROUTINE: DumpDirectory

    DESCRIPTION: Print the contents of a directory object

    ARGUMENTS: Object - The object to dump
               Target - The dump target

    RETURNS: VOID

    --*/
    VOID
    DumpDirectory(PVOID Object, OB_DUMP_TARGET Target);

} // namespace Ob