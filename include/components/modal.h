#ifndef MODAL_H
#define MODAL_H

#include "../clay_extensions.h"
#include "../../vendor/clay/clay.h"
#include "../config.h"

typedef struct {
    bool is_open;
    float width;
    float height;
} Modal;

void RenderModal(Modal* modal, void (*render_content)(void));
void CloseModal(Modal* modal);
void OpenModal(Modal* modal);

#endif // MODAL_H
