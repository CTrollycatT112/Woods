// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Core header

AUTHOR: Trollycat

ABSTRACT: Includes important components so you don't need to include all of them

--*/
#pragma once

// IWYU "include what you use"
// This tells clang to shut up basically
// It will tell it that these are exported,
// and used by other files...

#include "httypes.hpp" // IWYU pragma: export
#include "htdef.hpp" // IWYU pragma: export
#include "htstatus.hpp" // IWYU pragma: export