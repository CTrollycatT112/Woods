// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Type definitions

AUTHOR: Trollycat

ABSTRACT: Defines type's used across the project (like DWORD)

--*/
#pragma once

#include "htdef.hpp"

typedef void VOID;
typedef void *PVOID, *LPVOID;
typedef CONST void *PCVOID, *LPCVOID;

typedef void* HANDLE, **PHANDLE;

typedef char  CHAR;
typedef short SHORT;
typedef long  LONG;
typedef int   INT;

typedef unsigned char  UCHAR,  *PUCHAR;
typedef unsigned short USHORT, *PUSHORT;
typedef unsigned long  ULONG,  *PULONG;
typedef unsigned int   UINT,   *PUINT;

typedef unsigned char       BYTE,  *PBYTE;
typedef unsigned short      WORD,  *PWORD;
typedef unsigned long       DWORD, *PDWORD;
typedef unsigned long long  QWORD, *PQWORD;

typedef unsigned long SIZE_T,  *PSIZE_T;
typedef long          SSIZE_T, *PSSIZE_T;

typedef SHORT *PSHORT;
typedef LONG  *PLONG;

typedef bool BOOLEAN, *PBOOLEAN;
typedef int  BOOL,    *PBOOL;

typedef LONG        HTSTATUS, *PHTSTATUS;
typedef signed char SCHAR,    *PSCHAR;

typedef long long          LONGLONG,  *PLONGLONG;
typedef unsigned long long ULONGLONG, *PULONGLONG;

typedef CHAR *PCHAR, *LPCH, *PCH, *PNZCH;
typedef CONST CHAR *LPCCH, *PCCH, *PCNZCH;
typedef CHAR *NPSTR, *LPSTR, *PSTR;
typedef PSTR *PZPSTR;
typedef CONST PSTR *PCZPSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;
typedef PCSTR *PZPCSTR;

typedef char16_t WCHAR;
typedef WCHAR *PWCHAR, *LPWCH, *PWCH;
typedef CONST WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR, *LPWSTR, *PWSTR;
typedef PWSTR *PZPWSTR;
typedef CONST PWSTR *PCZPWSTR;
typedef WCHAR *LPUWSTR, *PUWSTR;
typedef CONST WCHAR *LPCWSTR, *PCWSTR;
typedef PCWSTR *PZPCWSTR;
typedef CONST WCHAR *LPCUWSTR, *PCUWSTR;

typedef WCHAR *PZZWSTR;
typedef CONST WCHAR *PCZZWSTR;
typedef WCHAR *PUZZWSTR;
typedef CONST WCHAR *PCUZZWSTR;

typedef WCHAR *PNZWCH;
typedef CONST WCHAR *PCNZWCH;
typedef WCHAR *PUNZWCH;
typedef CONST WCHAR *PCUNZWCH;

typedef CONST WCHAR *LPCWCHAR, *PCWCHAR;
typedef CONST WCHAR *LPCUWCHAR, *PCUWCHAR;

typedef CONST UCHAR *PCUCHAR;

typedef char CCHAR;
typedef CCHAR *PCCHAR;
typedef short CSHORT, *PCSHORT;
typedef ULONG CLONG, *LCLONG;

typedef struct PACKED _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWCH   Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct PACKED _LIST_ENTRY
{
    struct _LIST_ENTRY* Flink;
    struct _LIST_ENTRY* Blink;
} LIST_ENTRY, *PLIST_ENTRY;

#define MINCHAR   0x80
#define MAXCHAR   0x7f
#define MINSHORT  0x8000
#define MAXSHORT  0x7fff
#define MINLONG   0x80000000
#define MAXLONG   0x7fffffff
#define MAXUCHAR  0xff
#define MAXUSHORT 0xffff
#define MAXULONG  0xffffffff
#define MAXBYTE   0xff
#define MAXWORD   0xffff
#define MAXDWORD  0xffffffff
#define MAXLONGLONG (0x7fffffffffffffffLL)