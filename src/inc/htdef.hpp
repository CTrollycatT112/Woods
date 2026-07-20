/*++

MODULE: Macro definitions

AUTHOR: Trollycat

ABSTRACT: Defines macro's used across the project (like NODISCARD)

--*/
#pragma once

#ifndef NODISCARD
#define NODISCARD [[nodiscard]]
#endif

#ifndef NORETURN
#define NORETURN  [[noreturn]]
#endif

#ifndef PACKED
#define PACKED    [[gnu::packed]]
#endif

#ifndef UNLIKELY
#define UNLIKELY  [[unlikely]]
#endif

#ifndef MAYBE_UNUSED
#define MAYBE_UNUSED [[maybe_unused]]
#endif

#ifndef EXTERN_C
#define EXTERN_C extern "C"
#endif

#define IN
#define OUT
#define IN_OUT

#define HTAPI
#define __TEXT(qoute) L##qoute

#ifndef CONST
#define CONST const
#endif

#ifndef CONSTEXPR
#define CONSTEXPR constexpr
#endif

#ifndef STATIC
#define STATIC static
#endif

#ifndef VOLATILE
#define VOLATILE volatile
#endif

#ifndef EXTERN
#define EXTERN extern
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL nullptr
#endif

#ifndef INLINE
#define INLINE inline
#endif

#ifndef INLINECONST
#define INLINECONST inline constexpr
#endif

#ifndef ALWAYSINLINE
#define ALWAYSINLINE __attribute__((always_inline))
#endif

#ifndef NOINLINE
#define NOINLINE __attribute__((noinline))
#endif

#ifndef ALIGN
#define ALIGN(x) __attribute__((aligned(x)))
#endif

#ifndef CODESEG
#define CODESEG(x) __attribute__((section(x)))
#endif

#ifndef LIMINE_REQUEST
#define LIMINE_REQUEST __attribute__((used, section(".limine_requests")))
#endif

#ifndef ISALIGNED
#define ISALIGNED(addr, alignment) (((addr) & ((alignment) - 1)) == 0)
#endif

#ifndef OFFSETOF
#define OFFSETOF(type, member) reinterpret_cast<SIZE_T>(&(reinterpret_cast<type*>(0)->member))
#endif

#define UNREFERENCED_PARAMETER(P) ((void)(P))

#define HT_SUCCESS(Status)              (((HTSTATUS)(Status)) >= 0)
#define HT_INFORMATION(Status)          ((((ULONG)(Status)) >> 30) == 1)
#define HT_WARNING(Status)              ((((ULONG)(Status)) >> 30) == 2)
#define HT_ERROR(Status)                ((((ULONG)(Status)) >> 30) == 3)

#define ANSI_NULL ((CHAR)0)
#define UNICODE_NULL ((WCHAR)0)
#define UNICODE_STRING_MAX_BYTES ((USHORT) 65534)
#define UNICODE_STRING_MAX_CHARS (32767)

#define RTL_FIELD_TYPE(type, field)    (((type*)0)->field)
#define RTL_BITS_OF(sizeOfArg)         (sizeof(sizeOfArg) * 8)
#define RTL_BITS_OF_FIELD(type, field) (RTL_BITS_OF(RTL_FIELD_TYPE(type, field)))
#define RTL_FIELD_SIZE(type, field) (sizeof(((type *)0)->field))

#define BASE36_ALPHABET L"0123456789abcdefghijklmnopqrstuvwxyz"

#define RTL_SIZEOF_THROUGH_FIELD(type, field) \
    (FIELD_OFFSET(type, field) + OFFSETOF(type, field))

#define RTL_CONTAINS_FIELD(Struct, Size, Field) \
    ( (((PCHAR)(&(Struct)->Field)) + sizeof((Struct)->Field)) <= (((PCHAR)(Struct))+(Size)) )

#define RTL_NUMBER_OF_V1(A) (sizeof(A)/sizeof((A)[0]))

#define ALIGNOFF(P, Boundary)   ((ULONG_PTR)(P) & ((Boundary) - 1))
#define IS_ALIGNED(P, Boundary) (ALIGNOFF(P, Boundary) == 0)

#define UNREACHABLE __builtin_unreachable()