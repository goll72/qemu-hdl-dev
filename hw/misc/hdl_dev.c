/*
 * HDL common device for qemu-hdl-dev
 *
 * Copyright (c) 2025 João Gabriel <joaog.bastos@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "qemu/timer.h"
#include "qemu/hdl-dev.h"
#include "qemu/error-report.h"

#include "qapi/error.h"

#include "hw/hw.h"
#include "hw/misc/hdl_dev.h"
#include "hw/qdev-properties.h"
#include "qom/object.h"

#include <dlfcn.h>

static void hdl_dev_timeout(void *opaque)
{
    HDLDevClass *hdc = HDL_DEVICE_CLASS(opaque);

    int n = 9 + hdc->timeprecision;
    uint64_t interval = 1;
    uint64_t repeat = 1;

    for (int i = 0; i < n; i++)
        interval *= 10;

    for (int i = 0; i < -n; i++)
        repeat *= 10;

    HDLDevState *head = hdc->head;

    for (int i = 0; i < repeat; i++) {
        for (HDLDevState *current = head; current != NULL; current = current->next)
            current->hdl_dev_step(current->ctx, STEP_TIME_INC);

        for (HDLDevState *current = head; current != NULL; current = current->next)
            current->hdl_dev_step(current->ctx, STEP_EVAL);

        for (HDLDevState *current = head; current != NULL; current = current->next)
            current->hdl_dev_step(current->ctx, STEP_EVAL_END);
    }

    for (HDLDevState *current = head; current != NULL; current = current->next)
        current->timeout(current->timeout_ctx, interval);

    uint64_t now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    timer_mod(hdc->timer, now + interval);
}

static void hdl_dev_realize(DeviceState *dev, Error **errp)
{
    HDLDevState *s = HDL_DEVICE(dev);

    if (s->dyn_filename == NULL) {
        error_setg(errp, "HDL device: property 'filename' not specified");
        return;
    }

    if (!strchr(s->dyn_filename, '/'))
        warn_report_once("HDL device: %s: shared library path should probably contain a slash character", s->dyn_filename);

    s->dyn_handle = dlopen(s->dyn_filename, RTLD_LAZY | RTLD_LOCAL);

    if (s->dyn_handle == NULL) {
        error_setg(errp, "HDL device: %s", dlerror());
        return;
    }

    void *(*hdl_dev_new)(void) = dlsym(s->dyn_handle, "hdl_dev_new");
    int (*hdl_dev_timeprecision)(void *) = dlsym(s->dyn_handle, "hdl_dev_timeprecision");

    s->hdl_dev_init = dlsym(s->dyn_handle, "hdl_dev_init");
    s->hdl_dev_step = dlsym(s->dyn_handle, "hdl_dev_step");
    s->hdl_dev_free = dlsym(s->dyn_handle, "hdl_dev_free");

    if (hdl_dev_new == NULL || hdl_dev_timeprecision == NULL ||
            s->hdl_dev_init == NULL || s->hdl_dev_step == NULL || s->hdl_dev_free == NULL) {
        char *error = dlerror();

        error_setg(errp, "HDL device: error loading symbols from file '%s'%s%s",
                   s->dyn_filename, error ? ": " : "", error ? error : "");
        return;
    }

    s->ctx = hdl_dev_new();

    s->next = NULL;

    HDLDevClass *hdc = HDL_DEVICE_GET_CLASS(s);

    if (hdc->head == NULL)
        hdc->head = s;

    if (hdc->tail != NULL)
        hdc->tail->next = s;

    hdc->tail = s;

    int timeprecision = hdl_dev_timeprecision(s->ctx);

    if (timeprecision < hdc->timeprecision)
        hdc->timeprecision = timeprecision;

    if (timer_pending(hdc->timer))
        timer_del(hdc->timer);

    int n = 9 + hdc->timeprecision;
    uint64_t interval = 1;

    for (int i = 0; i < n; i++)
        interval *= 10;

    uint64_t now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    timer_mod(hdc->timer, now + interval);
}

static void hdl_dev_unrealize(DeviceState *dev)
{
    HDLDevState *s = HDL_DEVICE(dev);

    s->hdl_dev_free(s->ctx);
    dlclose(s->dyn_handle);

    s->hdl_dev_init = NULL;
    s->hdl_dev_step = NULL;
    s->hdl_dev_free = NULL;

    s->dyn_handle = NULL;
    s->ctx = NULL;

    s->next = NULL;
}

static const Property hdl_dev_props[] = {
    DEFINE_PROP_STRING("filename", HDLDevState, dyn_filename)
};

static void hdl_dev_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    HDLDevClass *hdc = HDL_DEVICE_CLASS(klass);

    dc->realize = hdl_dev_realize;
    dc->unrealize = hdl_dev_unrealize;

    dc->desc = "HDL device";
    dc->user_creatable = true;

    set_bit(DEVICE_CATEGORY_MISC, dc->categories);

    device_class_set_props(dc, hdl_dev_props);

    hdc->head = NULL;
    hdc->tail = NULL;

    hdc->timeprecision = 2;

    hdc->timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, hdl_dev_timeout, hdc);
}

static const TypeInfo hdl_dev_info = {
    .name = TYPE_HDL_DEVICE,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(HDLDevState),
    .class_init = hdl_dev_class_init,
    .class_size = sizeof(HDLDevClass)
};

static void hdl_dev_register_types(void)
{
    type_register_static(&hdl_dev_info);
}

type_init(hdl_dev_register_types)
