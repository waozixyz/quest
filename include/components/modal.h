#ifndef MODAL_H
#define MODAL_H

#include "rocks_clay.h"
#include "../config.h"

typedef struct {
    bool is_open;
    float width;
    float height;
} Modal;

void RenderModal(Modal* modal, void (*render_content)(void));
void CloseModal(Modal* modal);
void OpenModal(Modal* modal);



#ifndef __EMSCRIPTEN__
#include "SDL.h"
#endif

#endif // MODAL_H
