// Copyright (c) 2026 Hobby Technologies

/*++

MODULE: Initial Boot Video

AUTHOR: Trollycat

ABSTRACT: Bare-bones boot video driver before GUI (LARP there won't be GUI anytime soon :troll:)

--*/
#pragma once

#include "htbase.hpp"

//
// Pulled from the framebuffer/screen
//
EXTERN ULONG64 InbvScreenWidth;
EXTERN ULONG64 InbvScreenHeight;

//
// Define wrappers for easier access
//
#define INBV_SCREEN_WIDTH  InbvScreenWidth
#define INBV_SCREEN_HEIGHT InbvScreenHeight

//
// Color constants
// Should probably store these in a dedicated Color.hpp
// Whatever (tung is too lazy..)
//
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

//
// Escape codes for Flanterm...
// Manages the little white cursor...
//
#define INBV_HIDE_CURSOR              "\033[?25l"
#define INBV_SHOW_CURSOR              "\033[?25h"
#define INBV_CLEAR_SCREEN_HOME_CURSOR "\033[H\033[2J"

namespace Inbv
{
    
    /*++

    ROUTINE: Initialize

    DESCRIPTION: Start's the boot video driver

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/
    VOID
    Initialize();

    /*++

    ROUTINE: FillDisplay

    DESCRIPTION: Fill a chunk of the screen with a Color

    ARGUMENTS: Left - Start of left side, Top - Start of Top side, Right - Start of right side, Bottom - Start of Bottom side, Color - Fill Color

    RETURNS: VOID

    --*/
    HTAPI
    VOID
    FillDisplay(ULONG Left,
                ULONG Top,
                ULONG Right,
                ULONG Bottom,
                ULONG Color);

    
    /*++

    ROUTINE: WriteChar

    DESCRIPTION: Print a single character to the screen

    ARGUMENTS: Character - Character to print..

    RETURNS: VOID

    --*/
    HTAPI
    VOID
    WriteChar(CHAR Character);
    
    /*++

    ROUTINE: WriteString

    DESCRIPTION: Print a string to the screen (usually white text)

    ARGUMENTS: String - Text to print...

    RETURNS: VOID

    --*/
    HTAPI
    VOID
    WriteString(PCSTR String);

        
    /*++

    ROUTINE: ResetDisplay

    DESCRIPTION: Refresh flanterm display to default settings

    ARGUMENTS: N/A

    RETURNS: VOID

    --*/
    HTAPI
    VOID
    ResetDisplay();

    /*++

    ROUTINE: QueryFramebufferInformation

    DESCRIPTION: Fetches the configuration of the display

    ARGUMENTS: Width - Screen width, Height - Screen height, Framebuffer - Video memory address, BitsPerPixel - Color depth

    RETURNS: VOID

    --*/
    HTAPI
    VOID
    QueryFramebufferInformation(PULONG Width,
                                PULONG Height,
                                PULONG64 Framebuffer,
                                PULONG BitsPerPixel);
} // namespace Inbv