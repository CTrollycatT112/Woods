/*++

MODULE: Initial Boot Video

AUTHOR: Trollycat

ABSTRACT: Basic GOP mode before the GUI is initialized (LARP there won't be GUI anytime soon :troll:)

--*/
#include "inbv/inbv.hpp"
#include "inbv/inbvfont.hpp"

#include "rtl/rtl.hpp"

#include "limine.h"

#include <flanterm.h>
#include <flanterm_backends/fb.h>

VOLATILE
LIMINE_REQUEST
struct limine_framebuffer_request framebuffer_request = 
{
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

struct 
limine_framebuffer *VOLATILE framebuffer = NULL;

STATIC
flanterm_context* VOLATILE InbvTermContext = NULL;

ULONG64 InbvScreenWidth  = 0;
ULONG64 InbvScreenHeight = 0;

// PROBLEM:
// WE DON'T HAVE MEMORY ALLOCATORS YET
// THIS IS A TERRIBLE HACK BUT HERE I USE A STATIC POOL...
// CHANGE THIS AS SOON AS POSSIBLE
// TODO: DON'T DO THIS LOL

static BYTE FlantermPool[1024 * 1024];
static QWORD FlantermPoolIndex = 0;

PVOID
FlantermAllocate(QWORD Size)
{
    Size = (Size + 15) & ~15;

    if (FlantermPoolIndex + Size > sizeof(FlantermPool))
    {
        return NULL;
    }

    PVOID Ptr = &FlantermPool[FlantermPoolIndex];
    FlantermPoolIndex += Size;

    return Ptr;
}

VOID
FlantermFree(PVOID Ptr, QWORD Size)
{
    // Yeah, literally do nothing
    UNREFERENCED_PARAMETER(Ptr);
    UNREFERENCED_PARAMETER(Size);
}

// ---- END OF STUPID SECTION...

namespace Inbv
{
    namespace
    {
        VOID
        PlotPixel(ULONG Color,
                ULONG X,
                ULONG Y)
        {
            if (X >= framebuffer->width ||
                Y >= framebuffer->height)
            {
                return;
            }

            PUCHAR Place = (PUCHAR)((ULONG64)framebuffer->address +
                                    framebuffer->pitch * Y + X *
                                    framebuffer->bpp / 8);

            switch (framebuffer->bpp / 8)
        	{
                case 1:
                    *Place = (UCHAR)(Color | (Color >> 8) | (Color >> 16));
                    break;
                case 2:
                    *((PUSHORT)Place) = Color | (Color >> 16);
                    break;
                case 3:
                    Place[0] = (UCHAR)Color;
                    Place[1] = (UCHAR)(Color >> 8);
                    Place[2] = (UCHAR)(Color >> 16);
                    break;
                case 4:
                    *((PULONG32)Place) = Color;
                    break;
                default:
                    break;
            }

        }
    } // namespace

    VOID
    FillDisplay(ULONG Left,
                ULONG Top,
                ULONG Right,
                ULONG Bottom,
                ULONG32 Color)
    {
        if (framebuffer == NULL)
        {
            return;
        }

        for (ULONG Y = Top; Y < Bottom; Y++)
        {
            for (ULONG X = Left; X < Right; X++)
            {
                PlotPixel(Color, X, Y);
            }
        }
    }

    VOID
    Initialize()
    {
        ASSERT(framebuffer_request.response != NULL && 
               framebuffer_request.response->framebuffer_count >= 1);

        framebuffer = framebuffer_request.response->framebuffers[0];

        InbvScreenWidth  = framebuffer->width;
        InbvScreenHeight = framebuffer->height;

        FillDisplay(0,
                    0,
                    framebuffer->width,
                    framebuffer->height,
                    INBV_COLOR_BLUE);

        ULONG32 DefaultBg = INBV_COLOR_BLUE;
        ULONG32 DefaultFg = INBV_COLOR_WHITE;

        InbvTermContext = flanterm_fb_init(
            FlantermAllocate, 
            FlantermFree, 
            (PDWORD)framebuffer->address,
            framebuffer->width,
            framebuffer->height, 
            framebuffer->pitch, 
            framebuffer->red_mask_size, 
            framebuffer->red_mask_shift,
            framebuffer->green_mask_size, 
            framebuffer->green_mask_shift, 
            framebuffer->blue_mask_size, 
            framebuffer->blue_mask_shift,
            NULL, 
            NULL, 
            NULL, 
            &DefaultBg, 
            &DefaultFg, 
            NULL, 
            NULL, 
            InbvDisplayFont, 
            8, 
            16, 
            0, 
            1, 
            1, 
            0,
            0
        );

        if (InbvTermContext != NULL)
        {
            flanterm_write(InbvTermContext,
                           INBV_HIDE_CURSOR,
                           Rtl::AnsiStringLength(INBV_HIDE_CURSOR));
        }
    }

    VOID
    ResetDisplay()
    {
        if (InbvTermContext == NULL)
        {
            return;
        }

        flanterm_write(InbvTermContext,
                       INBV_CLEAR_SCREEN_HOME_CURSOR,
                       Rtl::AnsiStringLength(INBV_CLEAR_SCREEN_HOME_CURSOR));
    }

    VOID
    WriteChar(CHAR Character)
    {
        if (InbvTermContext == NULL)
        {
            return;
        }

        flanterm_write(InbvTermContext, &Character, 1);
    }

    VOID
    WriteString(PCSTR String)
    {
        if (InbvTermContext == NULL)
        {
            return;
        }

        flanterm_write(InbvTermContext, String, Rtl::AnsiStringLength(String));
    }

    VOID
    QueryFramebufferInformation(PULONG Width, 
                                PULONG Height,
                                PULONG64 Framebuffer,
                                PULONG BitsPerPixel)
    {
        *Width = framebuffer->width;
        *Height = framebuffer->height;
        *Framebuffer = (QWORD)framebuffer->address;
        *BitsPerPixel = framebuffer->bpp;
    }
} // namespace Inbv