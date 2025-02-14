#include <verilated.h>

#include TOP_HEADER

#include "hdl-dev/verilated.h"

#ifndef PROTO
#error "Define PROTO to the protocol being used"
#endif

#define CONCAT2_(x, y) x ## y
#define CONCAT2(x, y) CONCAT2_(x, y)

#define CONCAT3_(x, y, z) x ## y ## z
#define CONCAT3(x, y, z) CONCAT3_(x, y, z)

extern "C" {

static uint64_t _time = 0;

void *hdl_dev_new(void)
{
    TOP *top = new TOP("");
    return static_cast<void *>(top);
}

void hdl_dev_init(void *top_, void *ports_)
{
    TOP *top = static_cast<TOP *>(top_);
    CONCAT3(HDL, PROTO, Ports) *ports = static_cast<CONCAT3(HDL, PROTO, Ports) *>(ports_);

#define X(T, x) ports->x = &top->x

    CONCAT2(PROTO, _PORTS)

#undef X
}

int hdl_dev_timeprecision(void *top_)
{
    TOP *top = static_cast<TOP *>(top_);
    return top->contextp()->timeprecision();
}

void hdl_dev_step(void *top_, hdl_step_stage_t stage)
{
    TOP *top = static_cast<TOP *>(top_);
    VerilatedContext *ctx = top->contextp();

    switch (stage) {
        case STEP_TIME_INC:
            // This ensures multiple devices using the
            // same context will be kept in sync even
            // when `hdl_dev_step` has already been called
            // with `STEP_TIME_INC` on another device using
            // the same context. This method will work as
            // long as all instances of the same device use
            // the same context, since all such instances
            // share the `_time` variable.
            if (_time == ctx->time())
                ctx->timeInc(1);

            break;
        case STEP_EVAL:
            _time = ctx->time();
            top->eval_step();

            break;
        case STEP_EVAL_END:
            top->eval_end_step();
            break;
    }
}

void hdl_dev_free(void *top_)
{
    TOP *top = static_cast<TOP *>(top_);
    delete top;
}

int CONCAT3(HDL_, PROTO, _DEVICE_SYM);

}
