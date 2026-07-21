/*++

MODULE: Bug check crash handler

AUTHOR: Trollycat

ABSTRACT: BugCheck/Panic System, the operating system cannot recover

--*/
#pragma once

#include "htbase.hpp"

#include "ke/amd64/amd64.hpp"

namespace Ke
{
    /*++

    ROUTINE: BugCheckWithIf

    DESCRIPTION:
        Called once an unrecoverable error happens
        This function exists for exception handlers,
        So they can pass raw CPU state directly

    ARGUMENTS:
        BugCheckCode
            - Code for the crash type
        BugCheckParameter1
            - First crash value, determined by crash code
        BugCheckParameter2
            - Second crash value, determined by crash code
        BugCheckParameter3
            - Third crash value, determined by crash code
        BugCheckParameter4
            - Fourth crash value, determined by crash code
        TrapFrame
            - Optional exception frame

    RETURNS: NORETURN

    --*/
    NORETURN
    HTAPI
    VOID
    BugCheckWithIf(IN ULONG     BugCheckCode,
                   IN ULONG64   BugCheckParameter1,
                   IN ULONG64   BugCheckParameter2,
                   IN ULONG64   BugCheckParameter3,
                   IN ULONG64   BugCheckParameter4,
                   PKTRAP_FRAME TrapFrame);

    /*++

    ROUTINE: BugCheckEx

    DESCRIPTION:
        Called once an unrecoverable error happens
        This function exists for drivers and software
        They do not have a raw CPU state to pass in
        SEE: BugCheckWithIf for exception handlers

    ARGUMENTS:
        BugCheckCode
            - Code for the crash type
        BugCheckParameter1
            - First crash value, determined by crash code
        BugCheckParameter2
            - Second crash value, determined by crash code
        BugCheckParameter3
            - Third crash value, determined by crash code
        BugCheckParameter4
            - Fourth crash value, determined by crash code

    RETURNS: NORETURN

    --*/
    NORETURN
    HTAPI
    VOID
    BugCheckEx(IN ULONG     BugCheckCode,
               IN ULONG64   BugCheckParameter1,
               IN ULONG64   BugCheckParameter2,
               IN ULONG64   BugCheckParameter3,
               IN ULONG64   BugCheckParameter4);

    /*++

    ROUTINE: BugCheck

    DESCRIPTION:
        Called once an unrecoverable error happens
        This is a legacy function for fallback
        Do not call this if not needed

    ARGUMENTS:
        BugCheckCode
            - Code for the crash type

    RETURNS: NORETURN

    --*/
    NORETURN
    HTAPI
    VOID
    BugCheck(IN ULONG BugCheckCode);
} // namespace Ke