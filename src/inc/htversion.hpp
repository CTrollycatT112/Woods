/*++

MODULE: Version tracker

AUTHOR: Trollycat

ABSTRACT: Tracks information about the os

--*/
#pragma once

//
// HELPERS
//
#define STR_HELPER(x)        #x
#define VER_TO_STR(x)        STR_HELPER(x)

//
// OS
//
#define OS_NAME          "Woods"
#define OS_VERSION_MAJOR 0
#define OS_VERSION_MINOR 1
#define OS_VERSION_PATCH 0
#define OS_ARCH          "X86_64"

//
// BUILD
//
#define OS_BUILD_DATE __DATE__
#define OS_BUILD_TIME __TIME__

#define OS_VERSION_STRING \
    OS_NAME " Version " VER_TO_STR(OS_VERSION_MAJOR) "." VER_TO_STR(OS_VERSION_MINOR) "." VER_TO_STR(OS_VERSION_PATCH) \
    " (" OS_BUILD_DATE " " OS_BUILD_TIME ") [Arch: " OS_ARCH "]"
