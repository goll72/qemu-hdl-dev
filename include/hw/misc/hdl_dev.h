#ifndef QEMU_HDL_DEV_H
#define QEMU_HDL_DEV_H

#include "hw/hw.h"
#include "hw/qdev-core.h"

#include "qom/object.h"

#define TYPE_HDL_DEVICE "hdl-dev"
OBJECT_DECLARE_TYPE(HDLDevState, HDLDevClass, HDL_DEVICE)

struct HDLDevState {
    DeviceState parent_obj;

    // Intrusive linked list
    HDLDevState *next;

    // Shared library containing the Verilator simulation code
    char *dyn_filename;
    void *dyn_handle;

    // Opaque pointer passed to the functions below
    void *ctx;

    void *(*hdl_dev_init)(void *ctx, void *ports);
    void (*hdl_dev_step)(void *ctx, int stage);
    void (*hdl_dev_free)(void *ctx);

    // Timer timeout function, will step the simulation by one
    // time unit, providing inputs and reacting to outputs
    void *timeout_ctx;
    void (*timeout)(void *timeout_ctx, uint64_t interval);
};

struct HDLDevClass {
    DeviceClass parent_class;

    // Linked list of devices, used so that we can
    // step all devices at the same time, keeping their
    // simulation time in sync
    HDLDevState *head;
    HDLDevState *tail;

    QEMUTimer *timer;

    // The timeprecision value of the finest device being simulated
    int timeprecision;
};

#endif
