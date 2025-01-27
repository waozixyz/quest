// components/date_picker.h
#ifndef DATE_PICKER_H
#define DATE_PICKER_H

#include <time.h>
#include "rocks_clay.h"
#include "modal.h"
#include <string.h>

void InitializeDatePicker(time_t initial_date, void (*on_date_change)(time_t), Modal* modal);

void RenderDatePicker(time_t current_date, void (*on_date_change)(time_t), Modal* modal);

#endif
