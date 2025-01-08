#ifndef MODAL_H
#define MODAL_H

#include "../clay_extensions.h"
#include "../../vendor/clay/clay.h"
#include "../styles.h"

typedef struct {
    bool is_open;
    float width;
    float height;
} Modal;

void RenderModal(Modal* modal, void (*render_content)(void));
void CloseModal(Modal* modal);

#endif // MODAL_H
