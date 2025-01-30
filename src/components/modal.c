#include "components/modal.h"

#include "rocks.h"
#include "quest_theme.h"

static void HandleBackdropClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        Modal* modal = (Modal*)userData;
        modal->is_open = false;
        
        #if defined(CLAY_MOBILE)
        SDL_StopTextInput();
        #endif
    }
}
void OpenModal(Modal* modal) {
    modal->is_open = true;
    
    #if defined(CLAY_MOBILE)
    SDL_StopTextInput();
    #endif
}

void RenderModal(Modal* modal, void (*render_content)(void)) {
    RocksTheme base_theme = rocks_get_theme(g_rocks);

    if (!modal->is_open) return;

    // Backdrop
    CLAY(CLAY_ID("ModalBackdrop"),
        CLAY_FLOATING({
            .parentId = Clay__HashString(CLAY_STRING("Clay__RootContainer"), 0, 0).id,
            .attachment = { 
                .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                .parent = CLAY_ATTACH_POINT_CENTER_CENTER 
            },
            .zIndex = 1000
        }),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_FIXED(2000), CLAY_SIZING_FIXED(2000) }
        }),
        CLAY_RECTANGLE({ .color = {0, 0, 0, 128} }),
        Clay_OnHover(HandleBackdropClick, (intptr_t)modal)
    );

    // Modal Content
    CLAY(CLAY_ID("ModalContent"),
        CLAY_FLOATING({
            .parentId = Clay__HashString(CLAY_STRING("Clay__RootContainer"), 0, 0).id,
            .attachment = { 
                .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                .parent = CLAY_ATTACH_POINT_CENTER_CENTER 
            },
            .zIndex = 1001
        }),
        CLAY_LAYOUT({
            .sizing = { 
                CLAY_SIZING_FIXED(modal->width),
                CLAY_SIZING_FIXED(modal->height)
            },
            .padding = { 20, 20, 20, 20 }
        }),
        CLAY_RECTANGLE({ 
            .color = base_theme.background_hover,
            .cornerRadius = CLAY_CORNER_RADIUS(8)
        })
    ) {
        if (render_content) {
            render_content();
        }
    }
}

void CloseModal(Modal* modal) {
    modal->is_open = false;
}