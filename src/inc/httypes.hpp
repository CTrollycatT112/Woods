// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Type definitions

AUTHOR: Trollycat

ABSTRACT: Defines type's used across the project (like DWORD)

--*/
#pragma once

#include "htdef.hpp"
#include <stdint.h>
#include <stddef.h>

typedef void VOID;
typedef void *PVOID, *LPVOID;
typedef const void *PCVOID, *LPCVOID;

typedef void* HANDLE, **PHANDLE;

typedef char    CHAR;
typedef int16_t SHORT;
typedef int32_t LONG;
typedef int32_t INT;

typedef uint8_t   UCHAR,   *PUCHAR;
typedef uint16_t  USHORT,  *PUSHORT;
typedef uint32_t  ULONG,   *PULONG;
typedef uint32_t  ULONG32, *PULONG32;
typedef uint32_t  UINT,    *PUINT;

typedef uint8_t   BYTE,  *PBYTE;
typedef uint16_t  WORD,  *PWORD;
typedef uint32_t  DWORD, *PDWORD;
typedef uint64_t  QWORD, *PQWORD;

typedef size_t    SIZE_T,  *PSIZE_T;
typedef __PTRDIFF_TYPE__    SSIZE_T, *PSSIZE_T;

typedef SHORT *PSHORT;
typedef LONG  *PLONG;

typedef bool BOOLEAN, *PBOOLEAN;
typedef bool BOOL,    *PBOOL;

typedef LONG        HTSTATUS, *PHTSTATUS;
typedef int8_t      SCHAR,    *PSCHAR;

typedef int64_t  LONGLONG,  *PLONGLONG;
typedef int64_t  LONG64,    *PLONG64;
typedef int64_t  LONG_PTR,  *PLONG_PTR;
typedef uint64_t ULONGLONG, *PULONGLONG;
typedef uint64_t ULONG64,   *PULONG64;
typedef uint64_t ULONG_PTR, *PULONG_PTR;
typedef uint64_t DWORD64,   *PDWORD64;

typedef CHAR *PCHAR, *LPCH, *PCH, *PNZCH;
typedef const CHAR *LPCCH, *PCCH, *PCNZCH;
typedef CHAR *NPSTR, *LPSTR, *PSTR;
typedef PSTR *PZPSTR;
typedef const PSTR *PCZPSTR;
typedef const CHAR *LPCSTR, *PCSTR;
typedef PCSTR *PZPCSTR;

typedef char16_t WCHAR;
typedef WCHAR *PWCHAR, *LPWCH, *PWCH;
typedef const WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR, *LPWSTR, *PWSTR;
typedef PWSTR *PZPWSTR;
typedef const PWSTR *PCZPWSTR;
typedef WCHAR *LPUWSTR, *PUWSTR;
typedef const WCHAR *LPCWSTR, *PCWSTR;
typedef PCWSTR *PZPCWSTR;
typedef const WCHAR *LPCUWSTR, *PCUWSTR;

typedef WCHAR *PZZWSTR;
typedef const WCHAR *PCZZWSTR;
typedef WCHAR *PUZZWSTR;
typedef const WCHAR *PCUZZWSTR;

typedef WCHAR *PNZWCH;
typedef const WCHAR *PCNZWCH;
typedef WCHAR *PUNZWCH;
typedef const WCHAR *PCUNZWCH;

typedef const WCHAR *LPCWCHAR, *PCWCHAR;
typedef const WCHAR *LPCUWCHAR, *PCUWCHAR;

typedef const UCHAR *PCUCHAR;

typedef char CCHAR;
typedef CCHAR *PCCHAR;
typedef int16_t CSHORT, *PCSHORT;
typedef uint32_t CLONG, *LCLONG;

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

#define MINCHAR     INT8_MIN
#define MAXCHAR     INT8_MAX
#define MINSHORT    INT16_MIN
#define MAXSHORT    INT16_MAX
#define MINLONG     INT32_MIN
#define MAXLONG     INT32_MAX
#define MAXUCHAR    UINT8_MAX
#define MAXUSHORT   UINT16_MAX
#define MAXULONG    UINT32_MAX
#define MAXBYTE     UINT8_MAX
#define MAXWORD     UINT16_MAX
#define MAXDWORD    UINT32_MAX
#define MAXLONGLONG INT64_MAX

typedef __builtin_va_list VA_LIST;