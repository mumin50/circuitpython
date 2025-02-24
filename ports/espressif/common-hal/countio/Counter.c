/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 microDev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "common-hal/countio/Counter.h"
#include "shared-bindings/countio/Counter.h"
#include "common-hal/microcontroller/Pin.h"

#include "py/runtime.h"
#include "supervisor/shared/translate/translate.h"

#include "driver/gpio.h"

void common_hal_countio_counter_construct(countio_counter_obj_t *self,
    const mcu_pin_obj_t *pin, countio_edge_t edge, digitalio_pull_t pull) {
    claim_pin(pin);

    // Prepare configuration for the PCNT unit
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = pin->number,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        .channel = PCNT_CHANNEL_0,
        // What to do on the rising / falling edge of pulse input?
        // If EDGE_RISE_AND_FALL, both modeswill do PCNT_COUNT_INC.
        .pos_mode = (edge == EDGE_FALL) ? PCNT_COUNT_DIS : PCNT_COUNT_INC, // Count up unless only fall
        .neg_mode = (edge == EDGE_RISE) ? PCNT_COUNT_DIS : PCNT_COUNT_INC, // Count up unless only rise
    };

    // Initialize PCNT unit
    const int8_t unit = peripherals_pcnt_init(&pcnt_config);
    if (unit == -1) {
        mp_raise_RuntimeError(translate("All PCNT units in use"));
    }

    self->pin = pin->number;

    gpio_pullup_dis(pin->number);
    gpio_pulldown_dis(pin->number);
    if (pull == PULL_UP) {
        gpio_pullup_en(pin->number);
    } else if (pull == PULL_DOWN) {
        gpio_pulldown_en(pin->number);
    }

    self->unit = (pcnt_unit_t)unit;
}

bool common_hal_countio_counter_deinited(countio_counter_obj_t *self) {
    return self->unit == PCNT_UNIT_MAX;
}

void common_hal_countio_counter_deinit(countio_counter_obj_t *self) {
    if (common_hal_countio_counter_deinited(self)) {
        return;
    }
    reset_pin_number(self->pin);
    peripherals_pcnt_deinit(&self->unit);
}

mp_int_t common_hal_countio_counter_get_count(countio_counter_obj_t *self) {
    int16_t count;
    pcnt_get_counter_value(self->unit, &count);
    return count + self->count;
}

void common_hal_countio_counter_set_count(countio_counter_obj_t *self,
    mp_int_t new_count) {
    self->count = new_count;
    pcnt_counter_clear(self->unit);
}
