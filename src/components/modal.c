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
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);

    if (!modal->is_open) return;

    // Backdrop
    CLAY({
        .id = CLAY_ID("ModalBackdrop"),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_FIXED(2000),
                .height = CLAY_SIZING_FIXED(2000)
            }
        },
        .floating = {
            .parentId = Clay_GetElementId(CLAY_STRING("Clay__RootContainer")).id,
            .attachPoints = {
                .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                .parent = CLAY_ATTACH_POINT_CENTER_CENTER
            },
            .zIndex = 1000,
            .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID
        },
        .backgroundColor = { 0, 0, 0, 128 }
    }) {
        Clay_OnHover(HandleBackdropClick, (intptr_t)modal);
    }

    // Modal Content
    CLAY({
        .id = CLAY_ID("ModalContent"),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_FIXED(modal->width),
                .height = CLAY_SIZING_FIXED(modal->height)
            },
            .padding = CLAY_PADDING_ALL(20)
        },
        .floating = {
            .parentId = Clay_GetElementId(CLAY_STRING("Clay__RootContainer")).id,
            .attachPoints = {
                .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                .parent = CLAY_ATTACH_POINT_CENTER_CENTER
            },
            .zIndex = 1001,
            .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID
        },
        .backgroundColor = base_theme.background_hover,
        .cornerRadius = CLAY_CORNER_RADIUS(8)
    }) {
        if (render_content) {
            render_content();
        }
    }
}

void CloseModal(Modal* modal) {
    modal->is_open = false;
}