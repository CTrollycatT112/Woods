// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Initial Boot Video

AUTHOR: Trollycat

ABSTRACT: Basic GOP mode before the GUI is initialized (LARP there won't be GUI anytime soon :troll:)

--*/
#pragma once

#include "htbase.hpp"

EXTERN ULONG64 InbvScreenWidth;
EXTERN ULONG64 InbvScreenHeight;

#define INBV_SCREEN_WIDTH  InbvScreenWidth
#define INBV_SCREEN_HEIGHT InbvScreenHeight

#define INBV_COLOR_BLACK         (ULONG)0x00000000
#define INBV_COLOR_BLUE          (ULONG)0x000000AA
#define INBV_COLOR_GREEN         (ULONG)0x0000AA00
#define INBV_COLOR_CYAN          (ULONG)0x0000AAAA
#define INBV_COLOR_RED           (ULONG)0x00AA0000
#define INBV_COLOR_MAGENTA       (ULONG)0x00AA00AA
#define INBV_COLOR_BROWN         (ULONG)0x00AA5500
#define INBV_COLOR_LIGHT_GRAY    (ULONG)0x00AAAAAA
#define INBV_COLOR_DARK_GRAY     (ULONG)0x00555555
#define INBV_COLOR_LIGHT_BLUE    (ULONG)0x005555FF
#define INBV_COLOR_LIGHT_GREEN   (ULONG)0x0055FF55
#define INBV_COLOR_LIGHT_CYAN    (ULONG)0x0055FFFF
#define INBV_COLOR_LIGHT_RED     (ULONG)0x00FF5555
#define INBV_COLOR_LIGHT_MAGENTA (ULONG)0x00FF55FF
#define INBV_COLOR_YELLOW        (ULONG)0x00FFFF55
#define INBV_COLOR_WHITE         (ULONG)0x00FFFFFF

#define INBV_HIDE_CURSOR              "\033[?25l"
#define INBV_SHOW_CURSOR              "\033[?25h"
#define INBV_CLEAR_SCREEN_HOME_CURSOR "\033[H\033[2J"

namespace Inbv
{
    VOID
    Initialize();

    HTAPI
    VOID
    FillDisplay(ULONG Left,
                ULONG Top,
                ULONG Right,
                ULONG Bottom,
                ULONG Color);

    HTAPI
    VOID
    WriteString(PCSTR String);

    HTAPI
    VOID
    ResetDisplay();

    HTAPI
    VOID
    QueryFramebufferInformation(PULONG Width,
                                PULONG Height,
                                PULONG64 Framebuffer,
                                PULONG BitsPerPixel);
} // namespace Inbv